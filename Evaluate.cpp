#include <cmath>
#include "Evaluate.h"
#include "moveGenerator.h"
using std::abs;

pawnScore::pawnScore()
{
	Hash = 0;
	MiddleGameScore = 0;
	EndGameScore = 0;

	for( color c = White; c <= Black; c++ )
	{
		KingSideDefects[ c ] = 0;
		QueenSideDefects[ c ] = 0;
		DFileDefects[ c ] = 0;
		EFileDefects[ c ] = 0;

		All[ c ] = 0;
		Passed[ c ] = 0;
		Hidden[ c ] = 0;
		Candidates[ c ] = 0;
	}

	OpenFiles = 255;
}

evaluation::evaluation()
{
	ZeroMemory( this, sizeof( evaluation ) );
}

evaluator::evaluator()
{
	Verbose = false;
	ZeroMemory( &LastPawnScore, sizeof( LastPawnScore ) );
}

void evaluator::EvaluatePassedPawns( color c )
{
	int MiddleGameScore = 0;
	int EndGameScore = 0;

	byte Pawns = Evaluation.PawnScore.Passed[ c ];
	square UpOne = ( c == White ) ? 8 : -8;
	color Enemy = ( c == White ) ? Black : White;

	while( Pawns )
	{
		int file = LSBi( Pawns );
		Pawns ^= ( 1 << file );

		square Square = MostAdvanced( c, Position->Pieces[ c ][ Pawn ] & FileMask[ file ] );
		int rank = Rank( Square );

		int RankBonus = PassedPawnRankBonus[ MyRank[ c ][ rank ] ];
		int DistanceBonus = PassedPawnDistanceBonus[ MyRank[ c ][ rank ] ];

		MiddleGameScore += PassedPawnValue[ MiddleGame ][ c ][ rank ];
		EndGameScore += PassedPawnValue[ EndGame ][ c ][ rank ];

		if( Position->Pieces[ c ][ Pawn ] & PawnConnectedMask[ Square ] )
		{
			MiddleGameScore += ConnectedPassedPawnValue[ MiddleGame ] * RankBonus;
			EndGameScore += ConnectedPassedPawnValue[ EndGame ] * RankBonus;
		}

		if( Position->Pieces[ White ][ Rook ] | Position->Pieces[ Black ][ Rook ] )
		{
			int Minus8 = ( c == White ) ? -8 : 8;
			bitboard Behind = Position->BothPieces & Ray( Square, Minus8 );
			if( Behind )
			{
				int BehindSquare = MostAdvanced( c, Behind );
				if( Mask[ BehindSquare ] & Position->Pieces[ c ][ Rook ] )
				{
					MiddleGameScore += RookBehindPassedPawn[ MiddleGame ];
					EndGameScore += RookBehindPassedPawn[ MiddleGame ];
				}
				else if( Mask[ BehindSquare ] & Position->Pieces[ Enemy ][ Rook ] )
				{
					MiddleGameScore -= RookBehindPassedPawn[ MiddleGame ];
					EndGameScore -= RookBehindPassedPawn[ MiddleGame ];
				}
			}
		}

		square BlockingSquare = Square + UpOne;
		if( Position->AllPieces[ Enemy ] & Mask[ BlockingSquare ] )
		{
			MiddleGameScore -= BlockadingPassedPawnValue[ MiddleGame ][ c ][ rank ];
			EndGameScore -= BlockadingPassedPawnValue[ EndGame ][ c ][ rank ];
		}
		else if( Position->AllPieces[ c ] & Mask[ BlockingSquare ] )
		{
			MiddleGameScore -= BlockadingPassedPawnValue[ MiddleGame ][ c ][ rank ] / 2;
			EndGameScore -= BlockadingPassedPawnValue[ EndGame ][ c ][ rank ] / 2;
		}

		EndGameScore -= Distance( BlockingSquare, Evaluation.KingSquare[ c ] ) * DistanceBonus - Distance( BlockingSquare, Evaluation.KingSquare[ Enemy ] ) * DistanceBonus;
	}

	if( Evaluation.PawnScore.Passed[ c ] )
	{
		if( IsOutside[ Evaluation.PawnScore.Passed[ c ] ][ Evaluation.PawnScore.All[ Enemy ] ] )
		{
			MiddleGameScore += OutsidePasser[ MiddleGame ];
			EndGameScore += OutsidePasser[ EndGame ];
		}
	}

	Evaluation.Scores[ MiddleGame ] += Sign[ c ] * MiddleGameScore;
	Evaluation.Scores[ EndGame ] += Sign[ c ] * EndGameScore;
}

void evaluator::EvaluatePassedPawnRaces( color c )
{
	// TODO: Implement 1126-1314
}

int evaluator::EvaluateKingsFile( color c, int WhichFile )
{
	int Defects = 0;
	color Enemy = ( c == White ) ? Black : White;
	bitboard MyPawns = Position->Pieces[ c ][ Pawn ];
	bitboard HisPawns = Position->Pieces[ Enemy ][ Pawn ];
	bitboard AllPawns = MyPawns | HisPawns;

	for( int file = WhichFile - 1; file <= WhichFile + 1; file++ )
	{
		if( IsEmpty( FileMask[ file ] & AllPawns ) )
			Defects += OpenFileDefects[ file ];
		else
		{
			if( IsEmpty( FileMask[ file ] & HisPawns ) )
				Defects += HalfOpenFileDefects[ file ] / 2;
			else
				Defects += PawnDefects[ c ][ Rank( MostAdvanced( Enemy, FileMask[ file ] & HisPawns ) ) ];

			if( IsEmpty( FileMask[ file ] & MyPawns ) )
				Defects += HalfOpenFileDefects[ file ];
			else
			{
				if( IsEmpty( MyPawns & Mask[ MakeSquare( file, MyRank[ c ][ RANK2 ] ) ] ) )
				{
					Defects++;
					if( IsEmpty( MyPawns & Mask[ MakeSquare( file, MyRank[ c ][ RANK3 ] ) ] ) )
						Defects++;
				}
			}
		}
	}

	return Defects;
}

