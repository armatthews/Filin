#include <cmath>
#include <stdlib.h>
#include "position.h"
#include "moveGenerator.h"
using std::abs;

position::position()
{
	BothPieces = 0;

	AllPieces[ White ] = AllPieces[ Black ] = 0;
	for( type t = Pawn; t <= King; t++ )
		Pieces[ White ][ t ] = Pieces[ Black ][ t ] = 0;

	EPSquare = NullSquare;
	Castling = 0;
	Fifty = 0;
	MoveNumber = 0;

	ToMove = White;
	Enemy = Black;

	Zobrist = 0;
	PawnHash = 0;
}

position::position( const position& o )
{
	SetUpPosition( o.FEN() );
 	PastPositions = o.PastPositions;
	MoveHistory = o.MoveHistory;
	assert( MoveHistory.size() == PastPositions.size() );
}

void position::operator=( const position& o )
{
	SetUpPosition( o.FEN() );
	PastPositions = o.PastPositions;
	MoveHistory = o.MoveHistory;
	assert( MoveHistory.size() == PastPositions.size() );
}

bool position::SetUpPosition( string FEN )
{
	Zobrist = 0;
	BothPieces = 0;
	for( color c = White; c <= Black; c++ )
	{
		AllPieces[ c ] = 0;

		for( type t = Pawn; t <= King; t++ )
			Pieces[ c ][ t ] = 0;
	}

	unsigned i = 0;

	for( int BoardSquare = 0; BoardSquare < 64; BoardSquare++ )
	{
		square s = MakeSquare( File( BoardSquare ), 7 - Rank( BoardSquare ) );
		if( i >= FEN.length() )
			return false;

		switch( FEN[ i ] )
		{
		case 'K':
			AddPiece( White, King, s );
			break;
		case 'Q':
			AddPiece( White, Queen, s );
			break;
		case 'R':
			AddPiece( White, Rook, s );
			break;
		case 'B':
			AddPiece( White, Bishop, s );
			break;
		case 'N':
			AddPiece( White, Knight, s );
			break;
		case 'P':
			AddPiece( White, Pawn, s );
			break;

		case 'k':
			AddPiece( Black, King, s );
			break;
		case 'q':
			AddPiece( Black, Queen, s );
			break;
		case 'r':
			AddPiece( Black, Rook, s );
			break;
		case 'b':
			AddPiece( Black, Bishop, s );
			break;
		case 'n':
			AddPiece( Black, Knight, s );
			break;
		case 'p':
			AddPiece( Black, Pawn, s );
			break;

		case '1':
			BoardSquare += 0;
			break;
		case '2':
			BoardSquare += 1;
			break;
		case '3':
			BoardSquare += 2;
			break;
		case '4':
			BoardSquare += 3;
			break;
		case '5':
			BoardSquare += 4;
			break;
		case '6':
			BoardSquare += 5;
			break;
		case '7':
			BoardSquare += 6;
			break;
		case '8':
			BoardSquare += 7;
			break;
		default:
			return false;
			break;
		}

		i++;

		if( BoardSquare % 8 == 7 && BoardSquare != 63 )
		{
			if( i >= FEN.length() || FEN[ i ] != '/' )
				return false;
			i++;
		}
	}

	if( i >= FEN.length() || FEN[ i ] != ' ' )
		return false;

	i++;

	if( i >= FEN.length() )
		return false;
	else if( FEN[ i ] == 'w' )
	{
		ToMove = White;
		Enemy = Black;
	}
	else if( FEN[ i ] == 'b' )
	{
		Enemy = White;
		ToMove = Black;
		Zobrist ^= ZK::BlackToMove;
	}
	else
		return false;

	i++;

	if( i >= FEN.length() || FEN[ i ] != ' ' )
		return false;
	i++;

	if( i >= FEN.length() )
		return false;

	if( FEN[ i ] != '-' )
	{
		Castling = 0;
		while( FEN[ i ] != ' ' )
		{
			if( FEN[ i ] == 'K' )
				Castling |= WhiteKingSideAble;
			if( FEN[ i ] == 'Q' )
				Castling |= WhiteQueenSideAble;
			if( FEN[ i ] == 'k' )
				Castling |= BlackKingSideAble;
			if( FEN[ i ] == 'q' )
				Castling |= BlackQueenSideAble;

			i++;
		}
	}
	else
	{
		Castling = 0;
		i++;
	}

	Zobrist ^= ZK::Castling[ Castling & 15 ];

	if( i >= FEN.length() || FEN[ i ] != ' ' )
	{
		EPSquare = NullSquare;
		return true;
	}
	i++;

	if( i >= FEN.length() )
	{
		EPSquare = NullSquare;
		return true;
	}

	if( FEN[ i ] != '-' )
	{
		char file = FEN[ i++ ];
		char rank = FEN[ i++ ];

		if( rank != '3' && rank != '6' )
			return false;

		if( ( file < 'a' || file > 'h' ) && ( file < 'A' && file > 'H' ) )
			return false;

		if( file >= 'a' ) file -= 'a';
		else file -= 'A';

		rank -= '1';

		EPSquare = rank * 8 + file;
	}
	else
	{
		EPSquare = NullSquare;
		i++;
	}

	if( EPSquare != NullSquare )
		Zobrist ^= ZK::EPKeys[ EPSquare ];

	if( i >= FEN.length() || FEN[ i ] != ' ' )
	{
		Fifty = 0;
		return i;
	}
	i++;

	char Buffer[ 8 ] = { 0 };
	while( i < FEN.length() && FEN[ i ] != ' ' )
	{
		size_t Next = strlen( Buffer );
		if( Next >= 7 )
			return false;
		Buffer[ Next ] = FEN[ i ];
		Buffer[ Next + 1 ] = 0;
		i++;
	}
	i++;

	Fifty = atoi( Buffer );

	Buffer[ 0 ] = 0;
	while( i < FEN.length() && FEN[ i ] != ' ' )
	{
		size_t Next = strlen( Buffer );
		if( Next >= 7 )
			return false;
		Buffer[ Next ] = FEN[ i ];
		Buffer[ Next + 1 ] = 0;
		i++;
	}
	i++;

	MoveNumber = atoi( Buffer );

	return true;
}

