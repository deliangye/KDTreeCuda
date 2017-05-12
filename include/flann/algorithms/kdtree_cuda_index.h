/***********************************************************************
* Software License Agreement (BSD License)
*
* Copyright 2017  Wolfgang Brandenburger (w.brandenburger@unibw.de). All rights reserved.
*
* THE BSD LICENSE
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*************************************************************************/

#ifndef FLANN_KDTREE_CUDA_INDEX_H_
#define FLANN_KDTREE_CUDA_INDEX_H_

#include <algorithm>
#include <map>
#include <cassert>
#include <cstring>
#include <stdarg.h>
#include <cmath>

#include "flann/general.h"
#include "flann/algorithms/nn_index.h"
#include "flann/util/dynamic_bitset.h"
#include "flann/util/matrix.h"
#include "flann/util/result_set.h"
#include "flann/util/heap.h"
//#include "flann/util/allocator.h"
#include "flann/util/random.h"
#include "flann/util/saving.h"
#include "flann/util/params.h"

#include "flann/util/datastructures.h"

#include "tools/utils/allocator.h" // FILE REPLACE THE ALLOCATOR FUNCTION IN FOLDER!!!
#include "tools/utils/matrix.h"
#include "tools/utils/nodes.h"

#ifdef _DEBUG
	#ifndef DEBUG_NEW
		#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
		#define new DEBUG_NEW
	#endif
#endif

namespace flann
{

	struct KDTreeCudaIndexParams : public IndexParams
	{
		KDTreeCudaIndexParams(int trees = 1)
		{
			(*this)["algorithm"] = FLANN_INDEX_KDTREE_CUDA;
			(*this)["trees"] = trees;
		}
	};

	/**
		Randomized kd-tree index on GPU

		Contains the k-d trees and other information for indexing a set of points
		for nearest-neighbor matching.
	*/
	template <typename Distance>
	class KDTreeCudaIndex : public NNIndex<Distance>
	{
	public:
		
		typedef typename Distance::ElementType ElementType;
		typedef typename Distance::ResultType DistanceType;

		typedef NNIndex<Distance> BaseClass;

		typedef utils::KdTreeNode<DistanceType> Node;
		typedef Node* NodePtr;

		/**
			KDTree constructor

			@param params: parameters passed to the kdtree algorithm
			@param d: distance functor
		*/
		KDTreeCudaIndex(const IndexParams& params = KDTreeCudaIndexParams(), Distance d = Distance())
			: BaseClass(params, d), mean_(NULL), var_(NULL), devtreeroots(nullptr), devpool(nullptr), devdataset(nullptr)
		{
			trees_ = get_param(params, "trees", 1);
			
			gpuMemCpyData();
		}

		/**
			KDTree constructor
			
			@param inputData:dataset with the input features
			@param params: parameters passed to the kdtree algorithm
			@param d: distance functor
		*/
		KDTreeCudaIndex(const Matrix<ElementType>& inputData, const IndexParams& params = KDTreeCudaIndexParams(),
			Distance d = Distance()) : BaseClass(params, d), mean_(NULL), var_(NULL), devtreeroots(nullptr), devpool(nullptr), devdataset(nullptr)
		{
			trees_ = get_param(params, "trees", 1);

			setDataset(inputData);
			gpuMemCpyData();
		}

		void gpuMemCpyData();

		/**
			Destructor
		*/
		virtual ~KDTreeCudaIndex()
		{
			gpuDestructor();
			pool_.clear();
		}

		void gpuDestructor();

		/**
			Assignment operator

			@param other: KDTree index
			@return reference to the KDTree index
		*/
		KDTreeCudaIndex& operator=(KDTreeCudaIndex other)
		{
			this->swap(other);
			return *this;
		}

		/**
			Clones this istance 
			
			@return pointer to a instance of the base class which this instance
		*/
		BaseClass* clone() const
		{
			return new KDTreeCudaIndex(*this);
		}

	private:

		/**
			Builds the index
		*/
		using BaseClass::buildIndex;

		flann_algorithm_t getType() const
		{
			return FLANN_INDEX_KDTREE_CUDA;
		}

		/**
		
		*/
		int usedMemory() const
		{
			return 0;
		}

		/**
		
		*/
		void saveIndex(FILE* stream)
		{
			throw FLANNException("Index saving not implemented!");

		}

		/**
		
		*/
		void loadIndex(FILE* stream)
		{
			throw FLANNException("Index loading not implemented!");
		}


		void findNeighbors(ResultSet<DistanceType>& result, const ElementType* vec, const SearchParams& searchParams) const
		{
		}

	protected:
		