void evaluator::EvaluatePawns( color c )
{
	int MiddleGameScore = 0;
	int EndGameScore = 0;
	color Enemy = ( c == White ) ? Black : White;
	bitboard AllPawns = Position->Pieces[ White ][ Pawn ] | Position->Pieces[ Black ][ Pawn ];
	int UpOne = Sign[ c ] * 8;

	Evaluation.PawnScore.All[ c ] = 0;
	Evaluation.PawnScore.Candidates[ c ] = 0;
	Evaluation.PawnScore.Passed[ c ] = 0;

	bitboard PawnMoves = 0;
	bitboard Pawns = Position->Pieces[ c ][ Pawn ];
	while( Pawns )
	{
		square Square = LSBi( Pawns );
		Evaluation.PawnScore.All[ c ] |= ( 1 << File( Square ) );

		for( square s = Square; s != MakeSquare( File( Square ), MyRank[ c ][ RANK7 ] ); s += UpOne )
		{
			PawnMoves |= Mask[ s ];
			if( Mask[ s + UpOne ] & AllPawns )
				break;

			int Defenders = Population( moveGenerator::GetPawnAttacks( s + UpOne, Enemy ) & Position->Pieces[ c ][ Pawn ] );
			int Attackers = Population( moveGenerator::GetPawnAttacks( s + UpOne, c ) & Position->Pieces[ Enemy ][ Pawn ] );

			if( Attackers > Defenders )
				break;
		}

		Pawns ^= Mask[ Square ];
	}

	Pawns = Position->Pieces[ c ][ Pawn ];
	while( Pawns )
	{
		square Square = LSBi( Pawns );
		int file = File( Square );
		int rank = Rank( Square );
		Pawns ^= Mask[ Square ];

		Evaluation.PawnScore.OpenFiles &= ~( 1 << file );

		MiddleGameScore += PieceSquare[ Pawn ][ MiddleGame ][ MySquare[ c ][ Square ] ];
		EndGameScore += PieceSquare[ Pawn ][ EndGame ][ MySquare[ c ][ Square ] ];

		if( IsEmpty( PawnIsolatedMask[ Square ] & Position->Pieces[ c ][ Pawn ] ) )
		{
			MiddleGameScore -= PawnIsolated[ MiddleGame ];
			EndGameScore -= PawnIsolated[ EndGame ];

			if( IsEmpty( Position->Pieces[ Enemy ][ Pawn ] & FileMask[ file ] ) )
			{
				MiddleGameScore -= PawnIsolated[ MiddleGame ] / 2;
				EndGameScore -= PawnIsolated[ EndGame ] / 2;
			}
		}
		else
		{
			int Attackers = 0;
			int Defenders = 0;
			bitboard ForwardDirection = ( c == White ) ? Ray( Square, +8 ) : Ray( Square, -8 );
			ForwardDirection &= ~Mask[ Square ];
			bitboard Temp = PawnMoves & ForwardDirection;
			while( Temp )
			{
				square s = LSBi( Temp );
				Temp ^= Mask[ s ];

				Defenders = Population( moveGenerator::GetPawnAttacks( s, Enemy ) & Position->Pieces[ c ][ Pawn ] );
				Attackers = Population( moveGenerator::GetPawnAttacks( s, c ) & Position->Pieces[ Enemy ][ Pawn ] );
				if( Defenders && Defenders >= Attackers )
					break;
			}

			if( !( Defenders && Defenders >= Attackers ) )
			{
				if( IsEmpty( moveGenerator::GetPawnAttacks( Square, Enemy ) & PawnMoves ) )
				{
					MiddleGameScore -= WeakPawn[ MiddleGame ];
					EndGameScore -= WeakPawn[ EndGame ];

					if( IsEmpty( Position->Pieces[ Enemy ][ Pawn ] & FileMask[ file ] ) )
						MiddleGameScore -= WeakPawn[ MiddleGame ] / 2;
				}
			}

			if( Population( FileMask[ file ] & Position->Pieces[ c ][ Pawn ] ) > 1 )
			{
				MiddleGameScore -= DoubledPawnValue[ MiddleGame ];
				EndGameScore -= DoubledPawnValue[ EndGame ];
			}

			if( PawnDuoMask[ Square ] & Position->Pieces[ c ][ Pawn ] )
			{
				MiddleGameScore += PawnDuo[ MiddleGame ];
				EndGameScore += PawnDuo[ EndGame ];
			}
		}

		if( IsEmpty( PawnPassedMask[ c ][ Square ] & Position->Pieces[ Enemy ][ Pawn ] ) )
			Evaluation.PawnScore.Passed[ c ] |= ( 1 << file );
		else
		{
			if( IsEmpty( FileMask[ file ] & Position->Pieces[ Enemy ][ Pawn ] ) && ( PawnIsolatedMask[ Square ] & Position->Pieces[ c ][ Pawn ] ) && IsEmpty( moveGenerator::GetPawnAttacks( Square, c ) & Position->Pieces[ Enemy ][ Pawn ] ) )
			{
				square s = Square;
				int Attackers = 1;
				int Defenders = 0;
				
				for( ; s != MakeSquare( File( Square ), MyRank[ c ][ RANK7 ] ); s += UpOne )
				{
					if( Mask[ s + UpOne ] & AllPawns )
						break;
					Defenders = Population( moveGenerator::GetPawnAttacks( s, Enemy ) & PawnMoves );
					Attackers = Population( moveGenerator::GetPawnAttacks( s, c ) & Position->Pieces[ Enemy ][ Pawn ] );
					if( Attackers )
						break;
				}

				if( Attackers <= Defenders )
				{
					if( IsEmpty( PawnPassedMask[ c ][ s + UpOne ] & Position->Pieces[ Enemy ][ Pawn ] ) )
					{
						Evaluation.PawnScore.Candidates[ c ] |= ( 1 << file );
						MiddleGameScore += PassedPawnCandidate[ MiddleGame ][ c ][ rank ];
						EndGameScore += PassedPawnCandidate[ EndGame ][ c ][ rank ];
					}
				}
			}
		}

		if( rank == MyRank[ c ][ RANK6 ] && ( Mask[ Square + UpOne ] & Position->Pieces[ Enemy ][ Pawn ] ) )
		{
			square Defender1 = ( c == White ) ? ( Square + 9 ) : ( Square - 7 );
			square Defender2 = ( c == White ) ? ( Square + 7 ) : ( Square - 9 );
			if( ( file < FILEH && ( Mask[ Defender1 ] & Position->Pieces[ c ][ Pawn ] ) && IsEmpty( HiddenRightMask[ c ][ file ] & Position->Pieces[ Enemy ][ Pawn ] ) ) ||
				( file > FILEA && ( Mask[ Defender2 ] & Position->Pieces[ c ][ Pawn ] ) && IsEmpty( HiddenLeftMask[ c ][ file ] & Position->Pieces[ Enemy ][ Pawn ] ) ) )
			{
				MiddleGameScore += PassedPawnHidden[ MiddleGame ];
				EndGameScore += PassedPawnHidden[ EndGame ];
			}
		}
	}

	Evaluation.PawnScore.QueenSideDefects[ c ] = EvaluateKingsFile( c, FILEB );
	Evaluation.PawnScore.DFileDefects[ c ] = EvaluateKingsFile( c, FILED );
	Evaluation.PawnScore.EFileDefects[ c ] = EvaluateKingsFile( c, FILEE );
	Evaluation.PawnScore.KingSideDefects[ c ] = EvaluateKingsFile( c, FILEG );

	Evaluation.PawnScore.MiddleGameScore += Sign[ c ] * MiddleGameScore;
	Evaluation.PawnScore.EndGameScore += Sign[ c ] * EndGameScore;
}

