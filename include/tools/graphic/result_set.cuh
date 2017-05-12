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

#ifndef GRAPHIC_RESULT_SET_CUH_
#define GRAPHIC_RESULT_SET_CUH_


namespace graphic
{
	template <typename DistanceType>
	struct DistanceIndex
	{
		/**
			Constructor
		*/
		__device__
		DistanceIndex()
		{
			dist = NULL;
			index = NULL;
		}

		/** 
			Constructor

			@param dist_ input value for dist
			@param index_ input value for index
		*/
		__device__
		DistanceIndex(DistanceType dist_, size_t index_) :
			dist(dist_), index(index_)
		{
		}

		/**
			Overloaded operator <

			@param dist_index_ instance of DistanceIndex
			@return true when dist < dist_index_.dist
		*/
		__device__
		bool operator<(const DistanceIndex& dist_index_) const
		{
			return (dist < dist_index_.dist) || ((dist == dist_index_.dist) && index < dist_index_.index);
		}

		DistanceType dist;
		size_t index;
	};

	template <typename DistanceType>
	class KNNResultSet
	{

	public:

		/**
			Constructor
					
			@param capacity_ number of elements in container ResultSet
		*/
		__device__
		KNNResultSet(size_t capacity_) : capacity(capacity_)
		{
			dist_index = new DistanceIndex<DistanceType>[capacity];
			worst_distance = -1;
			count = 0;
		}

		/**
			Destroys the container dist_index
		*/
		__device__
		void clear()
		{
			delete[] dist_index;
			worst_distance = -1;
			count = 0;
		}

		/**
			True when container has as many elements as capacity
		*/
		__device__
		bool full() const
		{
			return count == capacity;
		}

		/**
			Return the current number of elements in container

			@return count the current number of elements in container
		*/
		__device__
		size_t size() const
		{
			return count;
		}

		/**
			Adds elements to the container
		*/
		__device__
		void addPoint(DistanceType dist_, size_t index_) 
		{	
			if (worst_distance == -1 || dist_ < worst_distance) {

				if (count < capacity) {
					count++;
				}
				int i;
				for (i = count - 1; i > 0; --i) {

					if (dist_index[i - 1].dist > dist_) {
						dist_index[i] = dist_index[i - 1];
					}
					else {
						break;
					}
				}

				dist_index[i].dist = dist_;
				dist_index[i].index = index_;

				if (full()) {
					worst_distance = dist_index[capacity - 1].dist;
				}
			}
		}
		
		/**
			Returns the indices and distances of the elements

			@param indices_ pointer to a array with capability elements
			@parma dists_ pointer to array with capability elements
			@return indices_ pointer to a array with the indices of the elements
			@return dists_ pointer to a array with the distances of the elements
		*/
		__device__ 
		void copy(size_t* indices_, DistanceType* dists_)
		{
			for (size_t i = 0; i < capacity; i++) {
				*indices_++ = dist_index[i].index;
				*dists_++ = dist_index[i].dist;
			}
		}

		/** 
			Returns the current worst distance

			@return worst_distance the current worst distance
		*/
		__device__
		DistanceType worstDist() const 
		{
			return worst_distance;
		}

	private:
		/**
			Current number of elements in container
		*/
		size_t count;

		/**
			Current worst distance
		*/
		DistanceType worst_distance;

		/**
			Pointer to the elements of ResultSet
		*/
		DistanceIndex<DistanceType>* dist_index;
		
		/**
			Number of elements in container ResultSet
		*/
		size_t capacity;
	};

}

#endif /* GRAPHIC_RESULT_SET_CUH_*/