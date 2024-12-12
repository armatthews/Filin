#include "move.h"
#include "position.h"
#include "moveGenerator.h"

square move::From() const
{
	//return this->from;
	square r = ( Data & 0xFF );
	return r;
}

square move::To() const
{
	//return this->to;
	square r = ( Data >> 8 ) & 0xFF;
	return r;
}

type move::Promote() const
{
	//return this->promote;
	type r = ( Data >> 16 ) & 0xFF;
	return r;
}

void move::SetFrom( square from )
{
	//this->from = from;
	square Mask = ( unsigned char )( from );
	Data &= ~0xFF;
	Data |= ( ( long )( Mask ) << 0 );
}

void move::SetTo( square to )
{
	//this->to = to;

	square Mask = ( unsigned char )( to );
	Data &= ~0xFF00;
	Data |= ( ( long )( Mask ) << 8 );
}

void move::SetPromote( type promote )
{
	Data &= ~0xFF0000;
	square Mask = ( unsigned char )( promote );
	Data |= ( ( long )( Mask ) << 16 );
	//this->promote = promote;
}

vector< square > move::PiecesOfTypeThatCanMoveToSquare( position* Position, type Type, square to ) const
{
	vector< square > r;
	color c = Position->ToMove;
	Generator MoveGenerator( Position );
	move Move;
	
	while( ( Move = MoveGenerator.GetNextMove() ) != NullMove )
		if( ( Position->Pieces[ c ][ Type ] & Mask[ Move.From() ] ) && Move.To() == to && ( Move.Promote() == Queen || Move.Promote() == Empty ) )
			r.push_back( Move.From() );

	return r;
}

bool move::DoesMoveCheck( position* Position ) const
{
	bool r = false;

	Position->MakeMove( *this );
	if( Position->InCheck( Position->ToMove ) )
		r = true;
	Position->TakeBack();

	return r;
}

bool move::CanDistinguishByFile( vector< square >* Squares ) const
{
	for( unsigned int i = 0; i < Squares->size(); i++ )
		if( Squares->at( i ) != this->From() && File( Squares->at( i ) ) == File( this->From() ) )
			return false;

	return true;
}

bool move::CanDistinguishByRank( vector< square >* Squares ) const
{
	for( unsigned int i = 0; i < Squares->size(); i++ )
		if( Squares->at( i ) != this->From() && Rank( Squares->at( i ) ) == Rank( this->From() ) )
			return false;

	return true;
}

void move::SetToNull()
{
	Data = 0;
	SetFrom( NullSquare );
	SetTo( NullSquare );
	SetPromote( Empty );
}

move::move()
{
	SetToNull();
}

move::move( string Coords )
{
	SetToNull();

	if( Coords.length() < 4 || Coords.length() > 5 ) { return; }

	Coords = strlwr(Coords);

	int FromFile = Coords[ 0 ] - 'a';
	int FromRank = Coords[ 1 ] - '1';
	int ToFile = Coords[ 2 ] - 'a';
	int ToRank = Coords[ 3 ] - '1';

	if( FromFile >= 0 && FromFile < 8 && FromRank >= 0 && FromRank < 8 && ToRank >= 0 && ToRank < 8 && ToFile >= 0 && ToFile < 8 )
	{
		SetFrom( MakeSquare( FromFile, FromRank ) );
		SetTo( MakeSquare( ToFile, ToRank ) );
		SetPromote( Empty );

		if( Coords.length() > 4 )
		{
			if( Coords[ 4 ] == 'q' )
				SetPromote( Queen );
			else if( Coords[ 4 ] == 'r' )
				SetPromote( Rook );
			else if( Coords[ 4 ] == 'b' )
				SetPromote( Bishop );
			else if( Coords[ 4 ] == 'n' )
				SetPromote( Knight );
		}
	}
}

int move::GetPiece( char c, type& t )
{
	if( c == 'K' ) { t = King; return 1; }
	if( c == 'Q' ) { t = Queen; return 1; }
	if( c == 'R' ) { t = Rook; return 1; }
	if( c == 'B' ) { t = Bishop; return 1; }
	if( c == 'N' ) { t = Knight; return 1; }

	return 0;
}