string position::toString() const
{
	char board[64];
	const char pieceChars[] = { 'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k' };
	for( int i = 0; i < 64; i++ )
	{
		if( ( BothPieces & Mask[ i ] ) == 0 )
			board[i] = ' ';
		else
		{
			color Side = Empty;

			if( AllPieces[ White ] & Mask[ i ] )
				Side = White;

			if( AllPieces[ Black ] & Mask[ i ] )
				Side = Black;

			assert( Side != Empty );

			for( int j = 0; j < 6; j++ )
			{
				if( ( Pieces[ Side ][ j ] & Mask[ i ] ) != 0 )
					board[ i ] = pieceChars[ j + 6 * ( ( Side == Black ) ? 1 : 0 ) ];
			}
		}
	}

	string r = "";

	r += "   A B C D E F G H\n";
	for( int rank = 7; rank >= 0; rank-- )
	{
		r += itoa( rank + 1 ) + " |";
		for( int file = 0; file <= 7; file++ )
		{
			r += board[ MakeSquare(file, rank ) ];
			r += '|';
		}
		r += " " + itoa( rank + 1 ) + "\n";
	}
	r += "   A B C D E F G H\n";
	r += ColorToString( ToMove ) + " to move\tCastling: ";
	if( Castling & WhiteKingSideAble ) r += "K";
	if( Castling & WhiteQueenSideAble ) r += "Q";
	if( Castling & BlackKingSideAble ) r += "k";
	if( Castling & BlackQueenSideAble ) r += "q";
	if( Castling == 0 ) r += "-";

	r += "\n";
	r += "EP: " + SquareToString( EPSquare );
	r += "   Fifty: " + itoa( Fifty );
	r += "\n";
	r += "Move #";
	r += itoa( MoveNumber );

	return r;
}