void evaluator::EvaluateKnights( color c )
{
	int MiddleGameScore = 0;
	int EndGameScore = 0;
	color Enemy = ( c == White ) ? Black : White;

	bitboard MyKnights = Position->Pieces[ c ][ Knight ];
	while( MyKnights )
	{
		square s = LSBi( MyKnights );

		MiddleGameScore += PieceSquare[ Knight ][ MiddleGame ][ MySquare[ c ][ s ] ];
		EndGameScore += PieceSquare[ Knight ][ EndGame ][ MySquare[ c ][ s ] ];

		if( IsEmpty( NoPawnAttackMask[ Enemy ][ s ] & Position->Pieces[ Enemy ][ Pawn ] ) )
		{
			if( KnightOutpost[ c ][ s ] )
			{
				MiddleGameScore += KnightOutpost[ c ][ s ];
				EndGameScore += KnightOutpost[ c ][ s ];

				if( KnightOutpost[ c ][ s ] && moveGenerator::GetPawnAttacks( s, Enemy ) & Position->Pieces[ c ][ Pawn ] )
				{
					MiddleGameScore += KnightOutpost[ c ][ s ] / 2;
					EndGameScore += KnightOutpost[ c ][ s ] / 2;
					
					if( IsEmpty( Position->Pieces[ Enemy ][ Knight ] ) && IsEmpty( SquaresOfSameColor( s ) & Position->Pieces[ Enemy ][ Bishop ] ) )
					{
						MiddleGameScore += KnightOutpost[ c ][ s ];
						EndGameScore += KnightOutpost[ c ][ s ];
					}
				}
			}
		}

		bitboard Moves = KnightOffsets[ s ] & ~Position->AllPieces[ c ];
		int MobilityScore = MobilityBase[ Knight ];
		for( int i = 0; i < 4; i++ )
			MobilityScore += Population( Moves & KnightMobilityMasks[ i ] ) * KnightMobilityScore[ i ];
		MiddleGameScore += MobilityScore;
		EndGameScore += MobilityScore;

		if( Evaluation.Dangerous[ c ] )
			Evaluation.Tropism[ c ] += KingTropism[ Knight ][ (int)Distance( s, Evaluation.KingSquare[ Enemy ] ) ];

		MyKnights ^= Mask[ s ];
	}

	Evaluation.Scores[ MiddleGame ] += Sign[ c ] * MiddleGameScore;
	Evaluation.Scores[ EndGame ] += Sign[ c ] * EndGameScore;
}

