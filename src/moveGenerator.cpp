#include <cmath>
#include "moveGenerator.h"
#include "Magic.h"
using std::abs;

moveGenerator::~moveGenerator() {}

bitboard moveGenerator::GetPawnForwards( const position* Position, square s, color c )
{
	bitboard SquareMask = Mask[ s ];
	bitboard UpOneMask = ( c == White ) ? ( SquareMask << 8 ) : ( SquareMask >> 8 );
	UpOneMask &= ~Position->BothPieces;
	bitboard ThirdRank = RankMask[ MyRank[ c ][ RANK3 ] ];
	bitboard UpTwoMask = ( c == White ) ? ( ( UpOneMask & ThirdRank ) << 8 ) : ( ( UpOneMask & ThirdRank ) >> 8 );
	UpTwoMask &= ~Position->BothPieces;
	return UpOneMask | UpTwoMask;
}

bitboard moveGenerator::GetPawnAttacks( square s, color c )
{
	return PawnAttacks[ c ][ s ];
}

bitboard moveGenerator::GetKnightMoves( const position* Position, square s, color c )
{
	return KnightOffsets[ s ];
}

bitboard moveGenerator::GetBishopMoves( square s, bitboard occupancy )
{
	occupancy &= MagicBishopMasks[ s ];
	bitboard index = ( occupancy * MagicBishopMultipliers[ s ] ) >> MagicBishopShifts[ s ];
	return *( MagicBishopIndices[ s ] + index );
}

bitboard moveGenerator::GetBishopMoves( const position* Position, square s, color c )
{
	bitboard occupancy = Position->BothPieces & MagicBishopMasks[ s ];
	bitboard index = ( occupancy * MagicBishopMultipliers[ s ] ) >> MagicBishopShifts[ s ];
	return *( MagicBishopIndices[ s ] + index );
}

bitboard moveGenerator::GetRookMoves( square s, bitboard occupancy )
{
	occupancy &= MagicRookMasks[ s ];
	bitboard index = ( occupancy * MagicRookMultipliers[ s ] ) >> MagicRookShifts[ s ];
	return *( MagicRookIndices[ s ] + index );
}

bitboard moveGenerator::GetRookMoves( const position* Position, square s, color c )
{
	bitboard occupancy = Position->BothPieces & MagicRookMasks[ s ];
	bitboard index = ( occupancy * MagicRookMultipliers[ s ] ) >> MagicRookShifts[ s ];
	return *( MagicRookIndices[ s ] + index );
}

bitboard moveGenerator::GetQueenMoves( square s, bitboard Occupancy )
{
	return GetBishopMoves( s, Occupancy ) | GetRookMoves( s, Occupancy );
}

bitboard moveGenerator::GetQueenMoves( const position* Position, square s, color c )
{
	return GetBishopMoves( Position, s, c ) | GetRookMoves( Position, s, c );
}

bitboard moveGenerator::GetKingMoves( const position* Position, square s, color c )
{
	return KingOffsets[ s ];
}

bitboard moveGenerator::GetBishopXRay( square s, bitboard occupancy )
{
	return GetBishopMoves( s, occupancy & ~GetBishopMoves( s, occupancy ) );
}

bitboard moveGenerator::GetRookXRay( square s, bitboard occupancy )
{
	return GetRookMoves( s, occupancy & ~GetRookMoves( s, occupancy ) );
}

