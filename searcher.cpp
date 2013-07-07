#include "searcher.h"
#include "Evaluate.h"
#include "moveGenerator.h"
#include "Pondering.h"
#include "Node.h"

int Ply( int Depth ) { return Depth * PLY; }

plyInfo::plyInfo() {}
plyInfo::plyInfo( const plyInfo& o ) { InCheck = o.InCheck; memcpy( &Killers, &o.Killers, NUM_KILLERS * sizeof( move ) ); }
void plyInfo::operator=( const plyInfo& o ) { InCheck = o.InCheck; memcpy( &Killers, &o.Killers, NUM_KILLERS * sizeof( move ) ); }

searcher::searcher()
{
	TranspositionTable.Resize( 256 );

	//OpeningBook.OpenBook( "NewBook.txt" );

	Reset( true );
}

searcher::~searcher()
{
}

void searcher::MakeMove( const move Move )
{
	ThinkingPosition.MakeMove( Move );
}

void searcher::TakeBack()
{
	ThinkingPosition.TakeBack();
}

void searcher::Reset( bool ResetTT )
{
	if( ResetTT )
		TranspositionTable.Clear();

	PlyInfo.resize( 1 );

	ZeroMemory( &Statistics, sizeof( Statistics ) );

	SoftTimeOutTime = HardTimeOutTime = 0;
}

void searcher::Stop()
{
	SoftTimeOutTime = HardTimeOutTime = 0;
}

move searcher::GetHashMove()
{
	return TranspositionTable.BestMove( ThinkingPosition.Zobrist );
}

void searcher::RecordKiller( int DistanceFromRoot, move Move )
{
	if( Move.Promote() != Empty )
		return;

	if( ThinkingPosition.BothPieces & Mask[ Move.To() ] )
		return;

	if( TranspositionTable.BestMove( ThinkingPosition.Zobrist ) == Move )
		return;

	if( PlyInfo.size() <= DistanceFromRoot )
		PlyInfo.resize( DistanceFromRoot + 1 );

	for( int i = 0; i < NUM_KILLERS - 1; i++ )
		if( PlyInfo[ DistanceFromRoot ].Killers[ i ] == Move )
			return;

	memcpy( &PlyInfo[ DistanceFromRoot ].Killers[ 1 ], &PlyInfo[ DistanceFromRoot ].Killers[ 0 ], sizeof( Move ) * ( NUM_KILLERS - 1 ) );
	PlyInfo[ DistanceFromRoot ].Killers[ 0 ] = Move;
}

move searcher::GetKiller( int DistanceFromRoot, int Index )
{
	if( DistanceFromRoot < PlyInfo.size() && Index < NUM_KILLERS )
		return PlyInfo[ DistanceFromRoot ].Killers[ Index ];
	else
		return NullMove;
}

square searcher::GetLowestPiece( bitboard& Attackers, color c )
{
	if( ( Attackers & ThinkingPosition.AllPieces[ c ] ) == 0 )
		return NullSquare;

	bitboard LowestPiece = 0;

	for( type t = Pawn; t <= King; t++ )
	{
		LowestPiece = Attackers & ThinkingPosition.Pieces[ c ][ t ];
		if( LowestPiece != 0 )
			break;
	}

	return LSBi( LowestPiece );
}

bitboard searcher::GetAttackers( square Target )
{
	bitboard r = 0;

	bitboard PawnMoves[ 2 ];
	for( color c = White; c <= Black; c++ )
		PawnMoves[ c ]= moveGenerator::GetPawnAttacks( Target, FlipColor( c ) );

	bitboard KnightMoves = moveGenerator::GetKnightMoves( &ThinkingPosition, Target, Empty );
	bitboard BishopMoves = moveGenerator::GetBishopMoves( &ThinkingPosition, Target, Empty );
	bitboard RookMoves = moveGenerator::GetRookMoves( &ThinkingPosition, Target, Empty );
	bitboard KingMoves = moveGenerator::GetKingMoves( &ThinkingPosition, Target, Empty );

	bitboard (&Pieces)[ 2 ][ 6 ] = ThinkingPosition.Pieces;

	for( color c = White; c <= Black; c++ )
	{
		r |= BishopMoves & ( Pieces[ c ][ Queen ] | Pieces[ c ][ Bishop ] );
		r |= RookMoves & ( Pieces[ c ][ Queen ] | Pieces[ c ][ Rook ] );
		r |= KnightMoves & Pieces[ c ][ Knight ];
		r |= KingMoves & Pieces[ c ][ King ];
		r |= PawnMoves[ c ] & Pieces[ c ][ Pawn ];
	}

	return r;
}