void evaluator::EvaluateBishops( color c )
{
	int MiddleGameScore = 0;
	int EndGameScore = 0;
	color Enemy = ( c == White ) ? Black : White;

	bitboard MyBishops = Position->Pieces[ c ][ Bishop ];
	int Pair = ( MyBishops & ( MyBishops - 1 ) ) ? 1 : 0;
	while( MyBishops )
	{
		square s = LSBi( MyBishops );

		MiddleGameScore += PieceSquare[ Bishop ][ MiddleGame ][ MySquare[ c ][ s ] ];
		EndGameScore += PieceSquare[ Bishop ][ MiddleGame ][ MySquare[ c ][ s ] ];

		if( IsEmpty( NoPawnAttackMask[ Enemy ][ s ] & Position->Pieces[ Enemy ][ Pawn ] ) )
		{
			if( BishopOutpost[ c ][ s ] )
			{
				MiddleGameScore += BishopOutpost[ c ][ s ];
				EndGameScore += BishopOutpost[ c ][ s ];

				if( BishopOutpost[ c ][ s ] && moveGenerator::GetPawnAttacks( s, Enemy ) & Position->Pieces[ c ][ Pawn ] )
				{
					MiddleGameScore += BishopOutpost[ c ][ s ] / 2;
					EndGameScore += BishopOutpost[ c ][ s ] / 2;

					if( IsEmpty( Position->Pieces[ Enemy ][ Knight ] ) && IsEmpty( SquaresOfSameColor( s ) & Position->Pieces[ Enemy ][ Bishop ] ) )
					{
						MiddleGameScore += BishopOutpost[ c ][ s ];
						EndGameScore += BishopOutpost[ c ][ s ];
					}
				}
			}
		}

		bitboard EnemyPawns = Position->Pieces[ Enemy ][ Pawn ];
		if( s == MySquare[ c ][ A7 ] && ( EnemyPawns & Mask[ MySquare[ c ][ B6 ] ] ) )
		{
			MiddleGameScore -= BishopTrapped;
			EndGameScore -= BishopTrapped;
		}
		else if( s == MySquare[ c ][ B8 ] && ( EnemyPawns & Mask[ MySquare[ c ][ C7 ] ] ) )
		{
			MiddleGameScore -= BishopTrapped;
			EndGameScore -= BishopTrapped;
		}
		else if( s == MySquare[ c ][ H7 ] && ( EnemyPawns & Mask[ MySquare[ c ][ G6 ] ] ) )
		{
			MiddleGameScore -= BishopTrapped;
			EndGameScore -= BishopTrapped;
		}
		else if( s == MySquare[ c ][ G8 ] && ( EnemyPawns & Mask[ MySquare[ c ][ F7 ] ] ) )
		{
			MiddleGameScore -= BishopTrapped;
			EndGameScore -= BishopTrapped;
		}

		bitboard Moves = moveGenerator::GetBishopMoves( s, Position->BothPieces ) &  ~Position->AllPieces[ c ];
		Moves |= Mask[ s ];
		int MobilityScore = MobilityBase[ Bishop ];
		for( int i = 0; i < 4; i++ )
			MobilityScore += Population( Moves & BishopMobilityMasks[ i ] ) * BishopMobilityScore[ Pair ][ i ];
		MiddleGameScore += MobilityScore;
		EndGameScore += MobilityScore;

		bitboard AllPawns = Position->Pieces[ White ][ Pawn ] | Position->Pieces[ Black ][ Pawn ];
		if( AllPawns & FileMaskABC && AllPawns & FileMaskFGH )
		{
			MiddleGameScore += BishopWithWingPawns[ MiddleGame ];
			EndGameScore += BishopWithWingPawns[ EndGame ];
		}

		if( Evaluation.Dangerous[ c ] )
		{
			bitboard Moves = moveGenerator::GetBishopMoves( s, Position->BothPieces & ~Position->Pieces[ c ][ Queen ] );
			int Tropism = ( Moves & KingOffsets[ Evaluation.KingSquare[ Enemy ] ] ) ? 1 : Distance( s, Evaluation.KingSquare[ Enemy ] );
			Evaluation.Tropism[ c ] += KingTropism[ Bishop ][ Tropism ];
		}

		MyBishops ^= Mask[ s ];
	}

	Evaluation.Scores[ MiddleGame ] += Sign[ c ] * MiddleGameScore;
	Evaluation.Scores[ EndGame ] += Sign[ c ] * EndGameScore;
}

void evaluator::EvaluateRooks( color c )
{
	int MiddleGameScore = 0;
	int EndGameScore = 0;
	color Enemy = ( c == White ) ? Black : White;

	bitboard MyPawns = Position->Pieces[ c ][ Pawn ];
	bitboard HisPawns = Position->Pieces[ Enemy ][ Pawn ];
	bitboard MyRooks = Position->Pieces[ c ][ Rook ];
	while( MyRooks )
	{
		square s = LSBi( MyRooks );
		int file = File( s );
		int rank = Rank( s );
		int RelativeRank = MyRank[ c ][ Rank( s ) ];

		if( IsEmpty( FileMask[ file ] & MyPawns ) )
		{
			if( IsEmpty( FileMask[ file ] & HisPawns ) )
			{
				MiddleGameScore += RookOnOpenFile[ MiddleGame ];
				EndGameScore += RookOnOpenFile[ EndGame ];
			}
			else
			{
				MiddleGameScore += RookOnHalfOpenFile[ MiddleGame ];
				EndGameScore += RookOnHalfOpenFile[ EndGame ];
			}
		}

		if( RelativeRank == RANK1 )
		{
			if( Rank( Evaluation.KingSquare[ c ] ) == rank )
			{
				int KingFile = File( Evaluation.KingSquare[ c ] );
				if( KingFile > FILEE )
				{
					if( file > KingFile )
					{
						MiddleGameScore -= RookTrapped;
						EndGameScore -= RookTrapped;
					}
				}
				else if( KingFile < FILED )
				{
					if( file < KingFile )
					{
						MiddleGameScore -= RookTrapped;
						EndGameScore -= RookTrapped;
					}
				}
			}
		}
		else if( RelativeRank == RANK7 )
		{
			MiddleGameScore += RookOn7th[ MiddleGame ];
			EndGameScore += RookOn7th[ EndGame ];
		}

		bitboard Moves = moveGenerator::GetRookMoves( s, Position->BothPieces ) & ~Position->AllPieces[ c ];
		Moves |= Mask[ s ];
		int MobilityScore = -1;
		int SquaresTargetted = -6;
		for( int i = 0; i < 4; i++ )
		{
			int Pop = Population( Moves & RookMobilityMasks[ i ] );
			MobilityScore += Pop * RookMobilityScore[ i ];
			SquaresTargetted += Pop;
		}

		MiddleGameScore += RookMobilityCurve[ MobilityScore ];
		EndGameScore += 3 * SquaresTargetted;

		if( Evaluation.Dangerous[ c ] )
		{
			bitboard TropismMoves = moveGenerator::GetRookMoves( s, Position->BothPieces & ~( Position->Pieces[ c ][ Rook ] | Position->Pieces[ c ][ Queen ] ) );
			int Tropism = ( TropismMoves & KingOffsets[ Evaluation.KingSquare[ Enemy ] ] ) ? 1 : Distance( s, Evaluation.KingSquare[ Enemy ] );
			Evaluation.Tropism[ c ] += KingTropism[ Rook ][ Tropism ];
		}

		MyRooks ^= Mask[ s ];
	}

	Evaluation.Scores[ MiddleGame ] += Sign[ c ] * MiddleGameScore;
	Evaluation.Scores[ EndGame ] += Sign[ c ] * EndGameScore;
}