bitboard moveGenerator::GetPinnedPieces( const position* Position, color c )
{
	color Enemy = ( c == White ) ? Black : White;
	square KingSquare = LSBi( Position->Pieces[ c ][ King ]  );

	bitboard PinnedPieces = 0;
	bitboard Pinners, b;

	square s;
	direction Direction;
	
	Pinners = Position->Pieces[ Enemy ][ Queen ] | Position->Pieces[ Enemy ][ Rook ];
	b = ( GetRookXRay( KingSquare, Position->BothPieces ) ) & Pinners;
	while( b )
	{
		s = LSBi( b );
		b ^= Mask[ s ];
		PinnedPieces |= GetRookMoves( s, Position->BothPieces ) & GetRookMoves( KingSquare, Position->BothPieces ) & Position->AllPieces[ c ];
	}

	Pinners = Position->Pieces[ Enemy ][ Queen ] | Position->Pieces[ Enemy ][ Bishop ];
	b = ( GetBishopXRay( KingSquare, Position->BothPieces ) ) & Pinners;
	while( b )
	{
		s = LSBi( b );
		b ^= Mask[ s ];
		PinnedPieces |= GetBishopMoves( s, Position->BothPieces ) & GetBishopMoves( KingSquare, Position->BothPieces ) & Position->AllPieces[ c ];
	}

	return PinnedPieces;
}

bitboard moveGenerator::GetLegalPawnForwards( const position* Position, square s, color c )
{
	direction d = abs( Directions[ s ][ LSBi( Position->Pieces[ c ][ King ] ) ] );

	if( d == 8 )
		return GetPawnForwards( Position, s, c );
	else
		return 0;
}

bitboard moveGenerator::GetLegalPawnAttacks( const position* Position, square s, color c )
{
	square KingSquare = LSBi( Position->Pieces[ c ][ King ] );
	direction PinDirection = abs( Directions[ s ][ KingSquare ] );

	square Cap1 = s + ( ( c == White ) ? 7 : -9 );
	square Cap2 = s + ( ( c == White ) ? 9 : -7 );

	bitboard r = 0;

	if( IsEmpty( FileMask[ FILEA ] & Mask[ s ] ) )
		if( Cap1 >= A1 && Cap1 <= H8 && PinDirection == abs( Cap1 - s ) )
			r |= Mask[ Cap1 ];

	if( IsEmpty( FileMask[ FILEH ] & Mask[ s ] ) )
		if( Cap2 >= A1 && Cap2 <= H8 && PinDirection == abs( Cap2 - s ) )
			r |= Mask[ Cap2 ];

	bitboard Targets = Position->AllPieces[ ( c == White ) ? Black : White ];
	if( Position->EPSquare != NullSquare )
		Targets |= Mask[ Position->EPSquare ];
	r &= Targets;
	return r;
}

bitboard moveGenerator::GetLegalKnightMoves( const position* Position, square s, color c )
{
	return 0;
}

bitboard moveGenerator::GetLegalBishopMoves( const position* Position, square s, color c )
{
	direction PinDirection = abs( Directions[ s ][ LSBi( Position->Pieces[ c ][ King ] ) ] );
	if( PinDirection == 1 || PinDirection == 8 )
		return 0;

	bitboard AllMoves = GetBishopMoves( Position, s, c );
	if( PinDirection == 7 )
		AllMoves &= RayPlus7[ s ] | RayMinus7[ s ];
	else if( PinDirection == 9 )
		AllMoves &= RayPlus9[ s ] | RayMinus9[ s ];

	return AllMoves & ~Position->AllPieces[ c ];
}

bitboard moveGenerator::GetLegalRookMoves( const position* Position, square s, color c )
{
	direction PinDirection = abs( Directions[ s ][ LSBi( Position->Pieces[ c ][ King ] ) ] );
	if( PinDirection == 7 || PinDirection == 9 )
		return 0;

	bitboard AllMoves = GetRookMoves( Position, s, c );
	if( PinDirection == 1 )
		AllMoves &= RayPlus1[ s ] | RayMinus1[ s ];
	else if( PinDirection == 8 )
		AllMoves &= RayPlus8[ s ] | RayMinus8[ s ];

	return AllMoves & ~Position->AllPieces[ c ];
}

