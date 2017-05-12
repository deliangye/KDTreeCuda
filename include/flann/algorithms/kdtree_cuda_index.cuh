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

#ifndef FLANN_KDTREE_CUDA_INDEX_CUH_
#define FLANN_KDTREE_CUDA_INDEX_CUH_

#include "tools/graphic.h"

//#include "tools/graphic/nodes.cuh"

namespace flann
{

	template <typename DistanceType>
	struct Branch {

		int nodeIdx;
		DistanceType mindist;
		DistanceType* dists;

		__device__
		Branch() : nodeIdx(NULL), mindist(NULL), dists(nullptr) {}

		__device__
		Branch(int nodeIdx_, DistanceType mindist_) : nodeIdx(nodeIdx_), mindist(mindist_), dists(nullptr) {}
		
		__device__
		Branch(int nodeIdx_, DistanceType mindist_, DistanceType* dists_) : nodeIdx(nodeIdx_),
			mindist(mindist_), dists(dists_) {}

		__device__
		void clear() 
		{
			delete[] dists;
		}

		__device__
		bool operator<(const Branch<DistanceType> element_) const
		{
			return mindist < element_.mindist;
		}

		__device__
		bool operator>(const Branch<DistanceType> element_) const
		{
			return mindist > element_.mindist;
		}

		__device__
		bool operator==(DistanceType value_) const
		{
			return mindist == value_;
		}

		__device__
		bool operator!=(DistanceType value_) const
		{
			return mindist != value_;
		}

		__device__
		bool operator!() const
		{
			return !mindist;
		}

		__device__
		void operator=(DistanceType value_)
		{
			mindist = value_;
		}
	};

	template <typename Distance>
	class gpuknnSearch {

	public:

		typedef typename Distance::ElementType ElementType;
		typedef typename Distance::ResultType DistanceType;

		typedef utils::KdTreeNode<DistanceType> Node;
		
		/**
			Constructor
		*/
		__host__ 
		__device__
		gpuknnSearch()
		{
			devdataset = nullptr;
			devpool = nullptr;
			devtreeroots = nullptr;
			devqueries = nullptr;
			devindices = nullptr;
			devdists = nullptr;
			/*devHeapNumber = nullptr;*/

			veclen = 0;
			knn = 0;
			size = 0;
			trees = 0;
		}

		/**
			Constructor
		*/
		__host__ 
		__device__
		gpuknnSearch(ElementType* devdataset_, Node* devpool_, int* devtreeroots_, ElementType* devqueries_, size_t* devindices_, DistanceType* devdists_,
			/*size_t* devHeapNumber_,*/
			int veclen_, int size_, int trees_, int knn_)
		{
			devdataset = devdataset_;
			devpool = devpool_;
			devtreeroots = devtreeroots_;
			devqueries = devqueries_;
			devindices = devindices_;
			devdists = devdists_;
			/*devHeapNumber = devHeapNumber_;*/

			knn = knn_;
			veclen = veclen_;
			trees = trees_;
			size = size_;
		}

		/**
			Deconstructor
		*/
		__host__ 
		__device__
		~gpuknnSearch()
		{
		}

		/**
			Generates the essential container for the neighbor search and calls the recursive search

			@param index_ the index of the point which neighbors are searched
		*/
		__device__
		void getNeighbors(int index_)
		{
			graphic::KNNResultSet<DistanceType> resultset(knn);
			
			graphic::Heap<Branch<DistanceType>, 0> heap(trees*knn*(int)(logf(size) / logf(2)));
			
			////////for (int i = 0; i < trees; i++) {
			////////	ElementType* dists = new ElementType[veclen];
			////////	for (int i = 0; i < veclen; i++) {
			////////		dists[i] = 0;
			////////	}

			////////	Branch<DistanceType> initialBranch(devtreeroots[i], 0, dists);
			////////	heap.add(initialBranch);
			////////}

			//ElementType* dists = new ElementType[veclen];
			//for (int i = 0; i < veclen; i++) {
			//	dists[i] = 0;
			//}

			Branch<DistanceType> initialBranch(devtreeroots[0], 0/*, dists*/);
			heap.add(initialBranch);

			ElementType* vec = &devqueries[index_ * veclen];

			Branch<DistanceType> branch;
			while (heap.pop(branch)) {
				searchLevelExact(resultset, heap, branch.nodeIdx, vec, branch.mindist/*, branch.dists*/);
				//branch.clear();
			}
			resultset.copy(&devindices[index_*knn], &devdists[index_*knn]);

			resultset.clear();
			heap.clear();
		}


