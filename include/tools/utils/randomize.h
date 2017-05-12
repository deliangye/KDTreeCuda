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


#ifndef UTILS_RANDOMIZE_H_
#define UTILS_RANDOMIZE_H_

namespace utils
{
	/**
		Seeds the random number generator
	*/
	inline void rand_seed()
	{
		srand(time(NULL));
	}

	/**
		Computes a random double value between a higher and a lower bound
	 
		@param high upper bound
		@param lowlLower bound
		@return random double value
	*/
	inline double rand_double(double high = 1.0, double low = 0)
	{
		return low + (double)((high-low) * (std::rand() / (RAND_MAX + 1.0)));
	}

	/**
		Computes a random float value between a higher and a lower bound

		@param high upper bound
		@param lowlLower bound
		@return random double value
	*/
	inline double rand_float(float high = 1.0, float low = 0)
	{
		return low + (float)((high - low) * (std::rand() / (RAND_MAX + 1.0)));
	}

	/**
		Computes a random integer value between a higher and a lower bound
		
		@param high upper bound
		@param low lower bound
		@return random integer value
	*/
	inline int rand_int(int high = RAND_MAX, int low = 0)
	{
		return low + (int)((high - low) * (std::rand() / (RAND_MAX + 1.0)));
	}

	template <typename ElementType>
	inline ElementType rand(ElementType high = 0, ElementType low = 0)
	{
		if (high == 0) {
			if (std::is_same<ElementType, char>::value) {
				high = 127;
			}
			else if (std::is_same<ElementType, unsigned char>::value) {
				high = 255;
			}
			else if (std::is_same<ElementType, float>::value) {
				high = 1;
			}
			else if (std::is_same<ElementType, double>::value) {
				high = 1;
			}
			else {
				high = 32767;
			}
		}

		return low + (ElementType)((high - low) * (std::rand() / (RAND_MAX + 1.0)));
	}
}

/**
	You can extend RAND_MAX by selecting two random numbers a and b and compute a*(RAND_MAX+1)+b.
	So that you get numbers between [0,32767^2(1073676289)]. But this is still less than the maximum value 
	of integer (2147483647).

	RAND_MAX is 2^16 = 2 Byte. To get a random number with 2^32 = 4 Byte. You have to generate four 8 bit numbers 
	and shift them with a manifold of 8 bit. 

	ElementType a = 0;
	for ( i = sizeof(ElementType)-1; i>=0; i++){
		a |= rand(unsigned char [0..256]) << i*8;
	}

	When ElementType is a signed data type the random number has to be subtracted with 2^(8*sizeof(ElementType)-1). 
	When the data type is queried there have to be set a flag whether it si a unsigned data type
	
*/

#endif /* UTILS_RANDOMIZE_H_ */