bitboard moveGenerator::GetLegalQueenMoves( const position* Position, square s, color c )
{
	direction PinDirection = abs( Directions[ s ][ LSBi( Position->Pieces[ c ][ King ] ) ] );

	bitboard AllMoves = GetQueenMoves( Position, s, c );
	if( PinDirection == 1 )
		AllMoves &= RayPlus1[ s ] | RayMinus1[ s ];
	else if( PinDirection == 7 )
		AllMoves &= RayPlus7[ s ] | RayMinus7[ s ];
	else if( PinDirection == 8 )
		AllMoves &= RayPlus8[ s ] | RayMinus8[ s ];
	else if( PinDirection == 9 )
		AllMoves &= RayPlus9[ s ] | RayMinus9[ s ];

	return AllMoves & ~Position->AllPieces[ c ];
}

bitboard moveGenerator::GetLegalKingMoves( const position* Position, square s, color c )
{
	bitboard r = 0;
	color Enemy = ( c == White ) ? Black : White;

	bitboard KingMoves = GetKingMoves( Position, s, c );
	bitboard LeftToCheck = KingMoves;

	while( LeftToCheck )
	{
		square to = LSBi( LeftToCheck );
		if( Position->IsSquareAttackedByColor( to, Enemy ) )
			KingMoves ^= Mask[ to ];
		LeftToCheck ^= Mask[ to ];
	}

	KingMoves |= GetCastlingMoves( Position, s, c );

	return KingMoves & ~Position->AllPieces[ c ];
}


bitboard moveGenerator::GetAllPawnAttacks( const position* Position, color c )
{
	color Enemy = ( c == White ) ? Black : White;
	bitboard Pawns = Position->Pieces[ c ][ Pawn ];
	bitboard Targets = Position->AllPieces[ Enemy ];
	if( Position->EPSquare != NullSquare )
		Targets |= Mask[ Position->EPSquare ];

	bitboard NonA = Pawns & ~FileMask[ FILEA ];
	bitboard NonH = Pawns & ~FileMask[ FILEH ];

	if( c == White )
	{
		NonA <<= 7;
		NonH <<= 9;
	}
	else
	{
		NonA >>= 9;
		NonH >>= 7;
	}

	return ( NonA | NonH ) & Targets;
}

bitboard moveGenerator::GetCastlingMoves( const position* Position, square s, color c )
{
	bitboard b = 0;
	color Enemy = ( c == White ) ? Black : White;

	if( Position->Castling & KingSideAble[ c ] )
		if( IsEmpty( Position->BothPieces & ( Mask[ s + 1 ] | Mask[ s + 2 ] ) ) )
			if( !Position->IsSquareAttackedByColor( s, Enemy ) )
				if( !Position->IsSquareAttackedByColor( s + 1, Enemy ) )
					if( !Position->IsSquareAttackedByColor( s + 2, Enemy ) )
						b |= Mask[ s + 2 ];

	if( Position->Castling & QueenSideAble[ c ] )
		if( IsEmpty( Position->BothPieces & ( Mask[ s - 1 ] | Mask[ s - 2 ] | Mask[ s - 3 ] ) ) )
			if( !Position->IsSquareAttackedByColor( s, Enemy ) )
				if( !Position->IsSquareAttackedByColor( s - 1, Enemy ) )
					if( !Position->IsSquareAttackedByColor( s - 2, Enemy ) )
							b |= Mask[ s - 2 ];
	return b;
}

