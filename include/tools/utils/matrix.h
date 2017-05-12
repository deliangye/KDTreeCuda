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

#ifndef UTILS_MATRIX_H_
#define UTILS_MATRIX_H_

namespace utils
{

	template <typename ElementType>
	class Matrix
	{
	public:

		size_t rows;
		size_t cols;
		ElementType* data;

		/**
			Constructor
		*/
		Matrix(void) :
			rows(0), cols(0), data(NULL)
		{
		}

		/**
			Constructor
			
			@param data row-array of a specific Type
			@param rows
			@param cols
		*/
		Matrix(ElementType* data_, size_t rows_, size_t cols_) :
			rows(rows_), cols(cols_), data(data_)
		{
		}

		/**
			Deconstructor
		*/
		~Matrix()
		{
		}

		/**
			Deletes the data array
		*/
		void clear() const
		{
			delete[] data;
		}

		/**
			Returns the pointer of the data array
			
			@return pointer of data array
		*/
		ElementType* getPtr() const
		{
			return (data);
		}

		/**
			Return the pointer of the indexth row
			
			@return pointer of the indexth row
		*/
		inline ElementType* operator[](size_t index) const
		{
			return data + index*cols;
		}

	};

}

#endif UTILS_MATRIX_H_