void evaluator::EvaluateQueens( color c )
{
	int MiddleGameScore = 0;
	int EndGameScore = 0;
	color Enemy = ( c == White ) ? Black : White;
	
	bitboard MyQueens = Position->Pieces[ c ][ Queen ];
	while( MyQueens )
	{
		square s = LSBi( MyQueens );
		square EnemyKingSquare = Evaluation.KingSquare[ Enemy ];

		MiddleGameScore += PieceSquare[ Queen ][ MiddleGame ][ MySquare[ c ][ s ] ];
		EndGameScore += PieceSquare[ Queen ][ EndGame ][ MySquare[ c ][ s ] ];

		if( Evaluation.Dangerous[ c ] )
			Evaluation.Tropism[ c ] += KingTropism[ Queen ][ (int)Distance( s, EnemyKingSquare ) ];

		int ManhattanDistance = RankDistance( EnemyKingSquare, s ) + FileDistance( EnemyKingSquare, s );
		MiddleGameScore += ( 8 - ManhattanDistance );
		EndGameScore += ( 8 - ManhattanDistance );

		MyQueens ^= Mask[ s ];
	}

	Evaluation.Scores[ MiddleGame ] += Sign[ c ] * MiddleGameScore;
	Evaluation.Scores[ EndGame ] += Sign[ c ] * EndGameScore;
}

void evaluator::EvaluateKings( color c )
{
	int MiddleGameScore = 0;
	int EndGameScore = 0;
	int Defects = 0;
	color Enemy = ( c == White ) ? Black : White;
	square Square = Evaluation.KingSquare[ c ];

	bitboard AllPawns = Position->Pieces[ White ][ Pawn ] | Position->Pieces[ Black ][ Pawn ];

	if( AllPawns )
	{
		if( ( AllPawns & FileMaskEFGH ) && ( AllPawns & FileMaskABCD ) )
			EndGameScore += KingPieceSquareMiddle[ c ][ Square ];
		else if( AllPawns & FileMaskEFGH )
			EndGameScore += KingPieceSquareKingSide[ c ][ Square ];
		else
			EndGameScore += KingPieceSquareQueenSide[ c ][ Square ];
	}

	if( Evaluation.Dangerous[ Enemy ] )
	{
		// TODO: Trojan Check 640-647
		int Castling = Evaluation.CurrentCastlingPrivileges & CastlingFlags[ c ];
		if( Castling == 0 )
		{
			int file = File( Square );
			if( file >= FILEE )
			{
				if( file > FILEE )
					Defects = Evaluation.PawnScore.KingSideDefects[ c ];
				else
					Defects = Evaluation.PawnScore.EFileDefects[ c ];
			}
			else
			{
				if( file < FILED )
					Defects = Evaluation.PawnScore.QueenSideDefects[ c ];
				else
					Defects = Evaluation.PawnScore.DFileDefects[ c ];
			}
		}
		else
		{
			if( Castling == CastlingFlags[ c ] )
				Defects = min( Evaluation.PawnScore.KingSideDefects[ c ], min( Evaluation.PawnScore.QueenSideDefects[ c ], Evaluation.PawnScore.EFileDefects[ c ] ) );
			else if( Castling == KingSideAble[ c ] )
				Defects = min( Evaluation.PawnScore.KingSideDefects[ c ], Evaluation.PawnScore.EFileDefects[ c ] );
			else
				Defects = min( Evaluation.PawnScore.QueenSideDefects[ c ], Evaluation.PawnScore.EFileDefects[ c ] );

			if( Defects < 3 )
				Defects = 3;
		}

		int UpOne = ( c == White ) ? 8 : -8;
		if( Position->Pieces[ c ][ King ] & ( RankMask[ MyRank[ c ][ RANK1 ] ] | RankMask[ MyRank[ c ][ RANK2 ] ] ) )
			if( Position->Pieces[ Enemy ][ Pawn ] & ( KingOffsets[ Square ] | KingOffsets[ Square + UpOne ] ) )
				Evaluation.Tropism[ Enemy ] += 3;

		if( Evaluation.Tropism[ Enemy ] < 0 )
			Evaluation.Tropism[ Enemy ] = 0;
		else if( Evaluation.Tropism[ Enemy ] > 15 )
			Evaluation.Tropism[ Enemy ] = 15;
		if( Defects > 15 )
			Defects = 15;

		MiddleGameScore -= KingSafetyScore[ Defects ][ Evaluation.Tropism[ Enemy ] ];
	}

	Evaluation.Scores[ MiddleGame ] += Sign[ c ] * MiddleGameScore;
	Evaluation.Scores[ EndGame ] += Sign[ c ] * EndGameScore;
}

