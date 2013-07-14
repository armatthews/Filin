#include "Node.h"
#include "moveSorter.h"
#include "searcher.h"

bool Node::IsPVNode()
{
	int a = Searcher->Root.OriginalAlpha;
	int b = Searcher->Root.Beta;
	int Upper = ( ( DistanceFromRoot % 2 ) == 0 ) ? b : -a;
	int Lower = ( ( DistanceFromRoot % 2 ) == 0 ) ? a : -b;

	if( Upper == Beta && Lower == Alpha )
		return true;
	return false;
}

void Node::Reset()
{
	Alpha = OriginalAlpha;
	LocalPV.clear();
	LegalMovesTried = 0;
}

void Node::setSearcher(searcher* Searcher)
{
	this->Searcher = Searcher;
}

searcher* Node::getSearcher()
{
	return this->Searcher;
}

Node Node::CreateChild()
{
	Node Child;

	Child.Alpha = -Beta;
	Child.Beta = -Alpha;
	Child.OriginalAlpha = -Beta;
	Child.ParentPV = &LocalPV;
	Child.Searcher = Searcher;
	Child.LegalMovesTried = 0;
	Child.DistanceFromRoot = DistanceFromRoot + 1;
	Child.DepthRemaining = DepthRemaining - Ply( 1 );
	Child.AllowNullMove = true;
	Child.MateThreat = false;

	return Child;
}

void Node::UpdateParentPV( move Move )
{
	ParentPV->clear();
	ParentPV->push_back( Move );
	ParentPV->insert( ParentPV->end(), LocalPV.begin(), LocalPV.end() );
}

bool Node::TryNullMove()
{
	if( !AllowNullMove || IsPVNode() )
		return false;

	color ToMove = Searcher->ThinkingPosition.ToMove;
	if( Searcher->ThinkingPosition.AllPieces[ ToMove ] == ( Searcher->ThinkingPosition.Pieces[ ToMove ][ Pawn ] | Searcher->ThinkingPosition.Pieces[ ToMove ][ King ] ) )
		return false;

	if( Searcher->ThinkingPosition.InCheck( Searcher->ThinkingPosition.ToMove ) )
		return false;

	if( DepthRemaining >= Ply(7) )
	{
		if( evaluator::PieceMaterial( &Searcher->ThinkingPosition ) < CarefulNullMoveThreshold )
			return false;
	}

	int R = 3;

	LocalPV.clear();

	Searcher->MakeMove( NullMove );

	Node NullChild = CreateChild();
	NullChild.DepthRemaining = DepthRemaining - Ply( 1 ) - Ply( R );
	NullChild.Beta = NullChild.Alpha + 1;
	NullChild.AllowNullMove = false;
	int NullValue = -NullChild.AlphaBeta();

	Searcher->TakeBack();

	Searcher->Statistics.NullMoveAttempts++;
	if( NullValue >= Beta )
	{
		Searcher->Statistics.NullMoveSuccesses++;
		//Alpha = NullValue;
		return true;
	}

	return false;
}

int Node::CalculateExtensions( move& Move, bool WasInCheck, phase MoveSorterPhase )
{
	int Extensions = Ply( 0 );
	bool NowInCheck = Searcher->ThinkingPosition.InCheck( Searcher->ThinkingPosition.ToMove );
	if( NowInCheck )
	{
		Extensions += Ply( 1 );
		Searcher->Statistics.CheckExtensionsDone++;
	}
	else
	{
		if( MoveSorterPhase > KillerMovesPhase )
		{
			if( DepthRemaining >= Ply( 3 ) && !WasInCheck && Move.Promote() == Empty )
			{
				color ToMove = Searcher->ThinkingPosition.ToMove;
				color Enemy = ( ToMove == White ) ? Black : White;
				bitboard MoverIsPawn = Searcher->ThinkingPosition.Pieces[ Enemy ][ Pawn ] & Mask[ Move.To() ];
				bitboard PawnIsPassed = IsEmpty( PawnPassedMask[ Enemy ][ Move.To() ] & Searcher->ThinkingPosition.Pieces[ ToMove ][ Pawn ] ); // TODO: Double check this.
				if( !MoverIsPawn || !PawnIsPassed )
				{
					Extensions -= Ply( 1 );
					Searcher->Statistics.LateMoveReductionsDone++;
				}
			}
		}
	}

	return Extensions;
}

