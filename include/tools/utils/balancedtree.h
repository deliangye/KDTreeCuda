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

#ifndef UTILS_BALANCEDTREE_H_
#define UTILS_BALANCEDTREE_H_

namespace utils

{
	template <typename ElementType>
	class BalancedTree
	{
	public:

		/**
			Every node stores a value. When the node stores a value which is different from NAN
			occupied becomes true
		*/
		ElementType value;
		bool occupied;

		/**
			Depth is the maximal depth that a child from this node has
		*/
		int depth;

		/**
			Every node has a pointer to each child and a pointer to the parent
		*/
		BalancedTree<ElementType>* parent;
		BalancedTree<ElementType>* nodel;
		BalancedTree<ElementType>* noder;

		/**
			Constructor

			@param pointer to the parent of this node
		*/
		BalancedTree(BalancedTree<ElementType>* parent_) : parent(parent_)
		{
			value = NAN;

			occupied = 0;
			depth = 0;

			nodel = NULL;
			noder = NULL;
		}

		/**
			Deconstructor
		*/
		~BalancedTree(void)
		{
			if (nodel != NULL) { nodel->~BalancedTree(); delete nodel; nodel = NULL;}
			if (noder != NULL) { noder->~BalancedTree(); delete noder; noder = NULL;}
		}

		/**
			Adds a value to the tree
			
			@param root_, pointer to root
			@param value_, the value which should be added
		*/
		void addNode(void** root_, ElementType value_)
		{
			/**
				Call addNode() when the respective node exits, otherwise create a node and define the values
			*/
			if (!occupied) {
				value = value_;
				occupied = 1;
				return;
			}
			else if (value > value_) {
				if (!nodel) {
					nodel = new BalancedTree<ElementType>(this);
					nodel->value = value_;
					nodel->occupied = 1;
				}
				else {
					nodel->addNode(root_, value_);
				}
			}
			else if (value < value_) {
				if (!noder) {
					noder = new BalancedTree<ElementType>(this);
					noder->value = value_;
					noder->occupied = 1;
				}
				else {
					noder->addNode(root_, value_);
				}
			}
			else {
				return;
			}

			/**
				Compute the possibly changed depth and the balancefactor. Rotate the tree if the balancefactor is greater or equal than 2
			*/
			setDepthNode();

			int balanceFactor = getBalanceFactor();

			if (balanceFactor == 2){
				bool rootparent = 0;
				if (!parent) {
					rootparent = 1;
					*root_ = nodel;
				}
				turnRight(root_, rootparent);
				parent->setDepth();
				
			}
			else if (balanceFactor == -2) {
				bool rootparent = 0;
				if (!parent) {
					rootparent = 1;
					*root_ = noder;
				}
				turnLeft(root_, rootparent);
				parent->setDepth();
			}
		}

		/**
			Rotates the node to the right

			@param root_, pointer to root
			@param rootparent_, determines whether the node is involved which points to root
		*/
		void turnRight(void** root_, bool rootparent_) {
			/**
				When the right subtree of the left node has a too large depth the left subtree has to turn left 			
			*/
			int depthl = 0, depthr = 0;
			if (nodel->noder) { depthr = nodel->noder->depth + 1; }
			if (nodel->nodel) { depthl = nodel->nodel->depth + 1; }
			if (depthr > depthl) {
				if (rootparent_ ) {
					*root_ = nodel->noder;
				}
				nodel->turnLeft(root_, rootparent_);
			}
			/**
				Assigning the respective left or right node in the parent the new child
			*/
			if (parent) {
				if (!parent->nodel && parent->noder) {
					parent->noder = nodel;
				}
				else if (parent->nodel && !parent->noder) {
					parent->nodel = nodel;
				}
				else if (parent->nodel && parent->noder) {
					if (parent->nodel->value == value) { parent->nodel = nodel; }
					else if (parent->noder->value == value) { parent->noder = nodel; }
				}
			}
			/**
				Exchange the respective pointers of the involved nodes using the node swap
			*/
			BalancedTree<ElementType>* swap = nodel;
			swap->parent = parent;
			nodel = nodel->noder;
			if (nodel) {
				nodel->parent = this;
			}
			parent = swap;
			swap->noder = this;
		}

		/**
			Rotates the node to the left

			@param root_, pointer to root
			@param rootparent_, determines whether the node is involved which points to root
		*/
		void turnLeft(void** root_, bool rootparent_) {
			/**
				When the left subtree of the right node has a too large depth the right subtree has to turn right
			*/
			int depthl = 0, depthr = 0;
			if (noder->nodel) { depthl = noder->nodel->depth + 1; }
			if (noder->noder) { depthr = noder->noder->depth + 1; }
			if (depthl > depthr) {
				if (rootparent_ ) {
					*root_ = noder->nodel ;
				}
				noder->turnRight(root_, rootparent_);
			}
			/**
				Assigning the respective left or right node in the parent the new child
			*/
			if (parent) {
				if (!parent->nodel && parent->noder) {
					parent->noder = noder;
				}
				else if (parent->nodel && !parent->noder) {
					parent->nodel = noder;
				}
				else if (parent->nodel && parent->noder) {
					if (parent->nodel->value == value) { parent->nodel = noder; }
					else if (parent->noder->value == value) { parent->noder = noder; }
				}
			}
			/**
				Exchange the respective pointers of the involved nodes using the node swap
			*/
			BalancedTree<ElementType>* swap = noder;
			swap->parent = parent;
			noder = noder->nodel;
			if (noder) {
				noder->parent = this;
			}
			parent = swap;
			swap->nodel = this;

		}

