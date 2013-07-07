#include "moveSorter.h"
#include "searcher.h"

moveSorter::moveSorter()
{
	MoveGenerator = NULL;
}

moveSorter::moveSorter( searcher* Searcher, int DistanceFromRoot )
{
	this->Searcher = Searcher;
	this->DistanceFromRoot = DistanceFromRoot;
	MoveGenerator = NULL;
	InCheck = Searcher->ThinkingPosition.InCheck( Searcher->ThinkingPosition.ToMove );

	Phase = HashMovePhase;
	MovesSorted = 0;

	HashMove = NullMove;
	GoodCaptures.Clear();
	BadCaptures.Clear();

	for( int i = 0; i < NUM_KILLERS; i++ )
		Killers[ i ] = NullMove;

	GoodCaptureScores.clear();
}

moveSorter::~moveSorter()
{
	if( MoveGenerator )
		delete MoveGenerator;
	MoveGenerator = NULL;
}

phase moveSorter::CurrentPhase()
{
	return Phase;
}

void moveSorter::AdvancePhase()
{
	Phase++;
	MovesSorted = 0;
}

move moveSorter::GetNextMove()
{
	move Move;

	if( Phase == HashMovePhase )
	{	
		if( MovesSorted++ == 0 )
		{
			HashMove = Searcher->GetHashMove();

			if( HashMove != NullMove )
				return HashMove;
		}

		AdvancePhase();
	}

	if( Phase == GenerateCapturesPhase )
	{
		PrepareCaptures();
		AdvancePhase();
	}

	if( Phase == GoodCapturesPhase )
	{
		if( MovesSorted >= GoodCaptures.size() )
			AdvancePhase();
		else
			return GoodCaptures[ MovesSorted++ ];
	}

	if( Phase == KillerMovesPhase )
	{
		while( MovesSorted < NUM_KILLERS && !InCheck )
		{
			Move = Searcher->GetKiller( DistanceFromRoot, MovesSorted );
			this->Killers[ MovesSorted ] = Move;
			MovesSorted++;
			if( Move != NullMove && Searcher->ThinkingPosition.MoveIsLegal( Move ) )
			{
#ifdef _DEBUG
				Generator DebugGenerator( &Searcher->ThinkingPosition );
				bool Found = false;
				move DebugMove;

				while( ( DebugMove = DebugGenerator.GetNextMove() ) != NullMove )
				{
					if( DebugMove == Move )
					{
						Found = true;
						break;
					}
				}

				if( !Found )
				{
					cout << "Recommending nonsense killer " << Move.toString() << " in the following position: \n" << Searcher->ThinkingPosition.toString() << "\n" << Searcher->ThinkingPosition.FEN() << "\n";
					cout << "DFR: " << DistanceFromRoot << "\n";
					system( "PAUSE" );
				}
#endif
				return Move;
			}
		}

		AdvancePhase();
	}

	if( Phase == BadCapturesPhase )
	{
		if( MovesSorted >= BadCaptures.size() )
			AdvancePhase();
		else
			return BadCaptures[ MovesSorted++ ];
	}

	if( Phase == GenerateNonCapturesPhase )
	{
		MoveGenerator->Reset( ~Searcher->ThinkingPosition.AllPieces[ Searcher->ThinkingPosition.Enemy ] );
		AdvancePhase();
	}

	if( Phase == NonCapturesPhase )
	{
		while( ( Move = MoveGenerator->GetNextMove() ) != NullMove )
		{
			bool MoveIsNew = true;
			
			if( Move == HashMove )
				MoveIsNew = false;
			else
				for( int i = 0; i < NUM_KILLERS; i++ )
					if( Move == Killers[ i ] )
						MoveIsNew = false;

			if( MoveIsNew )
			{
				MovesSorted++;
				return Move;
			}
		}

		AdvancePhase();
	}

	if( Phase == DonePhase )
		return NullMove;

	assert( "Invalid Phase" && 0 );
	return NullMove;
}