move::move( string SAN, position* Position )
{
	SetToNull();

	// Strip off the garbage at the end of the string
	while( 1 )
	{
		char LastChar = SAN[ SAN.length() - 1 ];
		if( LastChar == '+' || LastChar == '#' || LastChar == '?' || LastChar == '!' )
			SAN = SAN.substr( 0, SAN.length() - 1 );
		else
			break;
	}

	if( SAN.length() < 2 )
		return;

	// If the move is a castling move, deal with it before we get into the main function
	if( SAN == "O-O" || SAN == "o-o" || SAN == "0-0" )
	{
		if( Position->Pieces[ Position->ToMove ][ King ] != Mask[ MySquare[ Position->ToMove ][ E1 ] ] )
			return;

		if( ( Position->Pieces[ Position->ToMove ][ Rook ] & Mask[ MySquare[ Position->ToMove ][ H1 ] ] ) == 0 )
			return;

		if( Position->BothPieces & ( Mask[ MySquare[ Position->ToMove ][ F1 ] ] | Mask[ MySquare[ Position->ToMove ][ G1 ] ] ) )
			return;

		SetFrom( MySquare[ Position->ToMove ][ E1 ] );
		SetTo( MySquare[ Position->ToMove ][ G1 ] );
		return;
	}
	else if( SAN == "O-O-O" || SAN == "o-o-o" || SAN == "0-0-0" )
	{
		if( Position->Pieces[ Position->ToMove ][ King ] != Mask[ MySquare[ Position->ToMove ][ E1 ] ] )
			return;

		if( ( Position->Pieces[ Position->ToMove ][ Rook ] & Mask[ MySquare[ Position->ToMove ][ A1 ] ] ) == 0 )
			return;

		if( Position->BothPieces & ( Mask[ MySquare[ Position->ToMove ][ B1 ] ] | Mask[ MySquare[ Position->ToMove ][ C1 ] ] | Mask[ MySquare[ Position->ToMove ][ D1 ] ] ) )
			return;

		SetFrom( MySquare[ Position->ToMove ][ E1 ] );
		SetTo( MySquare[ Position->ToMove ][ C1 ] );
		return;
	}

	// Grab the promotion piece, if present. This bit also removes an = sign if present.
	type PromotionPiece = Empty;
	char LastChar = SAN[ SAN.length() - 1 ];
	if( LastChar == 'Q' || LastChar == 'q' || LastChar == 'R' || LastChar == 'r' || LastChar == 'B' || LastChar == 'b' || LastChar == 'N' || LastChar == 'n' )
	{
		SAN = SAN.substr( 0, SAN.length() - 1 );
		if( SAN[ SAN.length() - 1 ] == '=' )
			SAN = SAN.substr( 0, SAN.length() - 1 );

		if( LastChar == 'Q' || LastChar == 'q' )
			PromotionPiece = Queen;
		else if( LastChar == 'R' || LastChar == 'r' )
			PromotionPiece = Rook;
		else if( LastChar == 'B' || LastChar == 'b' )
			PromotionPiece = Bishop;
		else if( LastChar == 'N' || LastChar == 'n' )
			PromotionPiece = Knight;
	}

	if( SAN.length() < 2 )
		return;

	// Grab the rank and file of the destination square
	int ToRank = SAN[ SAN.length() - 1 ] - '1';
	SAN = SAN.substr( 0, SAN.length() - 1 );

	int ToFile = SAN[ SAN.length() - 1 ];
	if( ToFile >= 'a' && ToFile <= 'h' )
		ToFile -= 'a';
	else if( ToFile >= 'A' && ToFile <= 'H' )
		ToFile -= 'A';
	SAN = SAN.substr( 0, SAN.length() - 1 );

	if( ToRank < 0 || ToRank >= 8 || ToFile < 0 || ToFile >= 8 )
		return;

	square To = 8 * ToRank + ToFile;

	// The move may or may not have the capture indicator, rank distinguisher, and file distinguisher
	bool IsCapture = false;
	if( SAN.length() > 0 && ( SAN[ SAN.length() - 1 ] == 'x' || SAN[ SAN.length() - 1 ] == 'X' ) )
	{
		IsCapture = true;
		SAN = SAN.substr( 0, SAN.length() - 1 );
	}

	int FromRank = -1;
	if( SAN.length() > 0 && SAN[ SAN.length() - 1 ] >= '0' && SAN[ SAN.length() - 1 ] <= '9' )
	{
		FromRank = SAN[ SAN.length() - 1 ] - '1';
		SAN = SAN.substr( 0, SAN.length() - 1 );
	}

	int FromFile = -1;
	if( SAN.length() > 0 && SAN[ SAN.length() - 1 ] >= 'a' && SAN[ SAN.length() - 1 ] <= 'h' )
	{
		FromFile = SAN[ SAN.length() - 1 ] - 'a';
		SAN = SAN.substr( 0, SAN.length() - 1 );
	}
	else if( SAN.length() > 0 && SAN[ SAN.length() - 1 ] >= 'A' && SAN[ SAN.length() - 1 ] <= 'H' && ( SAN[ SAN.length() - 1 ] != 'B' || SAN.length() != 1 ) )
	{
		FromFile = SAN[ SAN.length() - 1 ] - 'A';
		SAN = SAN.substr( 0, SAN.length() - 1 );
	}

	// Figure out what type of piece is doing the move
	type PieceType = Pawn;
	if( SAN.length() > 0 )
	{
		LastChar = SAN[ SAN.length() - 1 ];

		if( LastChar == 'K' || LastChar == 'k' || LastChar == 'Q' || LastChar == 'q' || LastChar == 'R' || LastChar == 'r' || LastChar == 'B' || LastChar == 'b' || LastChar == 'N' || LastChar == 'n' || LastChar == 'P' || LastChar == 'p' )
		{
			SAN = SAN.substr( 0, SAN.length() - 1 );

			if( LastChar == 'K' || LastChar == 'k' )
				PieceType = King;
			else if( LastChar == 'Q' || LastChar == 'q' )
				PieceType = Queen;
			else if( LastChar == 'R' || LastChar == 'r' )
				PieceType = Rook;
			else if( LastChar == 'B' || LastChar == 'b' )
				PieceType = Bishop;
			else if( LastChar == 'N' || LastChar == 'n' )
				PieceType = Knight;
		}
	}

	if( SAN.length() > 0 )
		return;

	// Information parsed! Now find the correct match.
	square From;
	if( PieceType == Pawn )
	{
		bitboard Movers = 0;

		if( !IsCapture )
		{
			if( FromFile != -1 || FromRank != -1 )
				return;

			if( Position->ToMove == White )
			{
				Movers = Mask[ To - 8 ];
				if( Rank( To ) == 3 && ( Position->BothPieces & Mask[ To - 8 ] ) == 0 )
					Movers |= Mask[ To - 16 ];
			}
			else
			{
				Movers = Mask[ To + 8 ];
				if( Rank( To ) == 4 && ( Position->BothPieces & Mask[ To + 8 ] ) == 0 )
					Movers |= Mask[ To + 16 ];
			}

			Movers &= Position->Pieces[ Position->ToMove ][ Pawn ];
			if( Population( Movers ) != 1 )
				return;

			From = LSBi( Movers );
		}
		else
		{
			if( FromFile == -1 || FromRank != -1 )
				return;

			Movers = moveGenerator::GetPawnAttacks( To, Position->Enemy );
			Movers &= FileMask[ FromFile ];
			Movers &= Position->Pieces[ Position->ToMove ][ Pawn ];

			Movers &= Position->Pieces[ Position->ToMove ][ Pawn ];
			if( Population( Movers ) != 1 )
				return;

			From = LSBi( Movers );
		}
	}
	else 
	{
		if( Promote() != Empty )
			return;

		bitboard Movers = 0;
		if( PieceType == Knight )
			Movers = moveGenerator::GetKnightMoves( Position, To, Position->ToMove );
		else if( PieceType == Bishop )
			Movers = moveGenerator::GetBishopMoves( Position, To, Position->ToMove );
		else if( PieceType == Rook )
			Movers = moveGenerator::GetRookMoves( Position, To, Position->ToMove );
		else if( PieceType == Queen )
			Movers = moveGenerator::GetQueenMoves( Position, To, Position->ToMove );
		else if( PieceType == King )
			Movers = moveGenerator::GetKingMoves( Position, To, Position->ToMove );

		Movers &= Position->Pieces[ Position->ToMove ][ PieceType ];
		if( FromFile != -1 ) Movers &= FileMask[ FromFile ];
		if( FromRank != -1 ) Movers &= RankMask[ FromRank ];

		if( Population( Movers ) > 1 )
		{
			bitboard Possiblities = Movers;
			while( Possiblities )
			{
				square PotentialFrom = LSBi( Possiblities );
				direction PinDirection = Position->GetPinDirection( Position->ToMove, PotentialFrom );
				if( PinDirection != 0 && PinDirection != Directions[ PotentialFrom ][ To ] && PinDirection != Directions[ To ][ PotentialFrom ] )
					Movers ^= Mask[ PotentialFrom ];
				Possiblities ^= Mask[ PotentialFrom ];
			}
		}

		if( Population( Movers ) != 1 )
			return;

		From = LSBi( Movers );
	}

	SetFrom( From );
	SetTo( To );
	SetPromote( PromotionPiece );
	return;
}

