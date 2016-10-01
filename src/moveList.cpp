#include "moveList.h"

unsigned int moveList::size()
{
	return ( unsigned int )Data.size();
}

move& moveList::at( unsigned int i )
{
	return Data[ i ];
}

move& moveList::operator[]( unsigned int i )
{
	return at( i );
}

void moveList::Add( move Move )
{
	Data.push_back( Move );
}

void moveList::Remove( unsigned int i )
{
	Data.erase( Data.begin() + i );
}

void moveList::Reserve( unsigned int Size )
{
	Data.reserve( Size );
}

void moveList::Clear()
{
	Data.clear();
}

bool moveList::Contains( move Move )
{
	for( unsigned int i = 0; i < Data.size(); i++ )
		if( Data[ i ] == Move )
			return true;

	return false;
}

bool moveList::Equals( moveList& o )
{
	for( unsigned int i = 0; i < Data.size(); i++ )
		if( !o.Contains( Data[ i ] ) )
		{
			cout << "o doesn't contain " << Data[ i ].toString() << "\n";
			return false;
		}

	for( unsigned int i = 0; i < o.Data.size(); i++ )
		if( !Contains( o.Data[ i ] ) )
		{
			cout << "I don't contain " << o.Data[ i ].toString() << "\n";
			return false;
		}

	return true;
}