		__device__
		void searchLevelExact(graphic::KNNResultSet<DistanceType>& resultset_, graphic::Heap<Branch<DistanceType>, 0>& heap_, int nodeIdx_, ElementType* vec_,
			DistanceType mindist_ /*, DistanceType* dists_*/)
		{
			if (resultset_.worstDist() != -1 && mindist_ > resultset_.worstDist()) {
				return;
			}

			if (!devpool[nodeIdx_].child1 && !devpool[nodeIdx_].child2) {
				int idx = devpool[nodeIdx_].divfeat;
					
				DistanceType dist = distanceFunctor(&devdataset[idx*veclen], vec_, veclen);
				resultset_.addPoint(dist, idx);
				return;
			}

			ElementType val = vec_[devpool[nodeIdx_].divfeat];
			ElementType divval = devpool[nodeIdx_].divval;
			DistanceType diff = val - divval;
			int bestchild = (diff < 0) ? devpool[nodeIdx_].child1 : devpool[nodeIdx_].child2;
			int otherchild = (diff < 0) ? devpool[nodeIdx_].child2 : devpool[nodeIdx_].child1;

			//ElementType* bestdistsnew = new ElementType[veclen];
			//for (int i = 0; i < veclen; i++) {
			//	bestdistsnew[i] = dists_[i];
			//}
			Branch<DistanceType> bestbranch(bestchild, mindist_/*, bestdistsnew*/);
			heap_.add(bestbranch);

			DistanceType newDistsq = mindist_ + distanceFunctor.accum_dist(val,divval,veclen) /*- dists_[devpool[nodeIdx_].divfeat]*/;
			if (resultset_.worstDist() == -1 || newDistsq < resultset_.worstDist()) {
				
				//ElementType* otherdistsnew = new ElementType[veclen];
				//for (int i = 0; i < veclen; i++) {
				//	otherdistsnew[i] = dists_[i];
				//}
				//otherdistsnew[devpool[nodeIdx_].divfeat] = distanceFunctor.accum_dist(val, divval, veclen);
				
				Branch<DistanceType> otherbranch(otherchild, newDistsq/*, otherdistsnew*/);
				heap_.add(otherbranch);
			}
		}

	private:

		/**
			Instance of the Distance-Structure
		*/
		Distance distanceFunctor;

		/**
			Pointer to the data on GPU	
		*/
		ElementType* devdataset;

		/**
			Struct with nodes on GPU
		*/
		Node* devpool;

		/**
			Array with indices to the randomized k-d trees 
		*/
		int* devtreeroots;

		/**
			Array with the querypoints
		*/
		ElementType* devqueries;

		/** 
			Array with indices of the nearest neighbors
		*/
		size_t* devindices;
		
		/** 
			Array with the distances to the querypoint of the nearest neighbors
		*/
		DistanceType* devdists;



		/*size_t* devHeapNumber;*/



	public:

		/**
			Number of neighbors which will be searched	
		*/

		int knn;

		/**
			Dimension of data
		*/
		int veclen;
		
		/** 
			Size of data
		*/
		int size;

		/**
			Number of trees
		*/
		int trees;

	};

}

#endif /* FLANN_KDTREE_CUDA_INDEX_CUH_ */