#include "move.h"
#include "moveSorter.h"
#pragma once

class searcher;
class Node
{
public:
	int DistanceFromRoot;
	int DepthRemaining;
	int Alpha, Beta, OriginalAlpha;
	line LocalPV;
	line* ParentPV;
	searcher* Searcher;
	int LegalMovesTried;
	bool AllowNullMove;
	bool MateThreat;

	void Reset();

	Node CreateChild();
	Node CreateNullChild();

	bool IsPVNode();
	bool TryNullMove();
	void DoIID();
	bool DeltaPrune( int Eval );
	int CalculateExtensions( move& Move, bool WasInCheck, phase MoveSorterPhase );

	virtual void UpdateParentPV( move Move );
	virtual int AlphaBeta();
	virtual int Quiescent();

	bool IsSane();
};

class RootNode : public Node
{
	public:
	vector< UINT64 > PreviousNodeCounts;
	vector< UINT64 > CurrentNodeCounts;
	vector< move > MoveList;

	void Initialize();
	void SwapMoves( unsigned int i, unsigned int j );
	void SortMoveList( move PVMove );
	void UpdateParentPV( move Move );
	int AlphaBeta();
};