int Node::AlphaBeta()
{
	assert( Alpha == OriginalAlpha );
	assert( Alpha < Beta );
	assert( IsSane() );

	int val;

	if( DepthRemaining < Ply( 1 ) )
		return Quiescent();

	if( ( ( ++Searcher->Statistics.NodesVisited ) & 0x3FF ) == 0 )
		Searcher->TimeOut();

	if( Searcher->ThinkingPosition.IsDraw( 2 ) )
		return DRAWSCORE;

	BoundType TTProbeType = Searcher->TranspositionTable.Probe( Searcher->ThinkingPosition.Zobrist, DistanceFromRoot, DepthRemaining, Alpha, Beta );
	if( TTProbeType != NONE )
		return Alpha;

	if( TryNullMove() )
		return Beta;

	moveSorter MoveSorter( this->Searcher, DistanceFromRoot );
	move Move;

	bool WasInCheck = Searcher->ThinkingPosition.InCheck( Searcher->ThinkingPosition.ToMove );
	move BestMove = NullMove;

	int LocalBeta = Beta;

	for( Move = MoveSorter.GetNextMove(); Move != NullMove; Move = MoveSorter.GetNextMove() )
	{
		Searcher->MakeMove( Move );
		LocalPV.clear();

		int Extensions = 0;
		Extensions += CalculateExtensions( Move, WasInCheck, MoveSorter.CurrentPhase() );

		Node Child = CreateChild();
		Child.DepthRemaining += Extensions;
		//if( LegalMovesTried )
			//Child.Alpha = Child.OriginalAlpha = -Alpha - 1;
		val = -Child.AlphaBeta();

		if( val > Alpha && Extensions < 0 )
		{
			Searcher->Statistics.ReductionResearches++;

			Extensions = 0;

			Child = CreateChild();
			Child.DepthRemaining += Extensions;

			//if( LegalMovesTried )
			//	Child.Alpha = Child.OriginalAlpha = -Alpha - 1;
			val = -Child.AlphaBeta();
		}

		/*if( LegalMovesTried && val > Alpha && val < Beta )
		{
			Child = CreateChild();
			Child.DepthRemaining += Extensions;
			val = -Child.AlphaBeta();
		}*/

		Searcher->TakeBack();		
		LegalMovesTried++;

		if( val > Alpha )
		{
			BestMove = Move;
			Alpha = val;

			if( val >= Beta )
			{
				Searcher->Statistics.FailHighsByMove[ LegalMovesTried - 1 ]++;
				Searcher->RecordKiller( DistanceFromRoot, BestMove );
				Searcher->TranspositionTable.Store( Searcher->ThinkingPosition.Zobrist, DistanceFromRoot, DepthRemaining, LOWER, Beta, BestMove );
				return Beta;
			}

			UpdateParentPV( BestMove ); // TODO: This should be moved to the LegalMovesTried -> Alpha > OrignalAlpha block.
										// The reason why moving this doesn't work right now is because we lose the LocalPV variable when the loop goes to the next iteration.
										// Hence, only the first move of the PV actually shows up.
		}
	}

	if( LegalMovesTried )
	{
		if( Alpha != OriginalAlpha )
			Searcher->TranspositionTable.Store( Searcher->ThinkingPosition.Zobrist, DistanceFromRoot, DepthRemaining, EXACT, Alpha, BestMove );
		else
			Searcher->TranspositionTable.Store( Searcher->ThinkingPosition.Zobrist, DistanceFromRoot, DepthRemaining, UPPER, Alpha, NullMove );
		return Alpha;
	}
	else
	{
		int Score = DRAWSCORE;
		if( WasInCheck )
			Score = -MATESCORE + DistanceFromRoot;
		return Score;
	}
}