void evaluator::CalculateMaterial()
{
	int population;

	for( color c = White; c <= Black; c++ )
	{
		for( type t = Knight; t <= Queen; t++ )
		{
			int population = Population( Position->Pieces[ c ][ t ] );

			Evaluation.SimpleMaterial[ c ] += SimplePieceValues[ t ] * population;
			Evaluation.PieceMaterial[ c ] += PieceValues[ t ] * population;
		}

		population = Population( Position->Pieces[ c ][ Pawn ] );
		Evaluation.PawnsLeft[ c ] = population;
	}

	Evaluation.Material = PieceValues[ Pawn ] * ( Evaluation.PawnsLeft[ White ] - Evaluation.PawnsLeft[ Black ] );
	Evaluation.Material += Evaluation.PieceMaterial[ White ] - Evaluation.PieceMaterial[ Black ];

	Evaluation.Scores[ MiddleGame ] += Evaluation.Material;
	Evaluation.Scores[ EndGame ] += Evaluation.Material;

	Evaluation.Scores[ MiddleGame ] += ( Position->ToMove == White ) ? 5 : -5;
	Evaluation.Scores[ EndGame ] += ( Position->ToMove == White ) ? 8 : -8;

	Evaluation.MajorPieceBalance = 4 + Population( Position->Pieces[ White ][ Rook ] ) - Population( Position->Pieces[ Black ][ Rook ] ) + 2 * Population( Position->Pieces[ White ][ Queen ] ) - 2 * Population( Position->Pieces[ Black ][ Queen ] );
	Evaluation.MinorPieceBalance = 4 + Population( Position->Pieces[ White ][ Knight ] ) - Population( Position->Pieces[ Black ][ Knight ] ) + Population( Position->Pieces[ White ][ Bishop ] ) - Population( Position->Pieces[ Black ][ Bishop ] );

	Evaluation.MajorPieceBalance = min( max( Evaluation.MajorPieceBalance, 0 ), 8 );
	Evaluation.MinorPieceBalance = min( max( Evaluation.MinorPieceBalance, 0 ), 8 );

	Evaluation.Scores[ MiddleGame ] += Imbalance[ Evaluation.MajorPieceBalance ][ Evaluation.MinorPieceBalance ];
	Evaluation.Scores[ EndGame ] += Imbalance[ Evaluation.MajorPieceBalance ][ Evaluation.MinorPieceBalance ];
}

bool evaluator::EvaluateWinningChances( color c )
{
	color Enemy = ( c == White ) ? Black : White;

	if( IsEmpty( Position->Pieces[ c ][ Pawn ] ) )
	{
		if( Evaluation.SimpleMaterial[ c ] <= 300 )
			return false;

		if( Evaluation.SimpleMaterial[ c ] - Evaluation.SimpleMaterial[ Enemy ] <= 300 && ( Position->Pieces[ Enemy ][ King ] & NotEdgeMask ) )
			return false;
	}

	// TODO: Rook pawns (1902-1938)

	if( Position->Pieces[ c ][ Pawn ] )
		return true;

	if( Evaluation.SimpleMaterial[ c ] == 600 && Evaluation.SimpleMaterial[ Enemy ] == 300 && ( Position->Pieces[ c ][ Knight ] || IsEmpty( Position->Pieces[ Enemy ][ Knight ] ) ) )
		return false;

	if( Evaluation.SimpleMaterial[ c ] == 600 && IsEmpty( Position->Pieces[ c ][ Bishop ] ) && Evaluation.SimpleMaterial[ Enemy ] == 0 && Evaluation.PawnsLeft[ Enemy ] == 0 )
		return false;

	// TODO: KRPxKR and KQPxKQ (1982-1993)

	return true;
}

void evaluator::EvaluateMate( color Winner )
{
	int Score = 0;
	color Loser = ( Winner == White ) ? Black : White;

	if( Evaluation.SimpleMaterial[ Loser ] == 0 && Evaluation.SimpleMaterial[ Winner ] == 6 && Population( Position->Pieces[ Winner ][ Bishop ] ) == 1 )
	{
		if( DarkSquareMask & Position->Pieces[ Winner ][ Bishop ] )
			Score = DarkBishopKnightMateScores[ Evaluation.KingSquare[ Loser ] ];
		else
			Score = LightBishopKnightMateScores[ Evaluation.KingSquare[ Loser ] ];
	}
	else
	{
		Score = MateScores[ Evaluation.KingSquare[ Loser ] ];
		Score -= ( Distance( Evaluation.KingSquare[ Winner ], Evaluation.KingSquare[ Loser ] ) - 3 ) * KingKingTropism;
	}

	Evaluation.Scores[ MiddleGame ] += Sign[ Winner ] * Score;
	Evaluation.Scores[ EndGame ] += Sign[ Winner ] * Score;
}

