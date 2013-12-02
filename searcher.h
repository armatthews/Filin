#include <setjmp.h>
#include "Utilities.h"
#include "TranspositionTable.h"
#include "Evaluate.h"
#include "Book.h"
#include "moveGenerator.h"
#include "moveSorter.h"
#include "Node.h"

struct plyInfo
{
	move Killers[ NUM_KILLERS ];
	bool InCheck;

	plyInfo();
	plyInfo( const plyInfo& o );
	void operator=( const plyInfo& o );
};

class searcher
{
	friend class Node;
	friend class RootNode;
	friend class moveSorter;
	private:
		book OpeningBook;
		transpositionTable TranspositionTable;

		vector< plyInfo > PlyInfo;

		DWORD SearchStartTime;
		DWORD SoftTimeOutTime, HardTimeOutTime;
		jmp_buf JumpToSearchStart;
		jmp_buf JumpToPausePoint;

	public:
		position RootPosition;
	private:
		RootNode Root;
		line RootPV;

	public:
		evaluator Evaluator;
		position ThinkingPosition;

		struct
		{
			unsigned int NodesVisited;
			unsigned int QuiescentNodesVisited;
			unsigned int FailHighsByMove[ 100 ];
			unsigned int LateMoveReductionsDone;
			unsigned int ReductionResearches;
			unsigned int CheckExtensionsDone;
			unsigned int NullMoveAttempts;
			unsigned int NullMoveSuccesses;
			unsigned int SearchTime;
		} Statistics;

	private:
		bool CheckBook( position* Position, int Depth = 0 );

		void MakeMove( const move Move );
		void TakeBack();

		bool IsDraw();

		int Iterate( int Depth );

		void RecordKiller( int DistanceFromRoot, move Move );
		void RecordBestMove( int DepthRemaining, int OriginalAlpha, int Beta, int Score, move BestMove );

		square GetLowestPiece( bitboard& Attackers, color c ) const;

		bool TimeOut();

		bitboard GetAttackers( square Target ) const;

	public:
		void InitSearch( position* Position, int Depth, int Time );
		int SEE( move Move ) const;
		move GetHashMove() const;
		move GetKiller( int DistanceFromRoot, int Index ) const;
		void Stop();

	private:
		line PrincipleVariation;

	public:
		searcher();
		~searcher();

		void Reset( bool ResetTT );
		int Search( position* Position, int Depth, int Time );
		line GetPrincipleVariation() const;

		void TTDebugProbe( zobrist Zobrist ) const;
};

#pragma once