		/**
			Build the index
		*/
		void buildIndexImpl()
		{
			pool_ = utils::Allocator(std::pow(2, (std::ceil(std::log2(size_)) + 1)) * trees_, sizeof(Node));

			/* Create a permutable array of indices to the input vectors. */
			std::vector<int> ind(size_);
			for (size_t i = 0; i < size_; ++i) {
				ind[i] = int(i);
			}

			mean_ = new DistanceType[veclen_];
			var_ = new DistanceType[veclen_];

			tree_roots_.resize(trees_);
			/* Construct the randomized trees. */
			for (int i = 0; i < trees_; i++) {
				/* Randomize the order of vectors to allow for unbiased sampling. */
				std::random_shuffle(ind.begin(), ind.end());
				tree_roots_[i] = divideTree(&ind[0], int(size_));
			}
			delete[] mean_;
			delete[] var_;

			gpuMemCpyTrees();
		}

		void gpuMemCpyTrees();

		void freeIndex()
		{
			tree_roots_.clear();
			if (devpool && devtreeroots) {
				gpuFreeIndex();
			}
			if (pool_.ptr()) {
				pool_.clear();
			}
		}

		void gpuFreeIndex();

		/**
			Create a tree node that subdivides the list of vecs from vind[first]
			to vind[last].  The routine is called recursively on each sublist.
			Place a pointer to this new tree node in the location pTree.
		
			@params: pTree = the new node to create
			@params: first = index of the first vector
			@params: last = index of the last vector
		*/
		int divideTree(int* ind, int count)
		{
			int number;
			////////////NodePtr node = new(pool_,number) Node(); // allocate memory
			NodePtr node = (NodePtr) pool_.allocate(number);// allocate memory

			/* If too few exemplars remain, then make this a leaf node. */
			if (count == 1) {
				node->child1 = node->child2 = NULL; /* Mark as leaf node. */
				node->divfeat = *ind; /* Store index of this vec. */
			}
			else {
				int idx;
				int cutfeat;
				DistanceType cutval;
				meanSplit(ind, count, idx, cutfeat, cutval);

				node->divfeat = cutfeat;
				node->divval = cutval;
				node->child1 = divideTree(ind, idx);
				node->child2 = divideTree(ind + idx, count - idx);
			}

			return number;
		}


		/**
			Choose which feature to use in order to subdivide this set of vectors.
			Make a random choice among those with the highest variance, and use
			its variance as the threshold value.
		*/
		void meanSplit(int* ind, int count, int& index, int& cutfeat, DistanceType& cutval)
		{
			memset(mean_, 0, veclen_ * sizeof(DistanceType));
			memset(var_, 0, veclen_ * sizeof(DistanceType));

			/* Compute mean values.  Only the first SAMPLE_MEAN values need to be
			sampled to get a good estimate.
			*/
			int cnt = std::min((int)SAMPLE_MEAN + 1, count);
			for (int j = 0; j < cnt; ++j) {
				ElementType* v = points_[ind[j]];
				for (size_t k = 0; k<veclen_; ++k) {
					mean_[k] += v[k];
				}
			}
			DistanceType div_factor = DistanceType(1) / cnt;
			for (size_t k = 0; k<veclen_; ++k) {
				mean_[k] *= div_factor;
			}

			/* Compute variances (no need to divide by count). */
			for (int j = 0; j < cnt; ++j) {
				ElementType* v = points_[ind[j]];
				for (size_t k = 0; k<veclen_; ++k) {
					DistanceType dist = v[k] - mean_[k];
					var_[k] += dist * dist;
				}
			}
			/* Select one of the highest variance indices at random. */
			cutfeat = selectDivision(var_);
			cutval = mean_[cutfeat];

			int lim1, lim2;
			planeSplit(ind, count, cutfeat, cutval, lim1, lim2);

			if (lim1>count / 2) index = lim1;
			else if (lim2<count / 2) index = lim2;
			else index = count / 2;

			/* If either list is empty, it means that all remaining features
			* are identical. Split in the middle to maintain a balanced tree.
			*/
			if ((lim1 == count) || (lim2 == 0)) index = count / 2;
		}


		/**
			Select the top RAND_DIM largest values from v and return the index of
			one of these selected at random.
		*/
		int selectDivision(DistanceType* v)
		{
			int num = 0;
			size_t topind[RAND_DIM];

			/* Create a list of the indices of the top RAND_DIM values. */
			for (size_t i = 0; i < veclen_; ++i) {
				if ((num < RAND_DIM) || (v[i] > v[topind[num - 1]])) {
					/* Put this element at end of topind. */
					if (num < RAND_DIM) {
						topind[num++] = i;            /* Add to list. */
					}
					else {
						topind[num - 1] = i;         /* Replace last element. */
					}
					/* Bubble end value down to right location by repeated swapping. */
					int j = num - 1;
					while (j > 0 && v[topind[j]] > v[topind[j - 1]]) {
						std::swap(topind[j], topind[j - 1]);
						--j;
					}
				}
			}
			/* Select a random integer in range [0,num-1], and return that index. */
			int idx = 0;
			if (veclen_ > 5) {
				int idx = rand_int(num);
			}
			return (int)topind[idx];
		}