string position::FEN() const
{
	string r = "";
	square s;
	color c;
	type t;

	int Counter = 0;

	for( int Rank = 7; Rank >= 0; Rank-- )
	{
		for( int File = 0; File < 8; File++ )
		{
			s = MakeSquare( File, Rank );

			c = GetPieceColor( s );

			if( c != Empty )
			{
				if( Counter != 0 )
					r += itoa( Counter );
				Counter = 0;

				t = GetPieceType( s, c );

				if( c == White && t == Pawn ) r += "P";
				if( c == White && t == Knight ) r += "N";
				if( c == White && t == Bishop ) r += "B";
				if( c == White && t == Rook ) r += "R";
				if( c == White && t == Queen ) r += "Q";
				if( c == White && t == King ) r += "K";

				if( c == Black && t == Pawn ) r += "p";
				if( c == Black && t == Knight ) r += "n";
				if( c == Black && t == Bishop ) r += "b";
				if( c == Black && t == Rook ) r += "r";
				if( c == Black && t == Queen ) r += "q";
				if( c == Black && t == King ) r += "k";
			}
			else
				Counter++;
		}
		if( Counter != 0 )
			r += itoa( Counter );
		Counter = 0;

		if( Rank != 0 )
			r += "/";
	}

	if( Counter != 0 )
		r += itoa( Counter );
	Counter = 0;

	r += " ";

	if( ToMove == White ) r += "w ";
	else r += "b ";

	if( Castling == 0 ) r += "-";
	else
	{
		if( Castling & WhiteKingSideAble ) r += "K";
		if( Castling & WhiteQueenSideAble ) r += "Q";
		if( Castling & BlackKingSideAble ) r += "k";
		if( Castling & BlackQueenSideAble ) r += "q";
	}
	r += " ";

	if( EPSquare != NullSquare )
		r += SquareToString( EPSquare );
	else
		r += "-";
	r += " ";

	r += itoa( Fifty ) + " ";

	r += itoa( MoveNumber );

	return r;
}

void position::AddPiece( color c, type t, square s )
{
	assert( s != NullSquare && c != Empty && t != Empty );
	BothPieces |= Mask[ s ];
	AllPieces[ c ] |= Mask[ s ];

	Pieces[ c ][ t ] |= Mask[ s ];

	Zobrist ^= ZK::Keys[ c ][ t ][ s ];
	if( t == Pawn )
		PawnHash ^= ZK::Keys[ c ][ t ][ s ];
}

void position::RemovePiece( color c, type t, square s )
{
	assert( s != NullSquare && c != Empty && t != Empty );
	BothPieces &= ~Mask[ s ];
	AllPieces[ c ] &= ~Mask[ s ];

	Pieces[ c ][ t ] &= ~Mask[ s ];

	Zobrist ^= ZK::Keys[ c ][ t ][ s ];
	if( t == Pawn )
		PawnHash ^= ZK::Keys[ c ][ t ][ s ];
}

void position::RemovePiece( square s )
{
	assert( s != NullSquare );
	color c = GetPieceColor( s );
	if( c != Empty )
		RemovePiece( c, GetPieceType( s, c ), s );
}

color position::GetPieceColor( square s ) const
{
	assert( s != NullSquare );

	if( ( BothPieces & Mask[ s ] ) == 0 )
		return Empty;

	if( AllPieces[ White ] & Mask[ s ] )
	{
		assert( ( AllPieces[ Black ] & Mask[ s ] ) == 0 );
		return White;
	}

	if( AllPieces[ Black ] & Mask[ s ] )
	{
		assert( ( AllPieces[ White ] & Mask[ s ] ) == 0 );
		return Black;
	}

	return Empty;
}

type position::GetPieceType( square s, color c ) const
{
	assert( s != NullSquare && c != Empty );

	for( type i = Pawn; i <= King; i++ )
		if( ( Pieces[ c ][ i ] & Mask[ s ] ) != 0 )
			return i;

	return Empty;
}