int Node::Quiescent()
{
	assert( Alpha == OriginalAlpha );
	assert( IsSane() );

	++Searcher->Statistics.QuiescentNodesVisited;
	if( ( ( ++Searcher->Statistics.NodesVisited ) & 0x3FF ) == 0 )
		Searcher->TimeOut();

	int Eval = Searcher->Evaluator.Evaluate( &Searcher->ThinkingPosition, Searcher->RootPosition.Castling, Alpha, Beta );

	if( Eval >= Beta )
		return Beta;

	if( Eval > Alpha )
		Alpha = Eval;

	quiescentSorter MoveSorter( Searcher, DistanceFromRoot );

	for( move Move = MoveSorter.GetNextMove(); Move != NullMove; Move = MoveSorter.GetNextMove() )
	{
		if( Move.Promote() != Empty && Move.Promote() != Queen )
			continue;

		Searcher->MakeMove( Move );
		LocalPV.clear();
		int val = -CreateChild().Quiescent();
		Searcher->TakeBack();

		if( val > Alpha )
		{
			if( val >= Beta )
				return Beta;

			UpdateParentPV( Move );
			Alpha = val;
		}
	}

	return Alpha;
}

void RootNode::Initialize()
{	
	move Move;
	moveSorter Sorter( Searcher, 0 );

	MoveList.clear();
	for( Move = Sorter.GetNextMove(); Move != NullMove; Move = Sorter.GetNextMove() )	
		MoveList.push_back( Move );

	CurrentNodeCounts.resize( MoveList.size(), 0 );
	PreviousNodeCounts.resize( MoveList.size(), 0 );

	DistanceFromRoot = 0;
	LegalMovesTried = 0;
	AllowNullMove = false;

	Alpha = OriginalAlpha = -INFINITE;
	Beta = INFINITE;
}

void RootNode::SwapMoves( unsigned int i, unsigned int j )
{
	if( i == j ) return;

	int TempPrev = PreviousNodeCounts[ i ];
	int TempCurr = CurrentNodeCounts[ i ];
	move TempMove = MoveList[ i ];

	PreviousNodeCounts[ i ] = PreviousNodeCounts[ j ];
	CurrentNodeCounts[ i ] = CurrentNodeCounts[ j ];
	MoveList[ i ] = MoveList[ j ];

	PreviousNodeCounts[ j ] = TempPrev;
	CurrentNodeCounts[ j ] = TempCurr;
	MoveList[ j ] = TempMove;
}

void RootNode::SortMoveList( move PVMove )
{
	unsigned int Start = 0;
	for( unsigned int i = 0; i < MoveList.size(); i++ )
	{
		if( MoveList[ i ] == PVMove )
		{
			if( i != 0 )
				SwapMoves( i, 0 );
			Start = 1;
			break;
		}
	}

	for( unsigned int i = Start; i < MoveList.size(); i++ )
		for( unsigned int j = i + 1; j < MoveList.size(); j++ )
			if( PreviousNodeCounts[ i ] < PreviousNodeCounts[ j ] )
				SwapMoves( i, j );
}

void RootNode::UpdateParentPV( move Move )
{
	ParentPV->clear();
	ParentPV->push_back( Move );
	ParentPV->insert( ParentPV->end(), LocalPV.begin(), LocalPV.end() );

	DWORD TimeUsed = ( timeGetTime() - Searcher->SearchStartTime ) / 10;
	cout << DepthRemaining / PLY << "." << "\t" << Alpha << "\t" << TimeUsed << "\t" << Searcher->Statistics.NodesVisited << "\t" << LineToString( *ParentPV, &Searcher->RootPosition ) << "\n";
}

