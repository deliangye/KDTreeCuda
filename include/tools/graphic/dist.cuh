/***********************************************************************
* Software License Agreement (BSD License)
*
* Copyright 2017  Wolfgang Brandenburger. All rights reserved.
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

#ifndef GRAPHIC_DIST_CUH_
#define GRAPHIC_DIST_CUH_

namespace graphic
{
	template<typename ElementType>
	struct Accumulator { typedef  ElementType Type; };
	template<>
	struct Accumulator<unsigned char> { typedef float Type; };
	template<>
	struct Accumulator<unsigned short> { typedef float Type; };
	template<>
	struct Accumulator<unsigned int> { typedef float Type; };
	template<>
	struct Accumulator<char> { typedef float Type; };
	template<>
	struct Accumulator<short> { typedef float Type; };
	template<>
	struct Accumulator<int> { typedef float Type; };

	/**
	* Squared Euclidean distance functor.
	*
	* This is the simpler, unrolled version. This is preferable for
	* very low dimensionality data (eg 3D points)
	*/
	template<class T>
	struct L2_Simple
	{
		typedef T ElementType;
		typedef typename Accumulator<T>::Type ResultType;

		template <typename Iterator1, typename Iterator2>
		__device__
		ResultType operator()(Iterator1 a, Iterator2 b, int size, ResultType /*worst_dist*/ = -1) const
		{
			ResultType result = ResultType();
			ResultType diff;
			for (int i = 0; i < size; ++i) {
				diff = *a++ - *b++;
				result += diff*diff;
			}
			return result;
		}

		template <typename U, typename V>
		__device__
		inline ResultType accum_dist(const U& a, const V& b, int) const
		{
			return (a - b)*(a - b);
		}
	};

	template<class T>
	struct L2_3D
	{
		typedef T ElementType;
		typedef typename Accumulator<T>::Type ResultType;

		template <typename Iterator1, typename Iterator2>
		__device__
		ResultType operator()(Iterator1 a, Iterator2 b, int size, ResultType /*worst_dist*/ = -1) const
		{
			ResultType result = ResultType();
			ResultType diff;
			diff = *a++ - *b++;
			result += diff*diff;
			diff = *a++ - *b++;
			result += diff*diff;
			diff = *a++ - *b++;
			result += diff*diff;
			return result;
		}

		template <typename U, typename V>
		__device__
		inline ResultType accum_dist(const U& a, const V& b, int) const
		{
			return (a - b)*(a - b);
		}
	};

	/**
	* Squared Euclidean distance functor, optimized version
	*/
	template<class T>
	struct L2
	{
		typedef T ElementType;
		typedef typename Accumulator<T>::Type ResultType;

		/**
		*  Compute the squared Euclidean distance between two vectors.
		*
		*	This is highly optimised, with loop unrolling, as it is one
		*	of the most expensive inner loops.
		*
		*	The computation of squared root at the end is omitted for
		*	efficiency.
		*/
		template <typename Iterator1, typename Iterator2>
		__device__
		ResultType operator()(Iterator1 a, Iterator2 b, int size, ResultType worst_dist = -1)
		{
			ResultType result = ResultType();
			ResultType diff0, diff1, diff2, diff3;
			Iterator1 last = a + size;
			Iterator1 lastgroup = last - 3;

			/* Process 4 items with each loop for efficiency. */
			while (a < lastgroup) {
				diff0 = (ResultType)(a[0] - b[0]);
				diff1 = (ResultType)(a[1] - b[1]);
				diff2 = (ResultType)(a[2] - b[2]);
				diff3 = (ResultType)(a[3] - b[3]);
				result += diff0 * diff0 + diff1 * diff1 + diff2 * diff2 + diff3 * diff3;

				a += 4;
				b += 4;
				if ((worst_dist>0) && (result>worst_dist)) {
					return result;
				}
			}

			/* Process last 0-3 pixels.  Not needed for standard vector lengths. */
			while (a < last) {
				diff0 = (ResultType)(*a++ - *b++);
				result += diff0 * diff0;
			}

			return result;
		}

		/**
		*	Partial euclidean distance, using just one dimension. This is used by the
		*	kd-tree when computing partial distances while traversing the tree.
		*
		*	Squared root is omitted for efficiency.
		*/
		template <typename U, typename V>
		__device__
		inline ResultType accum_dist(const U& a, const V& b, int) const
		{
			return (a - b)*(a - b);
		}
	};


	/*
	* Manhattan distance functor, optimized version
	*/
	template<class T>
	struct L1
	{
		typedef T ElementType;
		typedef typename Accumulator<T>::Type ResultType;

		/**
		*  Compute the Manhattan (L_1) distance between two vectors.
		*
		*	This is highly optimised, with loop unrolling, as it is one
		*	of the most expensive inner loops.
		*/
		template <typename Iterator1, typename Iterator2>
		__device__
		ResultType operator()(Iterator1 a, Iterator2 b, int size, ResultType worst_dist = -1) const
		{
			ResultType result = ResultType();
			ResultType diff0, diff1, diff2, diff3;
			Iterator1 last = a + size;
			Iterator1 lastgroup = last - 3;

			/* Process 4 items with each loop for efficiency. */
			while (a < lastgroup) {
				diff0 = (ResultType)std::abs(a[0] - b[0]);
				diff1 = (ResultType)std::abs(a[1] - b[1]);
				diff2 = (ResultType)std::abs(a[2] - b[2]);
				diff3 = (ResultType)std::abs(a[3] - b[3]);
				result += diff0 + diff1 + diff2 + diff3;
				a += 4;
				b += 4;

				if ((worst_dist>0) && (result>worst_dist)) {
					return result;
				}
			}
			/* Process last 0-3 pixels.  Not needed for standard vector lengths. */
			while (a < last) {
				diff0 = (ResultType)std::abs(*a++ - *b++);
				result += diff0;
			}
			return result;
		}

		/**
		* Partial distance, used by the kd-tree.
		*/
		template <typename U, typename V>
		__device__
		inline ResultType accum_dist(const U& a, const V& b, int) const
		{
			return std::abs(a - b);
		}
	};



}
#endif /* GRAPHIC_DIST_CUH_ */