#include "Utilities.h"
#include "move.h"

class position
{
public:
	bitboard BothPieces;

	bitboard AllPieces[ 2 ];
	bitboard Pieces[ 2 ][ 6 ];

	square EPSquare;
	int Castling;
	int Fifty;
	unsigned int MoveNumber;

	color ToMove;
	color Enemy;

	zobrist Zobrist;
	zobrist PawnHash;

	vector< zobrist > PastPositions;
	vector< takeback > MoveHistory;

	position();
	position( const position& o );
	void operator=( const position& o );

	bool SetUpPosition( string FEN );

	string toString() const;
	string FEN() const;

	void AddPiece( color c, type t, square s );
	void RemovePiece( color c, type t, square s );
	void RemovePiece( square s );

	color GetPieceColor( square s ) const;
	type GetPieceType( square s, color c ) const;

	bool IsSquareAttackedByColor( square s, color c ) const;
	bitboard AttacksTo( square s, color c );
	bool InCheck( color c );

	void MakeMove( const move Move );
	void TakeBack();
	void MakeNullMove();

	bool MoveIsLegal( const move Move ) const;
	bool LineIsLegal( const line& Line );

	void Reverse( position& Output );

	bool IsDraw( int Repetitions ) const;
	direction GetPinDirection( color ToMove, square Square ) const;

	void DumpHistory();
};

#pragma once