void position::MakeMove( const move Move )
{
	if( Move == NullMove )
	{
		MakeNullMove();
		return;
	}

	if( MoveHistory.size() != PastPositions.size() )
		assert( MoveHistory.size() == PastPositions.size() );
	PastPositions.push_back( Zobrist );
	MoveHistory.push_back( takeback() );
	assert( PastPositions.size() == MoveHistory.size() );
	takeback& Top = MoveHistory.back();

	Top.SetFrom( Move.From() );
	Top.SetTo( Move.To() );

	color toColor = GetPieceColor( Move.To() );
	type toType = Empty;
	if( toColor != Empty )
	{
		toType = GetPieceType( Move.To(), toColor );
		if( toType == King )
		{
			Log( ColorToString( ToMove ) + " captures a king!\n" + toString() + "\n" + Move.toString() + "\n" );
			DumpHistory();
		}
		assert( toType != King );
		RemovePiece( toColor, toType, Move.To() );
		Top.Captured = toType;
	}
	else
		Top.Captured = Empty;

	type t = GetPieceType( Move.From(), ToMove );
	if( t == Empty )
	{
		cout << this->toString() << "\n";
		cout << Move.toString() << "\n";
	}
	if( t == Empty )
	{
		cout << toString() << "\n" << FEN() << "\n";
		cout << SquareToString( Move.From() ) << " does not have a " << ColorToString( ToMove ) << " piece on it! (To=" << SquareToString( Move.To() ) << ")\n";
		assert( t != Empty );
	}
	RemovePiece( ToMove, t, Move.From() );
	AddPiece( ToMove, t, Move.To() );

	if( t == King && Move.To() - Move.From() == 2 )
	{
		if( GetPieceType( Move.From() + 3, ToMove ) != Rook )
		{
			cout << this->toString() << "\n";
			cout << SquareToString( Move.From() ) << "\n";
			cout << (int) Castling << "\n";
			cout << WhiteKingSideAble << ", " << WhiteQueenSideAble << ", " << BlackKingSideAble << ", " << BlackQueenSideAble << "\n";

			while( MoveHistory.size() > 0 )
			{
				cout << MoveHistory.back().toString() << " ";
				MoveHistory.pop_back();
			}

			cout << "\n";
		}

		assert( File( Move.From() ) == 4 );
		assert( Castling & KingSideAble[ ToMove ] );
		assert( GetPieceColor( Move.From() + 3 ) == ToMove );
		assert( GetPieceType( Move.From() + 3, ToMove ) == Rook );

		RemovePiece( ToMove, Rook, Move.From() + 3 );
		AddPiece( ToMove, Rook, Move.From() + 1 );
	}

	if( t == King && Move.From() - Move.To() == 2 )
	{
		assert( File( Move.From() ) == 4 );
		assert( Castling & QueenSideAble[ ToMove ] );
		assert( GetPieceColor( Move.From() - 4 ) == ToMove );
		assert( GetPieceType( Move.From() - 4, ToMove ) == Rook );

		RemovePiece( ToMove, Rook, Move.From() - 4 );
		AddPiece( ToMove, Rook, Move.From() - 1 );
	}

	if( t == Pawn && ( Rank( Move.To() ) == 7 || Rank( Move.To() ) == 0 ) )
	{
		assert( Move.Promote() != Empty );

		RemovePiece( ToMove, Pawn, Move.To() );
		AddPiece( ToMove, Move.Promote(), Move.To() );
	}

	Top.SetPromote( Move.Promote() );
	Top.EPSquare = EPSquare;
	Top.Fifty = Fifty;
	Top.Castling = Castling;

	if( t == Pawn && Move.To() == EPSquare )
		RemovePiece( MakeSquare( File( EPSquare ), Rank( Move.From() ) ) );

	if( EPSquare != NullSquare )
		Zobrist ^= ZK::EPKeys[ EPSquare ];

	if( t == Pawn && abs( Move.From() - Move.To() ) == 16 )
		EPSquare = ( Move.From() + Move.To() ) / 2;
	else
		EPSquare = NullSquare;

	if( EPSquare != NullSquare )
		Zobrist ^= ZK::EPKeys[ EPSquare ];

	Zobrist ^= ZK::Castling[ Castling & 15 ];
	Castling &= CastlingMask[ Move.To() ];
	Castling &= CastlingMask[ Move.From() ];
	Zobrist ^= ZK::Castling[ Castling & 15 ];

	ToMove = Enemy;
	Enemy = ( ToMove == White ) ? Black : White;
	Zobrist ^= ZK::BlackToMove;	

	if( t == Pawn || toColor != Empty ) // TODO: Doesn't reset on EP
		Fifty = 0;
	else
		Fifty++;

	if( Move.From() == E1 || Move.To() == E1 )
	{
		assert( ( Castling & WhiteKingSideAble ) == 0 );
		assert( ( Castling & WhiteQueenSideAble ) == 0 );
	}

	if( ToMove == White )
		MoveNumber++;
}