bitboard moveGenerator::GetLegalMoves( const position* Position, square Square, type t, color ToMove, bool IsPinned )
{
	bitboard Moves;

	switch( t )
	{
		case Pawn:	
			if( IsPinned )
				Moves = GetLegalPawnForwards( Position, Square, ToMove ) | GetLegalPawnAttacks( Position, Square, ToMove );
			else
			{
				bitboard Targets = Position->AllPieces[ Position->Enemy ];
				Moves = GetPawnForwards( Position, Square, ToMove ) | ( GetPawnAttacks( Square, ToMove ) & Targets );

				if( Position->EPSquare != NullSquare )
				{
					if( GetPawnAttacks( Square, ToMove ) & Mask[ Position->EPSquare ] )
					{
						bool IsSafe = true;
						if( Position->Pieces[ ToMove ][ King ] & RankMask[ Rank( Square ) ] )
						{
							square Captured = MakeSquare( File( Position->EPSquare ), Rank( Square ) );
							bitboard RankAttacks = GetRookMoves( Square, Position->BothPieces ^ Mask[ Captured ] );
							RankAttacks &= RankMask[ Rank( Square ) ];
							if( RankAttacks & Position->Pieces[ ToMove ][ King ] )
							{
								color Enemy = ( ToMove == White ) ? Black : White;
								if( RankAttacks & ( Position->Pieces[ Enemy ][ Rook ] | Position->Pieces[ Enemy ][ Queen ] ) )
									IsSafe = false;
							}
						}
						if( IsSafe )
							Moves |= Mask[ Position->EPSquare ];
					}
				}
			}
			break;

		case Knight:
			if( IsPinned )
				Moves = GetLegalKnightMoves( Position, Square, ToMove );
			else
				Moves = GetKnightMoves( Position, Square, ToMove ) & ~Position->AllPieces[ ToMove ];
			break;

		case Bishop:
			if( IsPinned )
				Moves = GetLegalBishopMoves( Position, Square, ToMove );
			else
				Moves = GetBishopMoves( Position, Square, ToMove ) & ~Position->AllPieces[ ToMove ];
			break;

		case Rook:
			if( IsPinned )
				Moves = GetLegalRookMoves( Position, Square, ToMove );
			else
				Moves = GetRookMoves( Position, Square, ToMove ) & ~Position->AllPieces[ ToMove ];
			break;

		case Queen:
			if( IsPinned )
				Moves = GetLegalQueenMoves( Position, Square, ToMove );
			else
				Moves = GetQueenMoves( Position, Square, ToMove ) & ~Position->AllPieces[ ToMove ];
			break;

		case King:
			Moves = GetLegalKingMoves( Position, Square, ToMove );
			break;
	}

	return Moves;
}

type moveGenerator::GetNextPromotion()
{
	type r = NextPromotion;
	if( NextPromotion == Queen )
		NextPromotion = Knight;
	else if( NextPromotion == Knight )
		NextPromotion = Rook;
	else if( NextPromotion == Rook )
		NextPromotion = Bishop;
	else if( NextPromotion == Bishop )
		NextPromotion = Queen;
	return r;
}

LegalMoveGenerator::LegalMoveGenerator( position* Position )
{	
	this->Position = Position;
	this->ToMove = Position->ToMove;
	this->PinnedPieces = moveGenerator::GetPinnedPieces( Position, ToMove );
	this->TargetMask = 0xFFFFFFFFFFFFFFFF;
	Reset();
}

void LegalMoveGenerator::Reset()
{
	this->Movers = Position->AllPieces[ ToMove ];
	this->NextPromotion = Queen;
	this->NextType = 0;

	this->SetSquareAndMoves();
}

void LegalMoveGenerator::Reset( bitboard TargetMask )
{
	this->Movers = Position->AllPieces[ ToMove ];
	this->NextPromotion = Queen;
	this->TargetMask = TargetMask;
	this->NextType = 0;

	this->SetSquareAndMoves();
}

move LegalMoveGenerator::GetNextMove()
{
	while( Moves == 0 )
	{
		if( Movers == 0 )
			return NullMove;
		SetSquareAndMoves();
	}

	square To;

	while( Moves != 0 )
	{
		To = MostAdvanced( ToMove, Moves );

		int BackRank = ( ToMove == White ) ? 7 : 0;
		bool MoverIsPawn = ( Position->Pieces[ ToMove ][ Pawn ] & Mask[ Square ] );
		bool MoveIsToBackRank = ( Rank( To ) == BackRank );
		if( MoverIsPawn && MoveIsToBackRank )
		{
			if( NextPromotion == Bishop )
				Moves ^= Mask[ To ];
			return move( Square, To, GetNextPromotion() );
		}
		else
		{
			Moves ^= Mask[ To ];	
			return move( Square, To, Empty );
		}
	}

	assert( "Bad Incremental Move Generation" && 0 );
	return NullMove;
}

LegalMoveGenerator::LegalMoveGenerator()
{
}