		/**
			Subdivide the list of points by a plane perpendicular on axe corresponding
			to the 'cutfeat' dimension at 'cutval' position.
			
			On return:
			dataset[ind[0..lim1-1]][cutfeat]<cutval
			dataset[ind[lim1..lim2-1]][cutfeat]==cutval
			dataset[ind[lim2..count]][cutfeat]>cutval
		*/
		void planeSplit(int* ind, int count, int cutfeat, DistanceType cutval, int& lim1, int& lim2)
		{
			/* Move vector indices for left subtree to front of list. */
			int left = 0;
			int right = count - 1;
			for (;; ) {
				while (left <= right && points_[ind[left]][cutfeat]<cutval) ++left;
				while (left <= right && points_[ind[right]][cutfeat] >= cutval) --right;
				if (left>right) break;
				std::swap(ind[left], ind[right]); ++left; --right;
			}
			lim1 = left;
			right = count - 1;
			for (;; ) {
				while (left <= right && points_[ind[left]][cutfeat] <= cutval) ++left;
				while (left <= right && points_[ind[right]][cutfeat]>cutval) --right;
				if (left>right) break;
				std::swap(ind[left], ind[right]); ++left; --right;
			}
			lim2 = left;
		}

	public	:

		int knnSearch(const Matrix<ElementType>& queries, 
			Matrix<size_t>& indices, 
			Matrix<DistanceType>& dists, 
			size_t knn, 
			const SearchParams& params) const
		{
			knnSearchGpu(queries, indices, dists, knn, params);
			return knn*queries.rows;
		}

		void knnSearchGpu(const Matrix<ElementType>& queries,
			Matrix<size_t>& indices,
			Matrix<DistanceType>& dists,
			size_t knn,
			const SearchParams& params) const;
		
		//////int knnSearch(const Matrix<ElementType>& queries,
		//////	std::vector< std::vector<int> >& indices,
		//////	std::vector<std::vector<DistanceType> >& dists,
		//////	size_t knn,
		//////	const SearchParams& params) const
		//////{
		//////	knnSearchGpu(queries, indices, dists, knn, params);
		//////	return knn*queries.rows;
		//////}

		//////int knnSearchGpu(const Matrix<ElementType>& queries,
		//////	std::vector< std::vector<int> >& indices,
		//////	std::vector<std::vector<DistanceType> >& dists,
		//////	size_t knn,
		//////	const SearchParams& params) const
		//////{
		//////	flann::Matrix<int> ind(new int[knn*queries.rows], queries.rows, knn);
		//////	flann::Matrix<DistanceType> dist(new DistanceType[knn*queries.rows], queries.rows, knn);
		//////	knnSearchGpu(queries, ind, dist, knn, params);
		//////	for (size_t i = 0; i<queries.rows; i++) {
		//////		indices[i].resize(knn);
		//////		dists[i].resize(knn);
		//////		for (size_t j = 0; j<knn; j++) {
		//////			indices[i][j] = ind[i][j];
		//////			dists[i][j] = dist[i][j];
		//////		}
		//////	}
		//////	delete[] ind.ptr();
		//////	delete[] dist.ptr();
		//////	return knn*queries.rows;
		//////}

	private:
		
		/**
			Swap this KDTree with another KDTree

			@param other KDTree index of a tree
		*/
		void swap(KDTreeCudaIndex& other)
		{
			BaseClass::swap(other);
			std::swap(trees_, other.trees_);
			std::swap(tree_roots_, other.tree_roots_);
			std::swap(pool_, other.pool_);
		}

	private:
		enum
		{
			/**
				To improve efficiency, only SAMPLE_MEAN random values are used to
				compute the mean and variance at each level when building a tree.
				A value of 100 seems to perform as well as using all values.
			*/
			SAMPLE_MEAN = 100,
			/**
				Top random dimensions to consider
				
				When creating random trees, the dimension on which to subdivide is
				selected at random from among the top RAND_DIM dimensions with the
				highest variance.  A value of 5 works well.
			*/
			RAND_DIM = 5
		};

		/**
			Number of randomized trees that are used
		*/
		int trees_;

		DistanceType* mean_;
		DistanceType* var_;

		/**
			Array of k-d trees used to find neighbours.
		*/
		std::vector<int> tree_roots_;

		/**
			Array of k-d trees used to find neighbours on GPU
		*/
		int* devtreeroots;

		/**
			Pooled memory allocator.
		*/
		utils::Allocator pool_;

		/**
			Struct with nodes on GPU
		*/

		Node* devpool;

		/**
			Pointer to the data on GPU
		*/
		ElementType* devdataset;
	};
}

#endif /* FLANN_KDTREE_CUDA_INDEX_H_ */