void position::TakeBack()
{
	assert( PastPositions.size() == MoveHistory.size() );
	if( PastPositions.size() == 0 )
	{
		assert( MoveHistory.size() == 0 );
		assert( 0 );
		return;
	}
	PastPositions.pop_back();
	takeback& Top = MoveHistory.back();

	if( Top.To() != NullSquare || Top.From() != NullSquare )
	{
		assert( GetPieceColor( Top.To() ) == Enemy );
		type t = GetPieceType( Top.To(), Enemy );

		RemovePiece( Enemy, t, Top.To() );

		if( Top.Captured != Empty )
			AddPiece( ToMove, Top.Captured, Top.To() );

		if( Top.Promote() == Empty )
			AddPiece( Enemy, t, Top.From() );
		else
			AddPiece( Enemy, Pawn, Top.From() );

		if( t == King && ( Top.To() - Top.From() ) == 2 )
		{
			RemovePiece( Enemy, Rook, Top.From() + 1 );
			AddPiece( Enemy, Rook, Top.From() + 3 );
		}

		if( t == King && ( Top.From() - Top.To() ) == 2 )
		{
			RemovePiece( Enemy, Rook, Top.From() - 1 );
			AddPiece( Enemy, Rook, Top.From() - 4 );
		}

		if( t == Pawn && Top.To() == Top.EPSquare )
			AddPiece( ToMove, Pawn, MakeSquare( File( Top.To() ), Rank( Top.From() ) ) );
	}

	if( ToMove == White )
		MoveNumber--;

	ToMove = Enemy;
	Enemy = ( ToMove == White ) ? Black : White;
	Zobrist ^= ZK::BlackToMove;

	Fifty = Top.Fifty;

	Zobrist ^= ZK::Castling[ Castling & 15 ];
	Castling = Top.Castling;
	Zobrist ^= ZK::Castling[ Castling & 15 ];

	if( EPSquare != NullSquare )
		Zobrist ^= ZK::EPKeys[ EPSquare ];
	EPSquare = Top.EPSquare;
	if( EPSquare != NullSquare )
		Zobrist ^= ZK::EPKeys[ EPSquare ];

	MoveHistory.pop_back();
}

void position::MakeNullMove()
{
	PastPositions.push_back( Zobrist );
	MoveHistory.push_back( takeback() );
	assert( PastPositions.size() == MoveHistory.size() );

	takeback& Top = MoveHistory.back();
	Top.SetFrom( NullSquare );
	Top.SetTo( NullSquare );
	Top.SetPromote( Empty );
	Top.Castling = Castling;
	Top.EPSquare = EPSquare;
	Top.Captured = Empty;
	Top.Fifty = Fifty;

	ToMove = Enemy;
	Enemy = ( ToMove == White ) ? Black : White;
	Zobrist ^= ZK::BlackToMove;

	if( EPSquare != NullSquare )
		Zobrist ^= ZK::EPKeys[ EPSquare ];
	EPSquare = NullSquare;

	if( ToMove == White )
		MoveNumber++;

	Fifty = 0;
}