int RootNode::AlphaBeta()
{
	assert( IsSane() );

	int val;
	move Move;

	if( DepthRemaining < Ply( 1 ) )
		return Quiescent();

	bool WasInCheck = Searcher->ThinkingPosition.InCheck( Searcher->ThinkingPosition.ToMove );
	move BestMove = NullMove;

	for( unsigned int i = 0; i < MoveList.size(); i++ )
	{
		Move = MoveList[ i ];
		int SEE = Searcher->SEE( Move );
		Searcher->MakeMove( Move );
		LocalPV.clear();

		Node Child = CreateChild();

		unsigned int NodesBefore = Searcher->Statistics.NodesVisited;

		phase Phase = NonCapturesPhase;
		if( Searcher->ThinkingPosition.MoveHistory.back().Captured != Empty )
			Phase = ( SEE >= 0 ) ? GoodCapturesPhase : BadCapturesPhase;
		if( i == 0 )
			Phase = HashMovePhase;
		int Extensions = CalculateExtensions( Move, WasInCheck, Phase );
		Child.DepthRemaining += Extensions;

		//if( i != 0 )
		//	Child.Alpha = Child.OriginalAlpha = -Alpha - 1;
		val = -Child.AlphaBeta();

		if( val > Alpha && Extensions < 0 )
		{
			Extensions = 0;
			Child = CreateChild();
			Child.DepthRemaining += Extensions;
			//if( i != 0 )
			//	Child.Alpha = Child.OriginalAlpha = -Alpha - 1;
			val = -Child.AlphaBeta();
		}

		/*if( i != 0 && val > Alpha )
		{
			Child = CreateChild();
			Child.DepthRemaining += Extensions;
			val = -Child.AlphaBeta();
		}*/

		Searcher->TakeBack();

		CurrentNodeCounts[ i ] = Searcher->Statistics.NodesVisited - NodesBefore;

		if( val > Alpha )
		{
			Alpha = val;
			BestMove = Move;

			if( val >= Beta )
			{
				SwapMoves( i, 0 );
				Searcher->TranspositionTable.Store( Searcher->ThinkingPosition.Zobrist, DistanceFromRoot, DepthRemaining, LOWER, Beta, BestMove );
				return Beta;
			}

			UpdateParentPV( Move );
		}
	}
	
	if( Alpha != OriginalAlpha )
		Searcher->TranspositionTable.Store( Searcher->ThinkingPosition.Zobrist, DistanceFromRoot, DepthRemaining, EXACT, Alpha, BestMove );
	else
		Searcher->TranspositionTable.Store( Searcher->ThinkingPosition.Zobrist, DistanceFromRoot, DepthRemaining, UPPER, Alpha, NullMove );
	return Alpha;
}

bool Node::IsSane()
{
//#define CheckSanity( x ) assert( x )
#define CheckSanity( x ) if( !( x ) ) return false

	bitboard BothPieces = Searcher->ThinkingPosition.BothPieces;
	bitboard AllPieces[ 2 ];
	bitboard Pieces[ 2 ][ 6 ];

	for( color c = White; c <= Black; c++ )
	{
		AllPieces[ c ] = Searcher->ThinkingPosition.AllPieces[ c ];
		for( type t = Pawn; t <= King; t++ )
			Pieces[ c ][ t ] = Searcher->ThinkingPosition.Pieces[ c ][ t ];
	}
	CheckSanity( BothPieces == ( AllPieces[ White ] | AllPieces[ Black ] ) );
	CheckSanity( IsEmpty( AllPieces[ White ] & AllPieces[ Black ] ) );

	for( color c = White; c <= Black; c++ )
	{
		CheckSanity( Pieces[ c ][ King ] != 0 );

		for( type t = Pawn; t <= King; t++ )
		{
			CheckSanity( ( AllPieces[ c ] & Pieces[ c ][ t ] ) == Pieces[ c ][ t ] );
			CheckSanity( ( BothPieces & Pieces[ c ][ t ] ) == Pieces[ c ][ t ] );

			for( type t2 = t + 1; t2 <= King; t2++ )
				CheckSanity( IsEmpty( Pieces[ c ][ t ] & Pieces[ c ][ t2 ] ) );
		}
	}

	return true;
}
