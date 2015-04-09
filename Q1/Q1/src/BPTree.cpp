#include <BPTree.hpp>

#include <exception>

using namespace Q1;

BPTree::BPTree( uint32_t d ): m_d( d ) {

}

BPTree::~BPTree() {

}

void BPTree::insert( int32_t data ) {
	auto desiredLeafNode = m_root;

	if( ! desiredLeafNode ) {
		m_root = nodePtr_t( new BPTreeNode( nodePtr_t() ) );
		m_root->m_data.push_back( data );
		return;
	}

	//Find the leaf node where the new entry 'belongs'
	while( ! desiredLeafNode->isLeaf() ) {
		BPTreeIndexNode *currentIndexNode = dynamic_cast<BPTreeIndexNode *>( desiredLeafNode.get() );
		if ( ! currentIndexNode ) {
			throw std::exception( "Failed to cast node into index node" );
		}

		auto it = currentIndexNode->m_indexes.begin();
		auto end = currentIndexNode->m_indexes.end();
		std::size_t currentIndex = 0;
		for( it; it != end && currentIndexNode->m_data[currentIndex] < data; ++it, ++currentIndex );

		desiredLeafNode = currentIndexNode->m_indexes[currentIndex];

		//Check if this data has already been inserted
		if( desiredLeafNode->isLeaf() ) {
			for( auto it = desiredLeafNode->m_data.begin(); it != desiredLeafNode->m_data.end(); ++it ) {
				if( *it == data ) {
					return;
				}
			}
		}
	}

	desiredLeafNode->m_data.push_back( data );

	uint32_t twoD = 2 * m_d;

	if( desiredLeafNode->m_data.size() > twoD ) {

		auto currentNode = desiredLeafNode;

		//Loop until the current node is either the root, or has 2 * m_d or less entries
		while( currentNode->m_data.size() >= twoD && !currentNode->m_parent ) {
			BPTreeIndexNode *currentIndexNode = dynamic_cast< BPTreeIndexNode * >( currentNode.get() );
			auto parentUncast = currentNode->m_parent;
			BPTreeIndexNode *parent = dynamic_cast< BPTreeIndexNode * >( parentUncast.get() );

			if( currentIndexNode ) {
				//The current node is not a leaf

				std::shared_ptr<BPTreeIndexNode> newSiblingIndex;
				nodePtr_t newSibling = newSiblingIndex;

				uint32_t copyUp = currentNode->m_data[m_d];

				for( std::size_t i = m_d + 1; i < currentNode->m_data.size(); ++i ) {
					newSibling->m_data.push_back( currentNode->m_data[i] );
				}
				currentNode->m_data.erase( currentNode->m_data.begin() + m_d, currentNode->m_data.end() );

				for( std::size_t i = m_d; i < currentIndexNode->m_indexes.size(); ++i ) {
					newSiblingIndex->m_indexes.push_back( currentIndexNode->m_indexes[i] );
				}
				currentIndexNode->m_indexes.erase( currentIndexNode->m_indexes.begin() + m_d, currentIndexNode->m_indexes.end() );

				std::size_t locInParent = 0;

				for( ; locInParent < parent->m_data.size() && parent->m_data[locInParent] < data; ++locInParent );

				parent->m_indexes.insert( parent->m_indexes.begin() + locInParent + 1, newSibling );

				parent->m_data.insert( parent->m_data.begin() + locInParent, copyUp );
			} else {
				//The current node is a leaf

				nodePtr_t newSibling;
				for( std::size_t i = m_d; i < currentNode->m_data.size(); ++i ) {
					newSibling->m_data.push_back( currentNode->m_data[i] );
				}
				currentNode->m_data.erase( currentNode->m_data.begin() + m_d, currentNode->m_data.end() );

				std::size_t locInParent = 0;

				for( ; locInParent < parent->m_data.size() && parent->m_data[locInParent] < data; ++locInParent );

				parent->m_indexes.insert( parent->m_indexes.begin() + locInParent + 1, newSibling );

				parent->m_data.insert( parent->m_data.begin() + locInParent, newSibling->m_data[0] );
			}

			currentNode = parentUncast;
		}

		if( !currentNode->m_parent && currentNode->m_data.size() > twoD ) {
			//It appears we must split the root
			BPTreeIndexNode *currentIndexNode = dynamic_cast< BPTreeIndexNode * >(currentNode.get());

			if( currentIndexNode ) {
				//The root is not a leaf node
				std::shared_ptr<BPTreeIndexNode> newRoot;
				newRoot->m_data.push_back( currentNode->m_data[m_d] );
				newRoot->m_indexes.push_back( currentNode );
				std::shared_ptr<BPTreeIndexNode> newSibling;

				for( std::size_t i = m_d + 1; i < currentNode->m_data.size(); ++i ) {
					newSibling->m_data.push_back( currentNode->m_data[i] );
				}
				currentNode->m_data.erase( currentNode->m_data.begin() + m_d, currentNode->m_data.end() );
				newRoot->m_indexes.push_back( newSibling );
				m_root = newRoot;
			} else {
				//The root is a leaf node
				std::shared_ptr<BPTreeIndexNode> newRoot;
				newRoot->m_data.push_back( currentNode->m_data[m_d] );
				newRoot->m_indexes.push_back( currentNode );
				auto newSibling = nodePtr_t();

				for( std::size_t i = m_d; i < currentNode->m_data.size(); ++i ) {
					newSibling->m_data.push_back( currentNode->m_data[i] );
				}
				currentNode->m_data.erase( currentNode->m_data.begin() + m_d, currentNode->m_data.end() );
				newRoot->m_indexes.push_back( newSibling );
				m_root = newRoot;
			}
		}
	}

}
void BPTree::del( int32_t data ) {
	auto desiredLeafNode = m_root;

	if( !desiredLeafNode ) {
		//Nothing to delete
		return;
	}

	//Find the leaf node where the new entry 'belongs'
	std::vector<int32_t>::iterator condemedIt;

	while( !desiredLeafNode->isLeaf() ) {
		BPTreeIndexNode *currentIndexNode = dynamic_cast<BPTreeIndexNode *>(desiredLeafNode.get());
		if( !currentIndexNode ) {
			throw std::exception( "Failed to cast node into index node" );
		}

		auto it = currentIndexNode->m_indexes.begin();
		auto end = currentIndexNode->m_indexes.end();
		std::size_t currentIndex = 0;
		for( it; it != end && currentIndexNode->m_data[currentIndex] < data; ++it, ++currentIndex );

		desiredLeafNode = currentIndexNode->m_indexes[currentIndex];

		//Check if this data actually exists
		
		if( desiredLeafNode->isLeaf() ) {
			bool found = false;
			for( auto it = desiredLeafNode->m_data.begin(); it != desiredLeafNode->m_data.end(); ++it ) {
				if( *it == data ) {
					condemedIt = it;
					found = true;
				}
			}

			if( !found ) {
				return;
			}
		}
	}

	desiredLeafNode->m_data.erase( condemedIt );
	nodePtr_t currentNode = desiredLeafNode;

	while( currentNode->m_data.size() < m_d && currentNode->m_parent ) {
		//First we'll try to redistribute with adjacent siblings
		nodePtr_t uncastParent = currentNode->m_parent;
		BPTreeIndexNode *parent = dynamic_cast< BPTreeIndexNode * >( uncastParent.get() );

		auto it = parent->m_indexes.begin();
		auto end = parent->m_indexes.end();
		std::size_t currentIndex = 0;
		for( it; it != end && parent->m_data[currentIndex] < data; ++it, ++currentIndex );

		if( currentIndex > 0 ) {
			//There's a left sibling
			nodePtr_t siblingUncast = parent->m_indexes[currentIndex - 1];
			if( siblingUncast->m_data.size() > m_d ) {
				if( !currentNode->isLeaf() ) {
					BPTreeIndexNode *currentIndexNode = dynamic_cast<BPTreeIndexNode *>( currentNode.get() );
					BPTreeIndexNode *siblingIndexNode = dynamic_cast< BPTreeIndexNode * >(siblingUncast.get());

					currentIndexNode->m_indexes.insert( currentIndexNode->m_indexes.begin(), siblingIndexNode->m_indexes[siblingIndexNode->m_indexes.size() - 1] );
					siblingIndexNode->m_indexes.pop_back();
				}

				currentNode->m_data.insert( currentNode->m_data.begin(), siblingUncast->m_data[siblingUncast->m_data.size() - 1] );
				siblingUncast->m_data.pop_back();

				return;
			}
		}

		if( currentIndex < parent->m_data.size() - 1 ) {
			//There's a right sibling
			nodePtr_t siblingUncast = parent->m_indexes[currentIndex + 1];
			if( siblingUncast->m_data.size() > m_d ) {
				if( !currentNode->isLeaf() ) {
					BPTreeIndexNode *currentIndexNode = dynamic_cast<BPTreeIndexNode *>(currentNode.get());
					BPTreeIndexNode *siblingIndexNode = dynamic_cast< BPTreeIndexNode * >(siblingUncast.get());

					currentIndexNode->m_indexes.push_back( siblingIndexNode->m_indexes[0] );
					siblingIndexNode->m_indexes.erase( siblingIndexNode->m_indexes.begin() );
				}

				currentNode->m_data.push_back( siblingUncast->m_data[0] );
				siblingUncast->m_data.erase( siblingUncast->m_data.begin() );

				return;
			}
		}

		//There wasn't a sibling we could borrow from, so we need to do a merge
		if( currentIndex > 0 ) {
			//There's a left sibling
			nodePtr_t siblingUncast = parent->m_indexes[currentIndex - 1];
			if( !currentNode->isLeaf() ) {
				BPTreeIndexNode *currentIndexNode = dynamic_cast<BPTreeIndexNode *>(currentNode.get());
				BPTreeIndexNode *siblingIndexNode = dynamic_cast< BPTreeIndexNode * >(siblingUncast.get());

				for( std::size_t i = 0; i < currentIndexNode->m_indexes.size(); ++i ) {
					siblingIndexNode->m_indexes.push_back( currentIndexNode->m_indexes[i] );
				}
			}

			for( std::size_t i = 0; i < currentNode->m_data.size(); ++i ) {
				siblingUncast->m_data.push_back( currentNode->m_data[i] );
			}

			parent->m_indexes.erase( parent->m_indexes.begin() + currentIndex );

			if( !parent->m_parent && parent->m_data.size() == 0 ) {
				//Parent is root but won't be for long
				m_root = siblingUncast;
				siblingUncast->m_parent = nodePtr_t();
				return;
			}
		}

		if( currentIndex < parent->m_data.size() - 1 ) {
			//There's a right sibling
			nodePtr_t siblingUncast = parent->m_indexes[currentIndex + 1];
			if( !currentNode->isLeaf() ) {
				BPTreeIndexNode *currentIndexNode = dynamic_cast<BPTreeIndexNode *>(currentNode.get());
				BPTreeIndexNode *siblingIndexNode = dynamic_cast< BPTreeIndexNode * >(siblingUncast.get());

				for( std::size_t i = 0; i < siblingIndexNode->m_indexes.size(); ++i ) {
					currentIndexNode->m_indexes.push_back( siblingIndexNode->m_indexes[i] );
				}
			}

			for( std::size_t i = 0; i < siblingUncast->m_data.size(); ++i ) {
				currentNode->m_data.push_back( siblingUncast->m_data[i] );
			}

			parent->m_indexes.erase( parent->m_indexes.begin() + (currentIndex + 1) );

			if( !parent->m_parent && parent->m_data.size() == 0 ) {
				//Parent is root but won't be for long
				m_root = currentNode;
				currentNode->m_parent = nodePtr_t();
				return;
			}
		}
		
		currentNode = uncastParent;
	}

	
}

BPTreeNode::BPTreeNode( BPTree::nodePtr_t parent ): m_parent( parent ) {

}

BPTreeNode::~BPTreeNode() {

}

bool BPTreeNode::isLeaf() {
	return true;
}

BPTreeIndexNode::BPTreeIndexNode( BPTree::nodePtr_t parent ) : BPTreeNode( parent ) {

}

BPTreeIndexNode::~BPTreeIndexNode() {

}

bool BPTreeIndexNode::isLeaf() {
	return false;
}