bool position::InCheck( color c )
{
	assert( Pieces[ c ][ King ] != 0 );

	return IsSquareAttackedByColor( LSBi( Pieces[ c ][ King ] ), ( c == White ) ? Black : White );
}

bitboard position::AttacksTo( square s, color c )
{
	color Enemy = ( c == White ) ? Black : White;

	bitboard RookMoves = moveGenerator::GetRookMoves( this, s, Enemy );
	bitboard BishopMoves = moveGenerator::GetBishopMoves( this, s, Enemy );
	bitboard KnightMoves = moveGenerator::GetKnightMoves( this, s, Enemy );
	bitboard KingMoves = moveGenerator::GetKingMoves( this, s, Enemy );
	bitboard PawnMoves = moveGenerator::GetPawnAttacks( s, Enemy );

	bitboard r = 0;
	r |= BishopMoves & ( Pieces[ c ][ Queen ] | Pieces[ c ][ Bishop ] );
	r |= RookMoves & ( Pieces[ c ][ Queen ] | Pieces[ c ][ Rook ] );
	r |= KnightMoves & Pieces[ c ][ Knight ];
	r |= KingMoves & Pieces[ c ][ King ];
	r |= PawnMoves & Pieces[ c ][ Pawn ];

	return r;
}

bool position::IsSquareAttackedByColor( square s, color c ) const
{
	assert( A1 <= s && s <= H8 );
	assert( c == White || c == Black );

	bitboard BishopMoves = moveGenerator::GetBishopMoves( this, s, ( c == White ) ? Black : White );
	if( BishopMoves & ( Pieces[ c ][ Queen ] | Pieces[ c ][ Bishop ] ) )
		return true;

	bitboard RookMoves = moveGenerator::GetRookMoves( this, s, ( c == White ) ? Black : White );
	if( RookMoves & ( Pieces[ c ][ Queen ] | Pieces[ c ][ Rook ] ) )
		return true;

	bitboard KnightMoves = moveGenerator::GetKnightMoves( this, s, ( c == White ) ? Black : White );
	if( KnightMoves & Pieces[ c ][ Knight ] )
		return true;

	bitboard PawnMoves = moveGenerator::GetPawnAttacks( s, ( c == White ) ? Black : White );
	if( PawnMoves & Pieces[ c ][ Pawn ] )
		return true;

	bitboard KingMoves = moveGenerator::GetKingMoves( this, s, ( c == White ) ? Black : White );
	if( KingMoves & Pieces[ c ][ King ] )
		return true;

	return false;
}

void position::Reverse( position& Output )
{
	Output = *this;

	if( this->EPSquare != NullSquare )
		Output.EPSquare = FlipSquare( this->EPSquare );

	Output.Castling = 0;
	if( this->Castling & WhiteKingSideAble ) Output.Castling |= BlackKingSideAble;
	if( this->Castling & WhiteQueenSideAble ) Output.Castling |= BlackQueenSideAble;
	if( this->Castling & BlackKingSideAble ) Output.Castling |= WhiteKingSideAble;
	if( this->Castling & BlackQueenSideAble ) Output.Castling |= WhiteQueenSideAble;

	Output.Enemy = this->ToMove;
	Output.ToMove = this->Enemy;

	Output.MoveHistory.clear();
	Output.PastPositions.clear();

	Output.Zobrist ^= ZK::BlackToMove;

	for( square s = A1; s <= H8; s++ )
		Output.RemovePiece( s );

	for( square s = A1; s <= H8; s++ )
	{
		if( this->BothPieces & Mask[ s ] )
		{
			color Color = GetPieceColor( s );
			type Type = GetPieceType( s, Color );
			Output.AddPiece( ( Color == White ) ? Black : White, Type, FlipSquare( s ) );
		}
	}
}