void evaluator::EvaluateWinnability()
{
	for( color c = White; c <= Black; c++ )
		Evaluation.Dangerous[ c ] = ( Position->Pieces[ c ][ Queen ] && Evaluation.SimpleMaterial[ c ] > 1300 ) || ( Population( Position->Pieces[ c ][ Rook ] ) > 1 && Evaluation.SimpleMaterial[ c ] > 1500 );

	Evaluation.CanWin[ White ] = Evaluation.CanWin[ Black ] = true;

	if( Evaluation.SimpleMaterial[ White ] < 1300 && Evaluation.SimpleMaterial[ Black ] < 1300 )
	{
		bitboard NotRookPawns = FileMask[ FILEA ] | FileMask[ FILEH ];
		if( Evaluation.SimpleMaterial[ White ] == 0 && Evaluation.SimpleMaterial[ Black ] == 0 && ( Position->Pieces[ White ][ Pawn ] & NotRookPawns ) && ( Position->Pieces[ Black ][ Pawn ] & NotRookPawns ) )
			;
		else
		{
			if( abs( Evaluation.MajorPieceBalance ) == 1 )
				if( Evaluation.MajorPieceBalance == -Evaluation.MinorPieceBalance )
					for( color c = White; c <= Black; c++ )
						if( IsEmpty( Position->Pieces[ c ][ Pawn ] ) )
							Evaluation.CanWin[ c ] = false;

			if( Evaluation.CanWin[ White ] || Evaluation.CanWin[ Black ] )
				for( color c = White; c <= Black; c++ )
					if( !EvaluateWinningChances( c ) )
						Evaluation.CanWin[ c ] = false;
		}
	}
}

void evaluator::EvaluateDevelopment( color c )
{
	color Enemy = ( c == White ) ? Black : White;
	int MiddleGameScore = 0;
	bitboard MyPawns = Position->Pieces[ c ][ Pawn ];
	bitboard MyMinors = Position->Pieces[ c ][ Bishop ] | Position->Pieces[ c ][ Knight ];

	if( IsEmpty( Mask[ MySquare[ c ][ E4 ] ] & MyPawns ) && ( Mask[ MySquare[ c ][ D4 ] ] & MyPawns ) )
	{
		if( ( Mask[ MySquare[ c ][ C2 ] ] & MyPawns ) && ( Mask[ MySquare[ c ][ C3 ] ] & MyMinors ) )
			MiddleGameScore -= DevelopmentThematic;
	}

	int MyRootCastling = Evaluation.RootCastlingPrivileges & CastlingFlags[ c ]; // TODO: Double check this section. Crafty's version seems odd.
	int MyCurrentCastling = Evaluation.CurrentCastlingPrivileges & CastlingFlags[ c ];
	if( MyRootCastling )
	{
		int Multiplier = Position->Pieces[ Enemy ][ Queen ] ? 3 : 1;

		if( MyRootCastling != MyCurrentCastling )
		{
			if( MyCurrentCastling == 0 )
				MiddleGameScore -= Multiplier * DevelopmentLosingCastle;
			else
				MiddleGameScore -= Multiplier * DevelopmentLosingCastle / 2;
		}
		else
			MiddleGameScore -= Multiplier * DevelopmentNotCastled;
	}

	if( Mask[ MySquare[ c ][ A1 ] ] & Position->Pieces[ c ][ Rook ] )
		if( Mask[ MySquare[ c ][ B1 ] ] & Position->Pieces[ c ][ Knight ] )
			MiddleGameScore -= UndevelopedPiece;

	if( Mask[ MySquare[ c ][ H1 ] ] & Position->Pieces[ c ][ Rook ] )
		if( Mask[ MySquare[ c ][ G1 ] ] & Position->Pieces[ c ][ Knight ] )
			MiddleGameScore -= UndevelopedPiece;

	Evaluation.Scores[ MiddleGame ] += Sign[ c ] * MiddleGameScore;
}

void evaluator::EvaluatePieces()
{
	Evaluation.Tropism[ White ] = Evaluation.Tropism[ Black ] = 0;

	for( color c = White; c <= Black; c++ )
	{
		EvaluateKnights( c );
		EvaluateBishops( c );
		EvaluateRooks( c );
		EvaluateQueens( c );
	}

	for( color c = White; c <= Black; c++ )
		EvaluateKings( c );
}

void evaluator::InitEval( position* InPosition, int RootCastlingFlags )
{
	if( Verbose )
		VerboseEval.clear();

	this->Position = InPosition;
	Done = false;

	memset( &Evaluation, 0, sizeof( evaluation ) );

	for( color c = White; c <= Black; c++ )
		Evaluation.KingSquare[ c ] = LSBi( Position->Pieces[ c ][ King ] );

	Evaluation.CurrentCastlingPrivileges = InPosition->Castling;
	Evaluation.RootCastlingPrivileges = RootCastlingFlags;
}

int evaluator::EvaluateDraws( int Score )
{
	int TotalWhitePieces = Evaluation.SimpleMaterial[ White ];
	int TotalBlackPieces = Evaluation.SimpleMaterial[ Black ];
	int WhiteBishops = Population( Position->Pieces[ White ][ Bishop ] );
	int BlackBishops = Population( Position->Pieces[ Black ][ Bishop ] );
	int WhitePawns = Population( Position->Pieces[ White ][ Pawn ] );
	int BlackPawns = Population( Position->Pieces[ Black ][ Pawn ] );

	bool OppositeColorBishops = IsEmpty( SquaresOfSameColor( LSBi( Position->Pieces[ White ][ Bishop ] ) ) & Position->Pieces[ Black ][ Bishop ] );

	if( TotalWhitePieces <= 800 && TotalBlackPieces <= 800 )
	{
		if( WhiteBishops == 1 && BlackBishops == 1 )
		{
			if( OppositeColorBishops )
			{
				if( TotalWhitePieces == 300 && TotalBlackPieces == 300 && ( ( WhitePawns < 4 && BlackPawns < 4 ) || ( abs( WhitePawns - BlackPawns ) < 2 ) ) )
					Score /= 2;
				else
					Score = 3 * Score / 4;
			}
		}
	}

	if( !Evaluation.CanWin[ White ] || !Evaluation.CanWin[ Black ] )
	{
		if( !Evaluation.CanWin[ White ] && Score > DRAWSCORE )
			Score = DRAWSCORE;
		else if( !Evaluation.CanWin[ Black ] && Score < DRAWSCORE )
			Score = DRAWSCORE;
	}

	if( Position->Fifty > 80 )
	{
		int Scale = 101 - Position->Fifty;
		Score = DRAWSCORE + Score * Scale / 20;
	}

	return Score;
}