int searcher::SEE( move Move )
{
	int ValueList[ 32 ];
	int NumberOfCaptures = 1;
	square TargetSquare = Move.To();

	bitboard Attackers = GetAttackers( TargetSquare );
	bitboard Occupied = ThinkingPosition.BothPieces;
	color NextToMove = ThinkingPosition.Enemy;

	bitboard BishopsQueens = ( ThinkingPosition.Pieces[ White ][ Bishop ] | ThinkingPosition.Pieces[ White ][ Queen ] | ThinkingPosition.Pieces[ Black ][ Bishop ] | ThinkingPosition.Pieces[ Black ][ Queen ] );
	bitboard RooksQueens = ( ThinkingPosition.Pieces[ White ][ Rook ] | ThinkingPosition.Pieces[ White ][ Queen ] | ThinkingPosition.Pieces[ Black ][ Rook ] | ThinkingPosition.Pieces[ Black ][ Queen ] );

	type NextVictim = ThinkingPosition.GetPieceType( TargetSquare, ThinkingPosition.Enemy );
	ValueList[ 0 ] = PieceValues[ NextVictim ];
	NextVictim = ThinkingPosition.GetPieceType( Move.From(), ThinkingPosition.ToMove );
	Occupied ^= Mask[ Move.From() ];

	if( NextVictim != Knight && NextVictim != King )
	{
		if( NextVictim == Pawn || NextVictim == Bishop || NextVictim == Queen )
			Attackers |= moveGenerator::GetBishopMoves( TargetSquare, Occupied ) & BishopsQueens;
		if( NextVictim == Rook || NextVictim == Queen )
			Attackers |= moveGenerator::GetRookMoves( TargetSquare, Occupied ) & RooksQueens;
	}

	Attackers &= Occupied;

	while( Attackers )
	{
		bitboard Temp;
		type Piece;
		for( Piece = Pawn; Piece <= King; Piece++ )
		{
			Temp = ThinkingPosition.Pieces[ NextToMove ][ Piece ];
			if( Temp = ( ThinkingPosition.Pieces[ NextToMove ][ Piece ] & Attackers ) )
				break;
		}

		if( Piece > King )
			break;

		Occupied ^= ( Temp & -Temp );
		if( Piece != Knight && Piece != King )
		{
			if( Piece == Pawn || Piece == Bishop || Piece == Queen )
				Attackers |= moveGenerator::GetBishopMoves( TargetSquare, Occupied ) & BishopsQueens;
			if( Piece == Rook || Piece == Queen )
				Attackers |= moveGenerator::GetRookMoves( TargetSquare, Occupied ) & RooksQueens;
		}

		Attackers &= Occupied;

		ValueList[ NumberOfCaptures ] = -ValueList[ NumberOfCaptures - 1 ] + PieceValues[ NextVictim ];
		NextVictim = Piece;
		NumberOfCaptures++;
		NextToMove = FlipColor( NextToMove );
	}

	while( --NumberOfCaptures )
		ValueList[ NumberOfCaptures - 1 ] = -max( -ValueList[ NumberOfCaptures - 1 ], ValueList[ NumberOfCaptures ] );

	return ValueList[ 0 ];
}

void searcher::InitSearch( position* Position, int Depth, int Time )
{
	RootPosition = *Position;
	ThinkingPosition = RootPosition;

	ZeroMemory( &Statistics, sizeof( Statistics ) );
	RootPV.clear();

	SearchStartTime = timeGetTime();
	if( Time != 0 )
	{
		SoftTimeOutTime = SearchStartTime + Time;
		HardTimeOutTime = SearchStartTime + 2 * Time;
	}
	else
	{
		SoftTimeOutTime = HardTimeOutTime = 0xFFFFFFFF;
	}

	TranspositionTable.NextGeneration();
}