void LegalMoveGenerator::SetSquareAndMoves()
{
	bitboard TempMovers = 0;
	static const type Order[ 6 ] = { Knight, Bishop, Rook, Queen, King, Pawn };
	type Type;
	while( TempMovers == 0 )
	{
		assert( NextType < 6 );
		Type = Order[ NextType ]; 
		TempMovers = Movers & Position->Pieces[ ToMove ][ Type ];	
		if( TempMovers == 0 )
			NextType++;
	}

	Square = MostAdvanced( ToMove, TempMovers );

	Movers ^= Mask[ Square ];

	bool IsPinned = PinnedPieces & Mask[ Square ];
	Moves = GetLegalMoves( Position, Square, Type, ToMove, IsPinned ) & TargetMask;
}

EvasionGenerator::EvasionGenerator( position* Position )
{
	this->Position = Position;
	this->ToMove = Position->ToMove;
	this->Enemy = Position->Enemy;
	this->KingSquare = LSBi( Position->Pieces[ ToMove ][ King ] );
	this->TargetMask = 0xFFFFFFFFFFFFFFFF;

	Reset();
}

void EvasionGenerator::Reset()
{
	this->NextPromotion = Queen;
	Setup();
}

void EvasionGenerator::Reset( bitboard TargetMask )
{
	this->NextPromotion = Queen;
	this->TargetMask = TargetMask;
	Setup();
}

void EvasionGenerator::Setup()
{
	this->Checkers = Position->AttacksTo( KingSquare, Enemy );

	this->KingMoves = moveGenerator::GetKingMoves( Position, KingSquare, ToMove ) & ~Position->AllPieces[ ToMove ] & TargetMask;

	bitboard CheckersLeft = Checkers;
	while( CheckersLeft )
	{
		Checker = LSBi( CheckersLeft );

		if( ( Position->Pieces[ Enemy ][ Pawn ] | Position->Pieces[ Enemy ][ Knight ] ) & Mask[ Checker ] )
			;
		else
			KingMoves &= ~( Ray( Checker, Directions[ Checker ][ KingSquare ] ) & ~Mask[ Checker ] );

		CheckersLeft ^= Mask[ Checker ];
	}

	Phase = KingMovesPhase;
}

void EvasionGenerator::SetupAttackersPhase()
{
	this->PinnedPieces = moveGenerator::GetPinnedPieces( Position, ToMove );
	if( Mask[ Checker ] & TargetMask )
		this->Attackers = Position->AttacksTo( Checker, ToMove ) & ~Mask[ KingSquare ];
	Phase = CapturesPhase;
}

void EvasionGenerator::SetupInterceptionPhase()
{
	IntermediateSquares = GetIntermediateSquares( KingSquare, Checker ) & TargetMask;
	if( IntermediateSquares )
		Interceptors = FindInterceptors( LSBi( IntermediateSquares ) );
	
	Phase = InterceptionPhase;
}

bitboard EvasionGenerator::FindInterceptors( square To )
{
	bitboard Interceptors;

	int Backwards = ( ToMove == White ) ? -8 : +8;
	int FourthRank = ( ToMove == White ) ? 3 : 4;

	Interceptors = Position->AttacksTo( To, ToMove ) & ~Position->Pieces[ ToMove ][ King ] & ~Position->Pieces[ ToMove ][ Pawn ];
	if( To + Backwards < 64 && To + Backwards >= 0 )
		if( Position->Pieces[ ToMove ][ Pawn ] & Mask[ To + Backwards ] )
			Interceptors |= Mask[ To + Backwards ];

	if( RankMask[ FourthRank ] & Mask[ To ] )
		if( Position->Pieces[ ToMove ][ Pawn ] & Mask[ To + 2 * Backwards ] )
			if( IsEmpty( Position->BothPieces & Mask[ To + Backwards ] ) )
				Interceptors |= Mask[ To + 2 * Backwards ];

	return Interceptors;
}

