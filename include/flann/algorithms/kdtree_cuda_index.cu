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

#include "kdtree_cuda_index.h"
#include "kdtree_cuda_index.cuh"

#include <flann/algorithms/dist.h>

#include "tools/graphic.h"

namespace flann
{
	template <typename Distance>
	__global__
	void gpuSearch(gpuknnSearch<Distance> search_, int numberofqueries_)
	{
		for (int index = threadIdx.x + blockIdx.x*blockDim.x; index < numberofqueries_; index += gridDim.x*blockDim.x) {
			search_.getNeighbors(index);
		}
	}

	template void KDTreeCudaIndex<flann::L2<float>>::gpuMemCpyTrees();
	template void KDTreeCudaIndex<flann::L2<double>>::gpuMemCpyTrees();

	template <typename Distance> void KDTreeCudaIndex<Distance>::gpuMemCpyTrees()
	{
		HANDLE_ERROR(cudaMalloc((void**)&devtreeroots, tree_roots_.size() * sizeof(int)));
		HANDLE_ERROR(cudaMemcpy(devtreeroots, tree_roots_.data(), tree_roots_.size() * sizeof(int), cudaMemcpyHostToDevice));

		HANDLE_ERROR(cudaMalloc((void**)&devpool, std::pow(2, (std::ceil(std::log2(size_)) + 1)) * trees_ * sizeof(KDTreeCudaIndex<Distance>::Node)));
		HANDLE_ERROR(cudaMemcpy(devpool, pool_.base, std::pow(2, (std::ceil(std::log2(size_)) + 1)) * trees_ * sizeof(KDTreeCudaIndex<Distance>::Node), cudaMemcpyHostToDevice));
	}

	template void KDTreeCudaIndex<flann::L2<float>>::gpuMemCpyData();
	template void KDTreeCudaIndex<flann::L2<double>>::gpuMemCpyData();

	template <typename Distance> void KDTreeCudaIndex<Distance>::gpuMemCpyData()
	{
		HANDLE_ERROR(cudaMalloc((void**)&devdataset, size_ * veclen_ * sizeof(ElementType)));
		HANDLE_ERROR(cudaMemcpy(devdataset, points_[0], size_ * veclen_ * sizeof(ElementType), cudaMemcpyHostToDevice));
	}

	template void KDTreeCudaIndex<flann::L2<float>>::knnSearchGpu(const Matrix<ElementType>& queries,
		Matrix<size_t>& indices,
		Matrix<DistanceType>& dists,
		size_t knn,
		const SearchParams& params) const;
	template void KDTreeCudaIndex<flann::L2<double>>::knnSearchGpu(const Matrix<ElementType>& queries,
		Matrix<size_t>& indices,
		Matrix<DistanceType>& dists,
		size_t knn,
		const SearchParams& params) const;

	template<typename Distance>
	void knnSearchGpuKernel(gpuknnSearch<Distance> search_, int numberofqueries_)
	{
		typedef typename Distance::ElementType ElementType;

		////cudaDeviceProp prop;
		////int dev;		
		////HANDLE_ERROR(cudaGetDevice(&dev));
		////printf("ID of current CUDA device: %d\n", dev);
		////HANDLE_ERROR(cudaSetDevice(1)); // DETERMINE THE BEST GRAPHICCARD
		////HANDLE_ERROR(cudaGetDevice(&dev));
		////printf("ID of current CUDA device: %d\n", dev);

		//HANDLE_ERROR(cudaDeviceSetLimit(cudaLimitMallocHeapSize, std::pow(2, 18)*search_.knn*search_.trees*std::log(search_.size) / std::log(2) * sizeof(ElementType)));
		//std::cout << std::pow(2, 17)*search_.knn*search_.trees*std::log(search_.size) / std::log(2) * sizeof(ElementType) << std::endl;

		//cudaPrintfInit();
		dim3 grid(28*32, 1);
		dim3 block(256, 1);
		gpuSearch<Distance> << <grid, block >> > (search_, numberofqueries_);
		//cudaPrintfDisplay(stdout, true);
		//cudaPrintfEnd();
	}

