#include <vector>
#include "moveGenerator.h"
#include "moveList.h"
#include "move.h"
using std::vector;

typedef vector< int > scoreList;
typedef int phase;
const phase HashMovePhase = 0;
const phase GenerateCapturesPhase = 1;
const phase GoodCapturesPhase = 2;
const phase KillerMovesPhase = 3;
const phase BadCapturesPhase = 4;
const phase GenerateNonCapturesPhase = 5;
const phase NonCapturesPhase = 6;
const phase DonePhase = 7;

class searcher;
class moveSorter
{
protected:
	searcher* Searcher;
	int DistanceFromRoot;
	Generator* MoveGenerator;

	phase Phase;
	bool InCheck;

	move HashMove;
	move Killers[ NUM_KILLERS ];

	moveList GoodCaptures;
	moveList BadCaptures;

	scoreList GoodCaptureScores;

	unsigned MovesSorted;

	moveSorter();

public:
	moveSorter( searcher* Searcher, int DistanceFromRoot );
	~moveSorter();

	virtual move GetNextMove();
	phase GetCurrentPhase();
	int GetMovesSorted();
protected:
	void PrepareCaptures();
	static void Sort( moveList* MoveList, scoreList* ScoreList );
	move GetNextBest();
	void AdvancePhase();
};

class quiescentSorter : public moveSorter
{
public:
	quiescentSorter( searcher* Searcher, int DistanceFromRoot );
	move GetNextMove();
protected:
	void PrepareCaptures();
};

#pragma once