int searcher::Iterate( int Depth )
{
	int Score;
	int PreviousScore;
	DWORD TimeUsed;

	int FailLows = 0;
	int FailHighs = 0;

	Root.Searcher = this;
	Root.ParentPV = &RootPV;
	Root.Initialize();

	if( ThinkingPosition.IsDraw( 3 ) )
		return DRAWSCORE;

	if( Root.MoveList.size() == 0 )
		return ThinkingPosition.InCheck( ThinkingPosition.ToMove ) ? -MATESCORE : DRAWSCORE;

	int StartDepth = TranspositionTable.StartDepth( ThinkingPosition.Zobrist, Score );
	if( StartDepth == 0 )
		Score = Root.Quiescent();
	else
	{
		assert( RootPV.empty() );
		RootPV.push_back( TranspositionTable.BestMove( ThinkingPosition.Zobrist ) );
		TimeUsed = ( timeGetTime() - SearchStartTime ) / 10;
		cout << StartDepth - 1 << "." << "\t" << Score << "\t" << TimeUsed << "\t" << Statistics.NodesVisited << "\t" << LineToString( RootPV, &this->RootPosition ) << "\n";
	}

	for( int i = StartDepth; i <= Depth; i++ )
	{
		PreviousScore = Score;
		int Alphas[ 10 ] = { -INFINITE, PreviousScore - 40, PreviousScore - 140, PreviousScore - 340, -INFINITE };
		int Betas [ 10 ] = { INFINITE, PreviousScore + 40, PreviousScore + 140, PreviousScore + 340, +INFINITE };
		FailLows = FailHighs = 0;

		do
		{
			Root.Alpha = Root.OriginalAlpha = Alphas[ FailLows ];
			Root.Beta = Betas[ FailHighs ];
			Root.DepthRemaining = Ply( i );
			Score = Root.AlphaBeta();

			TimeUsed = ( timeGetTime() - SearchStartTime ) / 10;
			cout << i << "." << "\t" << Score << "\t" << TimeUsed << "\t" << Statistics.NodesVisited << "\t" << LineToString( RootPV, &this->RootPosition ) << "\n";

			if( Score <= Root.OriginalAlpha )
				FailLows++;

			if( Score >= Root.Beta )
				FailHighs++;
		} while( Score <= Root.OriginalAlpha || Score >= Root.Beta );

		Root.PreviousNodeCounts = Root.CurrentNodeCounts;
		Root.CurrentNodeCounts = vector< UINT64 >( Root.CurrentNodeCounts.size(), 0 );
		move PVMove = ( RootPV.size() == 0 ) ? NullMove : RootPV[ 0 ];
		Root.SortMoveList( PVMove );
		TranspositionTable.StorePV( &ThinkingPosition, RootPV, Ply( i ), Score );

		/*int TTScore = -INFINITE;
		BoundType TTProbeType = TranspositionTable.Probe( ThinkingPosition.Zobrist, 0, 0, TTScore, INFINITE );
		assert( TTProbeType == EXACT );
		assert( ThinkingPosition.Zobrist == RootPosition.Zobrist );
		assert( TTScore == Score );
		assert( PVMove == TranspositionTable.BestMove( RootPosition.Zobrist ) );*/

	}

	return Score; 
}

line searcher::GetPrincipleVariation()
{
	return RootPV;
}

int searcher::Search( position* Position, int Depth, int Time )
{
	int Score = 0;
	bool Started = false;

	if( Depth == 0 ) Depth = 0x7FFFFFFF;

	InitSearch( Position, Depth, Time );

	if( CheckBook( Position ) )
		return 0;

	if( setjmp( JumpToSearchStart ) == 0 )
	{
		Started = true;
		Score = Iterate( Depth );
	}

	Statistics.SearchTime = timeGetTime() - SearchStartTime;
	while( ThinkingPosition.Zobrist != RootPosition.Zobrist )
		ThinkingPosition.TakeBack();

	return Score;
}

bool searcher::TimeOut()
{
	DWORD Time = timeGetTime();
	if( Time >= SoftTimeOutTime )
		longjmp( JumpToSearchStart, 1 );

	return false;
}

bool searcher::CheckBook( position* Position, int Depth )
{
	move BookMove = OpeningBook.GetMove( Position->Zobrist );
	if( BookMove == NullMove )
		return false;

	assert( RootPV.size() == 0 );
	RootPV.push_back( BookMove );

	return true;
}

void searcher::TTDebugProbe( zobrist Zobrist )
{
	TranspositionTable.DebugProbe( Zobrist );
}
