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

#ifndef UTILS_POINTCLOUD_H_
#define UTILS_POINTCLOUD_H_

#include "utils/matrix.h"

namespace utils
{

	typedef unsigned char uchar;

	template <typename ElementType>
	class Pointcloud {

	public:

		utils::Matrix<ElementType> points;
		utils::Matrix<uchar> colors;

		size_t rows;
		size_t cols;

		/**
		* Constructor
		*/
		Pointcloud(void)
		{
		}

		/**
		* Constructor
		*
		* @param rows
		* @param cols
		*/
		Pointcloud(size_t rows_, size_t cols_)
		{
			points = utils::Matrix<ElementType>(new ElementType[rows_*cols_], rows_, cols_);

			rows = rows_;
			cols = cols_;
		}

		/**
		* Deconstructor
		*/
		~Pointcloud(void)
		{
		}

		/**
		* Deletes the point and color array
		*/
		void clear() const
		{
			points.clear();
			colors.clear();
		}

		/**
		* Returns the pointer of the data array
		*
		* @return pointer of data array
		*/
		ElementType* getPointsPtr() const
		{
			return points.getPtr();
		}

		/**
		* Sets the data array for the points of the pointcloud
		*
		* @param rows
		* @param cols
		*/
		void setPoints(size_t rows_, size_t cols_)
		{
			points = utils::Matrix<ElementType>(new ElementType[rows_*cols_], rows_, cols_);

			rows = rows_;
			cols = cols_;
		}

		/**
		* Sets the data array for the colorinformation
		*
		* @param rows
		* @param cols
		*/
		void setColors(size_t rows_, size_t cols_)
		{
			colors = utils::Matrix<uchar>(new uchar[rows_*cols_], rows_, cols_);
		}

	};

}

#endif /* UTILS_POINTCLOUD_H_ */