void evaluator::FinishEval( evaluation* Out )
{
	int MiddleGameScore = Evaluation.Scores[ MiddleGame ];
	int EndGameScore = Evaluation.Scores[ EndGame ];
	int Phase = Evaluation.GamePhase;

	if( Position->ToMove == White )
	{
		MiddleGameScore += TempoValue[ MiddleGame ];
		EndGameScore += TempoValue[ EndGame ];
	}
	else
	{
		MiddleGameScore -= TempoValue[ MiddleGame ];
		EndGameScore -= TempoValue[ EndGame ];
	}

	Evaluation.FinalScore = ( ( MiddleGameScore * Phase ) + ( EndGameScore * ( 6200 - Phase ) ) ) / 6200;
	Evaluation.FinalScore = EvaluateDraws( Evaluation.FinalScore );

	if( Position->ToMove == Black )
		Evaluation.FinalScore = -Evaluation.FinalScore;

	if( Out != NULL )
		*Out = Evaluation;

	if( !Verbose )
		assert( VerboseEval.size() == 0 );
}

int evaluator::PieceMaterial( const position* const InPosition )
{
	int r = 0;

	for( type t = Knight; t <= Queen; t++ )
		r += SimplePieceValues[ t ] * Population( InPosition->Pieces[ White ][ t ] );

	for( type t = Knight; t <= Queen; t++ )
		r -= SimplePieceValues[ t ] * Population( InPosition->Pieces[ Black ][ t ] );

	return r;
}

int evaluator::Evaluate( position* InPosition, int RootCastlingFlags, int Alpha, int Beta, evaluation* Out )
{	
	InitEval( InPosition, RootCastlingFlags );

	CalculateMaterial();
	Evaluation.GamePhase = min( 6200, Evaluation.SimpleMaterial[ White ] + Evaluation.SimpleMaterial[ Black ] );

	EvaluateWinnability();

	bitboard AllPawns = Position->Pieces[ White ][ Pawn ] | Position->Pieces[ Black ][ Pawn ];
	if( IsEmpty( AllPawns ) )
	{
		if( Evaluation.SimpleMaterial[ White ] > 300 || Evaluation.SimpleMaterial[ Black ] > 300 )
		{
			if( Evaluation.Material > 0 )
				EvaluateMate( White );
			else if( Evaluation.Material < 0 )
				EvaluateMate( Black );

			if( Evaluation.Scores[ EndGame ] > DRAWSCORE && !Evaluation.CanWin[ White ] )
				Evaluation.Scores[ EndGame ] /= 4;

			if( Evaluation.Scores[ EndGame ] < DRAWSCORE && !Evaluation.CanWin[ Black ] )
				Evaluation.Scores[ EndGame ] /= 4;

			Evaluation.FinalScore = Evaluation.Scores[ EndGame ];
			if( Position->ToMove == Black )
				Evaluation.FinalScore = -Evaluation.FinalScore;
			return Evaluation.FinalScore;
		}
	}
	else
	{
		// TODO: Compare with Crafty's Pawn Hashing Stuff 157-184
		if( LastPawnScore.Hash != Position->PawnHash )
		{
			Evaluation.PawnScore.Hash = Position->PawnHash;

			for( color c = White; c <= Black; c++ )
				EvaluatePawns( c );

			LastPawnScore = Evaluation.PawnScore;
		}
		else
			Evaluation.PawnScore = LastPawnScore;

		Evaluation.Scores[ MiddleGame ] += Evaluation.PawnScore.MiddleGameScore;
		Evaluation.Scores[ EndGame ] += Evaluation.PawnScore.EndGameScore;
	}

	if( Evaluation.PawnScore.Passed[ White ] || Evaluation.PawnScore.Passed[ Black ] )
	{
		for( color c = White; c <= Black; c++ )
			if( Evaluation.PawnScore.Passed[ c ] )
				EvaluatePassedPawns( c );
		if( ( Evaluation.SimpleMaterial[ White ] == 0 && Evaluation.PawnScore.Passed[ Black ] ) || ( Evaluation.SimpleMaterial[ Black ] == 0 && Evaluation.PawnScore.Passed[ White ] ) )
			EvaluatePassedPawnRaces( Position->ToMove );
	}

	for( color c = White; c <= Black; c++ )
		EvaluateDevelopment( c );

	int Score = ( Evaluation.Scores[ MiddleGame ] * Evaluation.GamePhase + Evaluation.Scores[ EndGame ] * ( 6200 - Evaluation.GamePhase ) ) / 6200;
	if( Position->ToMove == Black )
		Score = -Score;

	// TODO: Faster way to do this population?
	/*int TotalPieces = 0;// Population( 	Position->Pieces[ White ][ Knight ] | Position->Pieces[ White ][ Bishop ] | Position->Pieces[ White ][ Rook ] | Position->Pieces[ White ][ Queen ] |
						//				Position->Pieces[ Black ][ Knight ] | Position->Pieces[ Black ][ Bishop ] | Position->Pieces[ Black ][ Rook ] | Position->Pieces[ Black ][ Queen ] );
	int Cutoff = 125 + TotalPieces * 4;
	if( Score - Cutoff < Beta && Score + Cutoff > Alpha )*/
		EvaluatePieces();

	FinishEval( Out );

	assert( abs( Evaluation.FinalScore ) <= MATESCORE );
	return Evaluation.FinalScore;
}
