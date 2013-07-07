#include <stdlib.h>
#include "Book.h"

bool USE_BOOK = false;
#define ReadInt( f, x ) (f).read( ( char* )( &(x) ), 4 )
#define ReadInt64( f, x ) (f).read( ( char* )( &(x) ), 8 )
#define ReadChar( f, x ) (f).read( ( char* )( &(x) ), 1 )

book::book()
{
}

book::book( string FileName )
{
	OpenBook( FileName );
}

move book::ReadMove( ifstream* f )
{
	char from, to, promote;
	ReadChar( *f, from );
	ReadChar( *f, to );

	ReadChar( *f, promote );

	return move( from, to, promote );
}

void book::OpenBook( string s )
{
	if( ! USE_BOOK )
		return;

	std::ifstream f;
	f.open( s.c_str(), std::ios::in | std::ios::binary );

	assert( f.is_open() );

	int NumPositions;
	ReadInt( f, NumPositions );

	for( int i = 0; i < NumPositions; i++ )
	{
		zobrist Key;
		ReadInt64( f, Key );

		int Total;
		ReadInt( f, Total );

		Positions[ Key ].Total = Total;

		int NumMoves;
		ReadInt( f, NumMoves );

		Positions[ Key ].Moves.resize( NumMoves );
		Positions[ Key ].PlayCounts.resize( NumMoves );

		for( int j = 0; j < NumMoves; j++ )
		{
			Positions[ Key ].Moves[ j ] = ReadMove( &f );

			int plays;
			ReadInt( f, plays );
			Positions[ Key ].PlayCounts[ j ] = plays;
		}
	}

	f.close();
}

move book::GetMove( zobrist Zobrist )
{
	if( !USE_BOOK )
		return NullMove;

	BookData data = Positions[ Zobrist ];
	if( data.Total == 0 ) return NullMove;
	unsigned int Rand = ( ( rand() << 16 ) | rand() ) % data.Total;

	unsigned int sum = 0;
	for( unsigned int i = 0; i < data.Moves.size(); i++ )
	{
		sum += data.PlayCounts[ i ];
		if( sum > Rand )
			return data.Moves[ i ];
	}

	assert( 0 );
	return NullMove;
}
