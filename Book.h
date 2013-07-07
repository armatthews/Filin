#include "Utilities.h"
#include "move.h"

extern bool USE_BOOK;

class book
{
	struct BookData
	{
		int Total;
		vector< move > Moves;
		vector< int > PlayCounts;

		BookData() { Total = 0; }
	};

	std::map< zobrist, BookData > Positions;

public:
	book();
	book( string FileName );
	move ReadMove( ifstream* f );
	void OpenBook( string FileName );
	move GetMove( zobrist Zobrist );
};

#pragma once