		/**
			Computes the balance factor

			@return balancefactor which lies between [-2,2]
		*/
		int getBalanceFactor() {
			if (!nodel && !noder) {
				return 0;
			}	
			else if (!nodel) {
				return -noder->depth - 1;
			}
			else if (!noder) {
				return nodel->depth + 1;
			}
			else {
				return nodel->depth - noder->depth;
			}
		}

public:

		/**
			Computes the number of elements in the tree

			@return count number of elements in the tree
		*/
		void getNumber(int& count)
		{
			if (nodel) { nodel->getNumber(count); }
			if (noder) { noder->getNumber(count); }

			count = count +1;
		}

		/**
			Displays information about ervery node of the tree 
			
			Balancefactor - Depth - Value - Nodel->Value - Noder->Value
		*/
		void getInfos()
		{
			if (nodel) { nodel->getInfos(); }
			if (noder) { noder->getInfos(); }

			if (!nodel && !noder) {
				std::cout << 0 << " " << depth << " " << value << " " << "0" << " " << "0" << " ";

			}
			else if (!nodel && noder) {
				std::cout << -noder->depth - 1 << " " << depth << " " << value << " " << "0" << " " << noder->value << " ";
			}
			else if (nodel && !noder) {
				std::cout << nodel->depth + 1 << " " << depth << " " << value << " " << nodel->value << " " << "0" << " ";
			}
			else {
				std::cout << nodel->depth - noder->depth << " " << depth << " " << value << " " << nodel->value << " " << noder->value << " ";
			}
			
			if (parent) {
				std::cout << parent->value << " " << std::endl;
			}
			else
				std::cout << "0" << std::endl;
		}

		/**
			Checks whether the specification of the depth in every node of the tree is correct

			@return true when the specifications are correct
		*/
		bool checkDepth()
		{
			if (nodel) {
				if (!nodel->checkDepth()) { 
					return 0; 
				}
			}
			if (noder) { 
				if (!noder->checkDepth()) {
					return 0;
				}
			}

			if (!nodel && !noder) {
				if (depth != 0) {
					return 0;
				}
			}
			else if (!nodel) {
				if (depth != noder->depth + 1) {
					return 0;
				}
			}
			else if (!noder) {
				if (depth != nodel->depth + 1) {
					return 0;
				}
			}
			else {
				if (depth != (nodel->depth > noder->depth ? nodel->depth : noder->depth) + 1) {
					return 0;
				}
			}
			
			return 1;
		}

		/**
			Checks whether the left and right node have this as parent

			@return true when the the left and right node have this as parent
		*/
		bool checkParent()
		{
			if (nodel) {
				if (nodel->parent->value == value) {
					return nodel->checkParent();
				}
				else {
					return 0;
				}
			}
			if (noder) {
				if (noder->parent->value == value) {
					return noder->checkParent();
				}
				else {
					return 0;
				}
			}

			return 1;
		}

		/**
			Checks whether the value of the left node is smaller and the value of the right value is bigger than the value of the parent

			@return true when the values are co
		*/
		bool checkRelations()
		{
			 if (nodel) {
				 if (nodel->value > value) {
					 return 0;
				 }
				 else {
					 return nodel->checkRelations();
				 }
			 }
			 if (noder) {
				 if (noder->value < value) {
					 return 0;
				 }
				 else {
					 return noder->checkRelations();
				 }
			 }

			 return 1;
		}

private:

		/**
			Sets the depth of all children of a node
		*/
		void setDepth()
		{
			if (nodel) { nodel->setDepth(); }
			if (noder) { noder->setDepth(); }

			if (!nodel && !noder) {
				depth = 0;
			}
			else if (!nodel) {
				depth = noder->depth + 1;
			}
			else if (!noder) {
				depth = nodel->depth + 1;
			}
			else {
				depth = (nodel->depth > noder->depth ? nodel->depth : noder->depth) + 1;
			}
		}

		/**
			Sets the depth of a node
		*/
		void setDepthNode() 
		{
			if (!nodel && !noder) {
				depth = 0;
			}
			else if (!nodel) {
				depth = noder->depth + 1;
			}
			else if (!noder) {
				depth = nodel->depth + 1;
			}
			else {
				depth = (nodel->depth>noder->depth ? nodel->depth : noder->depth) + 1;
			}
		}

public:

		/**
			Searches for a node with the same value as value_

			@param value_ value which should be searched for in the tree
			@return true when the tree contains a node with the given value
		*/
		bool search(ElementType value_)
		{

			if (value_ == value) { return 1; }

			if (value_ < value && nodel) { return nodel->search(value_); }
			if (value_ > value && noder) { return noder->search(value_); }

			return 0;
		}
	};
}

#endif /* UTILS_BALANCEDTREE_H_ */