void moveSorter::PrepareCaptures()
{
	move Move;

	MoveGenerator = new Generator( &Searcher->ThinkingPosition );
	MoveGenerator->Reset( Searcher->ThinkingPosition.AllPieces[ Searcher->ThinkingPosition.Enemy ] );

	while( ( Move = MoveGenerator->GetNextMove() ) != NullMove )
	{
		if( Move == HashMove )
			continue;

		color Me = Searcher->ThinkingPosition.ToMove;
		color Enemy = ( Me == White ) ? Black : White;
		type Captured = Searcher->ThinkingPosition.GetPieceType( Move.To(), Enemy );
		type Mover = Searcher->ThinkingPosition.GetPieceType( Move.From(), Me );

		int Score = 128 * PieceValues[ Captured ] - PieceValues[ Mover ];
		if( PieceValues[ Mover ] < PieceValues[ Captured ] )
		{
			GoodCaptures.Add( Move );
			GoodCaptureScores.push_back( Score );
		}
		else
		{
			if( Searcher->SEE( Move ) >= 0 )
			{
				GoodCaptures.Add( Move );
				GoodCaptureScores.push_back( Score );
			}
			else
				BadCaptures.Add( Move );
		}
	}

	Sort( &GoodCaptures, &GoodCaptureScores );
}

void moveSorter::Sort( moveList* MoveList, scoreList* ScoreList )
{
	for( unsigned int i = 0; i < MoveList->size(); i++ )
		for( unsigned int j = i + 1; j < MoveList->size(); j++ )
			if( ScoreList->at( i ) < ScoreList->at( j ) )
			{
				int TempScore = ScoreList->at( i );
				ScoreList->at( i ) = ScoreList->at( j );
				ScoreList->at( j ) = TempScore;

				move TempMove = MoveList->at( i );
				MoveList->at( i ) = MoveList->at( j );
				MoveList->at( j ) = TempMove;
			}
}

quiescentSorter::quiescentSorter( searcher* Searcher, int DistanceFromRoot )
{
	this->Searcher = Searcher;
	this->DistanceFromRoot = DistanceFromRoot;
	MoveGenerator = NULL;
	InCheck = Searcher->ThinkingPosition.InCheck( Searcher->ThinkingPosition.ToMove );

	Phase = HashMovePhase;
	MovesSorted = 0;

	HashMove = NullMove;
	GoodCaptures.Clear();
	BadCaptures.Clear();

	for( int i = 0; i < NUM_KILLERS; i++ )
		Killers[ i ] = NullMove;

	GoodCaptureScores.clear();
}

void quiescentSorter::PrepareCaptures()
{
	move Move;

	MoveGenerator = new Generator( &Searcher->ThinkingPosition );
	MoveGenerator->Reset( Searcher->ThinkingPosition.AllPieces[ Searcher->ThinkingPosition.Enemy ] );

	while( ( Move = MoveGenerator->GetNextMove() ) != NullMove )
	{
		if( Move == HashMove )
			continue;

		color Me = Searcher->ThinkingPosition.ToMove;
		color Enemy = ( Me == White ) ? Black : White;
		type Captured = Searcher->ThinkingPosition.GetPieceType( Move.To(), Enemy );
		type Mover = Searcher->ThinkingPosition.GetPieceType( Move.From(), Me );

		int Score = 128 * PieceValues[ Captured ] - PieceValues[ Mover ];
		if( PieceValues[ Mover ] < PieceValues[ Captured ] )
		{
			GoodCaptures.Add( Move );
			GoodCaptureScores.push_back( Score );
		}
		else
		{
			if( Searcher->SEE( Move ) >= 0 )
			{
				GoodCaptures.Add( Move );
				GoodCaptureScores.push_back( Score );
			}
		}
	}

	Sort( &GoodCaptures, &GoodCaptureScores );
}

move quiescentSorter::GetNextMove()
{
	move Move;

	if( Phase == HashMovePhase )
	{
		if( MovesSorted++ == 0 )
		{
			HashMove = Searcher->GetHashMove();

			if( IsEmpty( Searcher->ThinkingPosition.BothPieces & Mask[ HashMove.To() ] ) )
				HashMove = NullMove;

			if( HashMove != NullMove )
				return HashMove;
		}

		AdvancePhase();
	}

	if( Phase == GenerateCapturesPhase )
	{
		PrepareCaptures();
		AdvancePhase();
	}

	if( Phase == GoodCapturesPhase )
	{
		if( MovesSorted >= GoodCaptures.size() )
			AdvancePhase();
		else
			return GoodCaptures[ MovesSorted++ ];
	}

	if( Phase == KillerMovesPhase )
		AdvancePhase();

	if( Phase == BadCapturesPhase )
		AdvancePhase();

	if( Phase == GenerateNonCapturesPhase )
		AdvancePhase();

	if( Phase == NonCapturesPhase )
		AdvancePhase();

	if( Phase == DonePhase )
		return NullMove;

	assert( "Invalid Phase" && 0 );
	return NullMove;
}