move::move( square f, square t, type p )
{
	Data = 0;
	SetFrom( f );
	SetTo( t );
	SetPromote( p );
}

string move::toString() const
{
	string r = SquareToString( From() ) + SquareToString( To() );
	switch( Promote() )
	{
		case Empty:
			return r;
		case Pawn:
			assert( "Cannot promote to a pawn!" && 0 );
		case Knight:
			return r + "=N";
		case Bishop:
			return r + "=B";
		case Rook:
			return r + "=R";
		case Queen:
			return r + "=Q";
		case King:
			assert( "Cannot promote to a king!" && 0 );
		default:
			assert( "Invalid promotion piece!" && 0 );
	}

	return "";
}

string move::toString( position* Position ) const
{
	string r = "";

	if( From() == NullSquare && To() == NullSquare )
		return "Pass";

	assert( From() != NullSquare && To() != NullSquare );
	type Type = Position->GetPieceType( From(), Position->ToMove );
	assert( Type != Empty );
	
	bool Checks = DoesMoveCheck( Position );
	bool Capture = ( Position->GetPieceColor( To() ) != Empty );
	if( Type == Pawn && To() == Position->EPSquare ) { Capture = true; }

	if( Type == King )
	{
		if( From() - To() == 2 ) return string( "O-O-O" ) + ( Checks ? "+" : "" );
		else if( To() - From() == 2 ) return string( "O-O" ) + ( Checks ? "+" : "" );
	}

	bool ShowFile = false;
	bool ShowRank = false;

	vector< square > OtherPieces = PiecesOfTypeThatCanMoveToSquare( Position, Type, To() );
	if( OtherPieces.size() > 1 )
	{
		bool ResolvedByFile = CanDistinguishByFile( &OtherPieces );
		bool ResolvedByRank = CanDistinguishByRank( &OtherPieces );

		if( ResolvedByFile )
			ShowFile = true;
		else if( ResolvedByRank )
			ShowRank = true;
		else
			ShowFile = ShowRank = true;
	}

	if( Type != Pawn ) r += PieceChars[ Type ];
	if( ( Type == Pawn && Capture ) || ShowFile ) r += 'a' + File( From() );
	if( ShowRank ) r += '1' + Rank( From() );
	if( Capture ) r += 'x';

	r += strlwr( SquareToString( To() ) );

	if( Type == Pawn && ( Rank( To() ) == 0 || Rank( To() ) == 7 ) )
		assert( Promote() != Empty );

	if( Promote() != Empty )
		r += string( "=" ) + PieceChars[ Promote() ];

	if( Checks )
		r += '+';

	return r;
}

