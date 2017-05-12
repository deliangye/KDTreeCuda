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

#ifndef UTILS_ALLOCATOR_H_
#define UTILS_ALLOCATOR_H_

#ifdef _DEBUG
	#ifndef DEBUG_NEW
		#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
		#define new DEBUG_NEW
	#endif
#endif

namespace utils

{
	class Allocator {
	
	public:
		size_t size;
		size_t chunk;
		int number;


		void* base;
		void* current;

		/**
			Constructor
		*/
		Allocator()
		{
			size = 0;
			chunk = 0;
			number = 0;

			base = nullptr;
			current = nullptr;
		}

		/**
			Constructor

			@param size_ number of elements
			@param chunk_ size of the elements in Bytes
		*/
		Allocator(size_t size_, size_t chunk_) 
		{
			size = size_;
			chunk = chunk_;
			number = 0;

			base = (void*) new char[size*chunk];
			current = base;
		}

		 /**
			Deconstructor
		 */
		~Allocator() {}
		
		/**
			Return a pointer to a memory area that can be used

			@param chunk_ size of the element which is required in Bytes
			@return pointer to the memory area
		*/
		void* allocate()
		{
			if (number == size) {
				std::cout << __FILE__ << " Line " << __LINE__ << " " << number << std::endl;
				exit(EXIT_FAILURE);
			}

			void* pointer = current;
			current = (char*)current + chunk;

			number = number + 1;
			return pointer;
		}

		/**
			Return a pointer to a memory area that can be used

			@param chunk_ size of the element which is required in Bytes
			@param number_ integer which specifies the location of the pointer
			@return pointer to the memory area
			@return number_ integer which specifies the location of the pointer
		*/
		void* allocate(int& number_) 
		{

			if (number == size) {
				std::cout << __FILE__ << " Line " << __LINE__ << " " << number << std::endl;
				exit(EXIT_FAILURE);
			}

			void* pointer = current;
			current = (char*)current + chunk;

			number_ = number;
			number = number + 1;
			return pointer;
		}

		/**
			Free memory
		*/
		void clear()
		{
			size = 0;
			chunk = 0;
			number = 0;
			
			delete[] base;
			base = nullptr;
			current = nullptr;
		}

		/**
			Shows how much memory is used

			@return number of Bytes that are used by this object
		*/
		size_t usedMemory() 
		{
			return number*chunk;
		}

		/**
			Shows how much memory is remaining

			@return number of Bytes that could be used
		*/
		size_t remainedMemory()
		{
			return (size - number)*chunk;
		}

		inline void* operator[](int number_)
		{
			return ((char*)base + chunk*number_);
		}

		void* ptr()
		{
			return base;
		}
	};

}

/////////**
////////	Overloading of the operator new - this operator offers a pointer to the allocated memory
////////
////////	@param allocator_ object which offers the allocated memory
////////	@return pointer to the respective memory
////////*/
////////inline void* operator new(size_t size_, utils::Allocator& allocator_) 
////////{
////////	return allocator_.allocate();
////////}
////////
/////////**
////////	Overloading of the operator new - this operator offers a pointer to the allocated memory
////////
////////	@param allocator_ object which offers the allocated memory
////////	@param number_ integer which specifies the location of the pointer
////////	@return pointer to the respective memory
////////	@return number_ integer which specifies the location of the pointer
////////*/
////////inline void* operator new(size_t size_, utils::Allocator& allocator_, int& number_)
////////{
////////	return allocator_.allocate(number_);
////////}

#endif /* UTILS_ALLOCATOR_H_ */