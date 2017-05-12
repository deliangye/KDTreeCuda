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


#ifndef GRAPHIC_RANDOMIZE_CUH_
#define GRAPHIC_RANDOMIZE_CUH_

#include <curand.h>
#include <curand_kernel.h>

namespace graphic
{
	/**
		Computes a random double value between a higher and a lower bound
		
		@param high upper bound
		@param lowlLower bound
		@return random double value
	*/
	__device__ inline double rand_double(curandState_t state, double high = 1.0, double low = 0)
	{
		unsigned long seed = (unsigned long)clock();
		curand_init(seed, 0, 0, &state);
		return low + (double)((high-low) * (curand_uniform(&state)));
	}

	/**
		Computes a random float value between a higher and a lower bound

		@param high upper bound
		@param lowlLower bound
		@return random float value
	*/
	__device__ inline float rand_float(curandState_t state, float high = 1.0, float low = 0)
	{
		unsigned long seed = (unsigned long)clock();
		curand_init(seed, 0, 0, &state);
		return low + (float)((high - low) * (curand_uniform(&state)));
	}

	/**
		Computes a random integer value between a higher and a lower bound
			
		@param high upper bound
		@param low lower bound
		@return random integer value
	*/
	__device__ inline int rand_int(curandState_t state, int high = RAND_MAX, int low = 0)
	{
		unsigned long seed = (unsigned long)clock();
		curand_init(seed, 0, 0, &state);
		return low + (int)((high - low) * (curand_uniform(&state)));
	}

	/**
		Computes a random value between a higher and a lower bound

		@param high upper bound
		@param low lower bound
		@return random value
	*/
	template<typename ElementType>
	__device__ inline ElementType rand(curandState_t state, ElementType high = 0, ElementType low = 0)
	{
		unsigned long seed = (unsigned long)clock();
		curand_init(seed, 0, 0, &state);

		if (high == 0) {
			if (std::is_same<ElementType, char>::value) {
				high = 127;
			}
			else if (std::is_same<ElementType, unsigned char>::value) {
				high = 255;
			}
			else if (std::is_same<ElementType, short>::value) {
				high = 32767;
			}
			else if (std::is_same<ElementType, unsigned short>::value) {
				high = 65535;
			}
			else if (std::is_same<ElementType, int>::value) {
				high = 2147483647;
			}
			else if (std::is_same<ElementType, unsigned int>::value) {
				high = 4294967295;
			}
			else if (std::is_same<ElementType, size_t>::value) {
				high = 4294967295;
			}
			else if (std::is_same<ElementType, long>::value) {
				high = 2147483647;
			}
			else if (std::is_same<ElementType, unsigned long>::value) {
				high = 4294967295;
			}
			else if (std::is_same<ElementType, long long>::value) {
				high = 9223372036854775807;
			}
			else if (std::is_same<ElementType, unsigned long long>::value) {
				high = 18446744073709551615;
			}
			else if (std::is_same<ElementType, float>::value) {
				high = 1;
			}
			else if (std::is_same<ElementType, double>::value) {
				high = 1;
			}
		}

		return low + (ElementType)((high - low) * (curand_uniform(&state)));
	}
}

#endif /* GRAPHIC_RANDOMIZE_CUH_ */