bool move::operator== ( const move& o ) const
{
	return ( Data == o.Data );
	//if( From() == o.From() && To() == o.To() && Promote() == o.Promote() )
	//	return true;
	//return false;
}

bool move::operator!= ( const move& o ) const
{
	return ( Data != o.Data );
	//return !( this->operator ==( o ) );
}

string LineToString( line& Line )
{
	string r = "";
	for( unsigned int i = 0; i < Line.size(); i++ )
	{
		r += Line[ i ].toString();
		if( i != Line.size() - 1 ) r += " ";
	}

	return r;
}

string LineToString( vector< takeback >& Line )
{
	string r = "";
	for( unsigned int i = 0; i < Line.size(); i++ )
	{
		r += Line[ i ].toString();
		if( i != Line.size() - 1 ) r += " ";
	}

	return r;
}

string LineToString( line& Line, position* Position )
{
	zobrist Before = Position->Zobrist;
	string r = "";

	for( unsigned int i = 0; i < Line.size(); i++ )
	{
		/*if( Line[ i ] != NullMove )
		{
			bool IsLegal = Position->MoveIsLegal( Line[ i ] );
			if( !IsLegal )
			{
				cout << Position->toString() << "\n";
				cout << r << "\n";
				cout << Line[ i ].toString() << "\n";
				cout << i << "/" << Line.size() << "\n";
			}
			assert( IsLegal );
		}*/

		r += Line[ i ].toString( Position );
		if( i != Line.size() - 1 ) r += " ";
		if( Line[ i ] == NullMove )
			Position->MakeNullMove();
		else
			Position->MakeMove( Line[ i ] );
	}

	for( unsigned int i = 0; i < Line.size(); i++ )
		Position->TakeBack();

	if( Position->Zobrist != Before )
	{
		cout << LineToString( Line ) << "\n";
		assert( Position->Zobrist == Before );
	}

	return r;
}

string LineToString( vector< takeback >& Line, position* Position )
{
	string r = "";

	for( unsigned int i = 0; i < Line.size(); i++ )
	{
		r += Line[ i ].toString( Position );
		if( i != Line.size() - 1 ) r += " ";
		Position->MakeMove( Line[ i ] );
	}

	for( unsigned int i = 0; i < Line.size(); i++ )
		Position->TakeBack();

	return r;
}
