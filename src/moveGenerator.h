#include "Utilities.h"
#include "position.h"
#include "moveList.h"

class moveGenerator
{
public:
	virtual move GetNextMove() = 0;
	virtual void Reset() = 0;
	virtual void Reset( bitboard TargetMask ) = 0;
        virtual ~moveGenerator();

protected:
	position* Position;
	color ToMove;
	bitboard PinnedPieces;
	type NextPromotion;
	bitboard TargetMask;

public:
	static bitboard GetPawnForwards( const position* Position, square s, color c );
	static bitboard GetPawnAttacks( square s, color c );
	static bitboard GetKnightMoves( const position* Position, square s, color c );
	static bitboard GetBishopMoves( const position* Position, square s, color c );
	static bitboard GetBishopMoves( square s, bitboard occupancy );
	static bitboard GetRookMoves( const position* Position, square s, color c );
	static bitboard GetRookMoves( square s, bitboard occupancy );
	static bitboard GetQueenMoves( const position* Position, square s, color c );
	static bitboard GetQueenMoves( square s, bitboard Occupancy );
	static bitboard GetKingMoves( const position* Position, square s, color c );

	static bitboard GetBishopXRay( square s, bitboard occupancy );
	static bitboard GetRookXRay( square s, bitboard occupancy );

	static bitboard GetPinnedPieces( const position* Position, color c );
	static bitboard GetLegalMoves( const position* Position, square Square, type t, color ToMove, bool IsPinned );

protected:
	static bitboard GetLegalPawnForwards( const position* Position, square s, color c );
	static bitboard GetLegalPawnAttacks( const position* Position, square s, color c );
	static bitboard GetLegalKnightMoves( const position* Position, square s, color c );
	static bitboard GetLegalBishopMoves( const position* Position, square s, color c );
	static bitboard GetLegalRookMoves( const position* Position, square s, color c );
	static bitboard GetLegalQueenMoves( const position* Position, square s, color c );
	static bitboard GetLegalKingMoves( const position* Position, square s, color c );

	static bitboard GetAllPawnAttacks( const position* Position, color c );
	static bitboard GetCastlingMoves( const position* Position, square s, color c );

	static void AddMovesToList( const position* Position, square From, bitboard Moves, moveList* MoveList );

	static void GenerateMoves( const position* Position, moveList* MoveList );

	type GetNextPromotion();
};

class LegalMoveGenerator : public moveGenerator
{
protected:
	bitboard Movers;
	square Square;
	bitboard Moves;
	int NextType;

public:
	move GetNextMove();
	void Reset();
	void Reset( bitboard TargetMask );
	LegalMoveGenerator( position* Position );
protected:
	LegalMoveGenerator();
	virtual void SetSquareAndMoves();
};

class EvasionGenerator : public moveGenerator
{
protected:
	color Enemy;

	square KingSquare;
	bitboard Checkers;
	bitboard KingMoves;

	square Checker;
	bitboard Attackers;

	bitboard IntermediateSquares;
	bitboard Interceptors;

	enum phase { KingMovesPhase, CapturesPhase, EPPhase, InterceptionPhase };
	phase Phase;
public:
	move GetNextMove();
	void Reset();
	void Reset( bitboard TargetMask );
	EvasionGenerator( position* Position );
protected:
	EvasionGenerator();
	void Setup();
	void SetupAttackersPhase();
	void SetupInterceptionPhase();
	bitboard FindInterceptors( square To );
};

class Generator
{
public:
	Generator( position* Position );
	virtual ~Generator();
	move GetNextMove();
	void Reset();
	void Reset( bitboard TargetMask );
protected:
	Generator();
	moveGenerator* MoveGenerator;
};

#pragma once
