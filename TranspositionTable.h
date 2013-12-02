#include "Utilities.h"
#include "move.h"

enum BoundType { NONE, LOWER, UPPER, EXACT, AVOIDNULLMOVE, MOVEONLY };

struct element
{
	zobrist Key;

	BoundType Type : 3;
	int Generation : 3;
	int Value : 32;
	unsigned short Depth : 16;
	move BestMove; // 32

	element();
	void Reset();
};

class transpositionTable
{
private:
	element* Data;

	unsigned int NumberOfEntries;
	unsigned int IndexMask;
	unsigned int Generation;

public:
	transpositionTable();
	transpositionTable( unsigned int Size );
	~transpositionTable();

	transpositionTable( const transpositionTable& o );
	bool operator=( const transpositionTable& o );

	BoundType Probe( zobrist Zobrist, int DistanceFromRoot, int DepthRemaining, int& Alpha, int Beta ) const;
	move BestMove( zobrist Zobrist ) const;
	int StartDepth( zobrist Zobrist, int& StartValue );
	void Store( zobrist Zobrist, int DistanceFromRoot, int DepthRemaining, BoundType Type, int Value, move Move );
	void StorePV( position* Position, line PV, int DepthRemaining, int Value );

	void Resize( unsigned int Size );
	void Clear();
	void NextGeneration();
	void DebugProbe( zobrist Zobrist ) const;
};

#pragma once