	template <typename Distance>
	void KDTreeCudaIndex<Distance>::knnSearchGpu(const Matrix<ElementType>& queries,
		Matrix<size_t>& indices,
		Matrix<DistanceType>& dists,
		size_t knn,
		const SearchParams& params) const
	{
		ElementType* devqueries;
		size_t* devindices;
		DistanceType* devdists;

		HANDLE_ERROR(cudaMalloc((void**)&devqueries, queries.rows * queries.cols * sizeof(ElementType)));
		HANDLE_ERROR(cudaMemcpy(devqueries, queries.ptr(), queries.rows * queries.cols * sizeof(ElementType), cudaMemcpyHostToDevice));

		HANDLE_ERROR(cudaMalloc((void**)&devindices, indices.rows * indices.cols * sizeof(size_t)));
		HANDLE_ERROR(cudaMemcpy(devindices, indices.ptr(), indices.rows * indices.cols * sizeof(size_t), cudaMemcpyHostToDevice));

		HANDLE_ERROR(cudaMalloc((void**)&devdists, dists.rows * dists.cols * sizeof(DistanceType)));
		HANDLE_ERROR(cudaMemcpy(devdists, dists.ptr(), dists.rows * dists.cols * sizeof(DistanceType), cudaMemcpyHostToDevice));

		//size_t* devHeapNumber;
		//size_t* heapNumber(new size_t[queries.rows]);
		//for (int i = 0; i < queries.rows; i++) {
		//	heapNumber[i] = 0;
		//}
		//HANDLE_ERROR(cudaMalloc((void**)&devHeapNumber, queries.rows * sizeof(size_t)));
		//HANDLE_ERROR(cudaMemcpy(devHeapNumber, heapNumber, queries.rows * sizeof(size_t), cudaMemcpyHostToDevice));

		if (std::is_same<Distance, flann::L2<ElementType>>::value) {
			typedef graphic::L2<ElementType> DistanceGpu;
			gpuknnSearch<DistanceGpu> search(devdataset, devpool, devtreeroots, devqueries, devindices, devdists/*, devHeapNumber*/, veclen_, size_, tree_roots_.size(), knn);
			knnSearchGpuKernel<DistanceGpu>(search, queries.rows);
			//versuchKernelCall<ElementType, DistanceType>(devtreeroots, trees_, veclen_, size_, devpool, devdataset);
		}
		else if (std::is_same<Distance, flann::L2_3D<ElementType>>::value) {
			typedef graphic::L2_3D<ElementType> DistanceGpu;
			gpuknnSearch<DistanceGpu> search(devdataset, devpool, devtreeroots, devqueries, devindices, devdists/*, devHeapNumber*/, veclen_, size_, tree_roots_.size(), knn);
			knnSearchGpuKernel<DistanceGpu>(search, queries.rows);
			//versuchKernelCall<ElementType,DistanceType>(devtreeroots, trees_, veclen_, size_, devpool, devdataset);
		}
		else if (std::is_same<Distance, flann::L2_Simple<ElementType>>::value) {
			typedef graphic::L2_Simple<ElementType> DistanceGpu;
			gpuknnSearch<DistanceGpu> search(devdataset, devpool, devtreeroots, devqueries, devindices, devdists/*, devHeapNumber*/, veclen_, size_, tree_roots_.size(), knn);
			knnSearchGpuKernel<DistanceGpu>(search, queries.rows);
			//versuchKernelCall<ElementType, DistanceType>(devtreeroots, trees_, veclen_, size_, devpool, devdataset);
		}

		HANDLE_ERROR(cudaMemcpy(indices.ptr(), devindices, indices.rows * indices.cols * sizeof(size_t), cudaMemcpyDeviceToHost));
		HANDLE_ERROR(cudaMemcpy(dists.ptr(),devdists,dists.rows * dists.cols * sizeof(DistanceType), cudaMemcpyDeviceToHost));

		//HANDLE_ERROR(cudaMemcpy(heapNumber,devHeapNumber,queries.rows*sizeof(size_t),cudaMemcpyDeviceToHost));

		HANDLE_ERROR(cudaFree(devqueries));
		HANDLE_ERROR(cudaFree(devindices));
		HANDLE_ERROR(cudaFree(devdists));
	}

	template void KDTreeCudaIndex<flann::L2<float>>::gpuDestructor();
	template void KDTreeCudaIndex<flann::L2<double>>::gpuDestructor();

	template <typename Distance > void KDTreeCudaIndex<Distance>::gpuDestructor()
	{
		HANDLE_ERROR(cudaFree(devpool));
		devpool = nullptr;
		HANDLE_ERROR(cudaFree(devdataset));
		devdataset = nullptr;
		HANDLE_ERROR(cudaFree(devtreeroots));
		devtreeroots = nullptr;
	}

	template void KDTreeCudaIndex<flann::L2<float>>::gpuFreeIndex();
	template void KDTreeCudaIndex<flann::L2<double>>::gpuFreeIndex();

	template <typename Distance > void KDTreeCudaIndex<Distance>::gpuFreeIndex()
	{
		HANDLE_ERROR(cudaFree(devpool));
		devpool = nullptr;
		HANDLE_ERROR(cudaFree(devtreeroots));
		devtreeroots = nullptr;
	}
}