bool position::MoveIsLegal( const move Move ) const
{
	if( Move == NullMove )
		return false;

	if( IsEmpty( AllPieces[ ToMove ] & Mask[ Move.From() ] ) )
		return false;

	bitboard PinnedPieces = moveGenerator::GetPinnedPieces( ( position* )this, ToMove );
	type Mover = GetPieceType( Move.From(), ToMove );
	bool IsPinned = Mask[ Move.From() ] & PinnedPieces; 

	bitboard LegalMoves = moveGenerator::GetLegalMoves( this, Move.From(), Mover, ToMove, IsPinned  );
	if( IsEmpty( LegalMoves & Mask[ Move.To() ] ) )
		return false;

	if( Mover == Pawn )
	{
		int rank = Rank( Move.To() );
		if( rank == 0 || rank == 7 )
			if( Move.Promote() == Empty )
				return false;
	}

	return true;
}

bool position::LineIsLegal( const line& Line )
{
	line NextLine;
	NextLine.assign( Line.begin() + 1, Line.end() );

	if( !MoveIsLegal( Line[ 0 ] ) )
		return false;

	else if( NextLine.size() > 0 )
	{
		MakeMove( Line[ 0 ] );
		bool r = LineIsLegal( NextLine );
		TakeBack();
		return r;
	}
	else
		return true;
}

bool position::IsDraw( int Repetitions ) const
{
	if( Fifty >= 100 )
		return true;

	int Found = 1;
	int Max = min( ( int )PastPositions.size(), Fifty );

	for( int i = 1; i < Max; i += 2 )
	{
		if( PastPositions[ PastPositions.size() - 1 - i ] == Zobrist )
			if( ++Found == Repetitions )
				return true;
	}
	return false;
}

direction position::GetPinDirection( color ToMove, square Square ) const
{
	square KingSquare = LSBi( Pieces[ ToMove ][ King ] );
	int Direction = Directions[ Square ][ KingSquare ];

	if( Direction == 0 )
		return 0;

	bitboard AttackedSquares, EnemyPieces;
	if( Direction == 1 || Direction == -1 )
	{
		EnemyPieces = Pieces[ Enemy ][ Queen ] | Pieces[ Enemy ][ Rook ];
		AttackedSquares = moveGenerator::GetRookMoves( Square, BothPieces ) & RankMask[ Rank( Square ) ];
		if( AttackedSquares & Pieces[ ToMove ][ King ] )
			if( AttackedSquares & EnemyPieces )
				return 1;
	}
	else if( Direction == 8 || Direction == -8 )
	{
		EnemyPieces = Pieces[ Enemy ][ Queen ] | Pieces[ Enemy ][ Rook ];
		AttackedSquares = moveGenerator::GetRookMoves( Square, BothPieces ) & FileMask[ File( Square ) ];
		if( AttackedSquares & Pieces[ ToMove ][ King ] )
			if( AttackedSquares & EnemyPieces )
				return 8;
	}
	else if( Direction == 7 || Direction == -7 )
	{
		EnemyPieces = Pieces[ Enemy ][ Queen ] | Pieces[ Enemy ][ Bishop ];
		AttackedSquares = moveGenerator::GetBishopMoves( Square, BothPieces ) & ( RayPlus7[ Square ] | RayMinus7[ Square ] );
		if( AttackedSquares & Pieces[ ToMove ][ King ] )
			if( AttackedSquares & EnemyPieces )
				return 7;
	}
	else if( Direction == 9 || Direction == -9 )
	{
		EnemyPieces = Pieces[ Enemy ][ Queen ] | Pieces[ Enemy ][ Bishop ];
		AttackedSquares = moveGenerator::GetBishopMoves( Square, BothPieces ) & ( RayPlus9[ Square ] | RayMinus9[ Square ] );
		if( AttackedSquares & Pieces[ ToMove ][ King ] )
			if( AttackedSquares & EnemyPieces )
				return 9;
	}

	return 0;
}

void position::DumpHistory()
{
	while( MoveHistory.size() )
	{
		Log( MoveHistory.back().toString() + "\n" );
		MoveHistory.pop_back();
	}
	assert( 0 );
}