move EvasionGenerator::GetNextMove()
{
	if( Phase == KingMovesPhase )
	{
		while( KingMoves )
		{
			square To = LSBi( KingMoves );
			KingMoves ^= Mask[ To ];

			if( !Position->IsSquareAttackedByColor( To, Enemy ) )
				return move( KingSquare, To, Empty );
		}

		// If there are two checkers then we are done
		if( Checkers & ( Checkers - 1 ) )
			return NullMove;

		SetupAttackersPhase();
	}

	if( Phase == CapturesPhase )
	{
		while( Attackers )
		{
			square From = LSBi( Attackers );

			direction PinDirection = abs( Directions[ From ][ KingSquare ] );
			bool IsPinned = ( PinnedPieces & Mask[ From ] );
			if( !IsPinned || PinDirection == abs( Directions[ From ][ Checker ] ) )
			{
				bool IsMoveToLastRank = ( Checkers & ( RankMask[ 0 ] | RankMask[ 7 ] ) );
				bool IsMoverPawn = ( Position->Pieces[ ToMove ][ Pawn ] & Mask[ From ] );

				if(  IsMoveToLastRank && IsMoverPawn  )
				{
					if( NextPromotion == Bishop )
						Attackers ^= Mask[ From ];
					return move( From, Checker, GetNextPromotion() );
				}
				else
				{
					Attackers ^= Mask[ From ];
					return move( From, Checker, Empty );
				}
			}
			else
				Attackers ^= Mask[ From ];
		}

		Phase = EPPhase;
	}

	if( Phase == EPPhase )
	{
		SetupInterceptionPhase();
		if( Position->EPSquare == Checker + 8 || Position->EPSquare == Checker - 8 )
		{
			if( File( Checker ) != 0 && Position->Pieces[ ToMove ][ Pawn ] & Mask[ Checker - 1 ] )
				if( !Position->GetPinDirection( ToMove, Checker - 1 ) )
					return move( Checker - 1, Position->EPSquare, Empty );

			if( File( Checker ) != 7 && Position->Pieces[ ToMove ][ Pawn ] & Mask[ Checker + 1 ] )
				if( !Position->GetPinDirection( ToMove, Checker + 1 ) )
					return move( Checker + 1, Position->EPSquare, Empty );
		}
	}

	if( Phase == InterceptionPhase )
	{
		while( IntermediateSquares )
		{
			square To = LSBi( IntermediateSquares );
			while( Interceptors )
			{
				square From = LSBi( Interceptors );
				if( !Position->GetPinDirection( ToMove, From ) )
				{
					bool IsMoveToLastRank = ( Mask[ To ] & ( RankMask[ 0 ] | RankMask[ 7 ] ) );
					bool IsMoverPawn = ( Position->Pieces[ ToMove ][ Pawn ] & Mask[ From ] );
					if( IsMoveToLastRank && IsMoverPawn )
					{
						if( NextPromotion == Bishop )
							Interceptors ^= Mask[ From ];
						return move( From, To, GetNextPromotion() );
					}
					else
					{
						Interceptors ^= Mask[ From ];
						return move( From, To, Empty );
					}
				}
				else
					Interceptors ^= Mask[ From ];
			}
			IntermediateSquares ^= Mask[ To ];
			if( IntermediateSquares )
				Interceptors = FindInterceptors( LSBi( IntermediateSquares ) );
		}
	}

	return NullMove;
}

Generator::Generator()
{
	MoveGenerator = 0;
}

Generator::Generator( position* Position )
{
	if( Position->InCheck( Position->ToMove ) )
		this->MoveGenerator = new EvasionGenerator( Position );
	else
		this->MoveGenerator = new LegalMoveGenerator( Position );
}

Generator::~Generator()
{
	if( this->MoveGenerator )
		delete this->MoveGenerator;
	this->MoveGenerator = 0;
}

move Generator::GetNextMove()
{
	return MoveGenerator->GetNextMove();
}

void Generator::Reset()
{
	MoveGenerator->Reset();
}

void Generator::Reset( bitboard TargetMask )
{
	MoveGenerator->Reset( TargetMask );
}
