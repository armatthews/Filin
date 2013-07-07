#include "move.h"
#pragma once

class Node;

class moveList
{
	private:
		vector< move > Data;

	public:
		unsigned int size();
		move& at( unsigned int i );
		move& operator[]( unsigned int i );
		void Add( move Move );
		void Remove( unsigned int i );
		void Reserve( unsigned int Size );
		void Clear();
		bool Contains( move Move );

		bool Equals( moveList& o );
};

#pragma once