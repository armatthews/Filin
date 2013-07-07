#include "Utilities.h"

class position;

/*
LSB                              MSB
******|******|**|*|*****************
 From |  To  |Pr|N|
******|******|**|*|*****************

From (6)
To (6)
Promote (2)
Null Flag (1)
15 bits

Captured (3)
EPSquare (3)
Castling (4)
Fifty (7)
17 bits
*/

#pragma pack(1)
class move
{
public:
	long Data : 32;
	move();
	move( string Coords );
	move( string SAN, position* Position );
	move( square f, square t, type p );

	string toString() const;
	string toString( position* Position ) const;

	bool operator== ( const move& o ) const;
	bool operator!= ( const move& o ) const;

	square From() const;
	square To() const;
	type Promote() const;

	void SetFrom( square from );
	void SetTo( square to );
	void SetPromote( type promote );

private:
	void SetToNull();

	vector< square > PiecesOfTypeThatCanMoveToSquare( position* Position, type Type, square To ) const;
	bool DoesMoveCheck( position* Position ) const;
	bool CanDistinguishByFile( vector< square >* Squares ) const;
	bool CanDistinguishByRank( vector< square >* Squares ) const;

	int GetPiece( char c, type& t );
};

enum { CHECK = 1, CAPTURE = 2, PROMOTION = 4, OO = 8, OOO = 16 };

class takeback : public move
{
public:
	type Captured;
	square EPSquare;
	int Castling;
	int Fifty;
};

const move NullMove( NullSquare, NullSquare, Empty );


typedef vector< move > MoveList;

typedef vector< move > line;
string LineToString( line& Line );
string LineToString( vector< takeback >& Line );
string LineToString( line& Line, position* Position );
string LineToString( vector< takeback >& Line, position* Position );

#pragma once