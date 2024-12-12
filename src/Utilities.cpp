#include "Utilities.h"

#ifdef NT_i386
#ifdef _WIN64
#  pragma intrinsic(_BitScanForward)
#  pragma intrinsic(_BitScanForward64)
#  pragma intrinsic(_BitScanReverse)
#  pragma intrinsic(_BitScanReverse64)
#endif
#else
#include <sys/time.h>
static inline unsigned char _BitScanForward64(unsigned long* Index, bitboard Mask)
{
	bitboard Ret;
	__asm__
	(
		"bsfq %[Mask], %[Ret]"
		:[Ret] "=r" (Ret)
		:[Mask] "mr" (Mask)
	);
	*Index = (unsigned long)Ret;
	return Mask ? 1 : 0;
}
static inline unsigned char _BitScanReverse64(unsigned long* Index, bitboard Mask)
{
	bitboard Ret;
	__asm__
	(
		"bsrq %[Mask], %[Ret]"
		:[Ret] "=r" (Ret)
		:[Mask] "mr" (Mask)
	);
	*Index = (unsigned long)Ret;
	return Mask ? 1 : 0;
}

unsigned int timeGetTime()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return 1000 * now.tv_sec + now.tv_usec / 1000;
}

void ZeroMemory(void* Destination, int Length)
{
	memset(Destination, 0, Length);
}

#endif

string itoa( int i )
{
	stringstream ss;
	ss << i;

	return ss.str();
}

string itoa( unsigned int i )
{
	stringstream ss;
	ss << i;

	return ss.str();
}

string itoa( bitboard i )
{
	stringstream ss;
	ss << i;

	return ss.str();
}

string strlwr( string s )
{
	string r = s;
	std::transform(r.begin(), r.end(), r.begin(), ::tolower);
	return r;
}

string Trim( string s )
{
	int Start = 0;
	for( ; Start < ( int )s.length(); Start++ )
		if( s[ Start ] != ' ' && s[ Start ] != '\t' && s[ Start ] != '\r' && s[ Start ] != '\n' )
			break;

	int End = ( int )s.length() - 1;
	for( ; End >= 0; End-- )
		if( s[ End ] != ' ' && s[ End ] != '\t' && s[ End ] != '\r' && s[ End ] != '\n' )
			break;

	return s.substr( Start, End - Start + 1 );
}

square LSBi( register bitboard a )
{
//#ifdef _WIN64
	register unsigned long r;
	if( _BitScanForward64( &r, a ) )
		return ( square )r;
	else
		return ( square )NullSquare;
/*#else
	unsigned long r;
	unsigned long* Longs = ( unsigned long* )&a;
	if( _BitScanForward( &r, Longs[ 0 ] ) )
		return r;
	else if( _BitScanForward( &r, Longs[ 1 ] ) )
		return r + 32;
	else
		return NullSquare;
#endif*/
}

square MSBi( register bitboard a )
{
//#ifdef _WIN64
register unsigned long r;
if( _BitScanReverse64( &r, a ) )
	return ( square )r;
else
	return NullSquare;
/*#else
	unsigned long r;
	unsigned long* Longs = ( unsigned long* )&a;
	if( _BitScanReverse( &r, Longs[ 1 ] ) )
		return r + 32;
	else if( _BitScanReverse( &r, Longs[ 0 ] ) )
		return r;
	else
		return NullSquare;
#endif*/
}

const int LSBi8[ 256 ] = { -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0 };
const int MSBi8[ 256 ] = { -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
int LSBi( byte b )
{
	return LSBi8[ b ];
}

int MSBi( byte b )
{
	return MSBi8[ b ];
}

int Population( bitboard b )
{
	register int r = 0;
	while( b )
	{
		b &= ( b - 1 );
		r++;
	}

	return r;
}

string SquareToString( square s )
{
	if( s == NullSquare )
		return "--";

	char rank = s / 8;
	char file = s % 8;

	file += 'a';
	rank += '1';

	string r = "";
	r += file;
	r += rank;

	return r;
}

string TypeToString( type t )
{
	if( t == Pawn ) return "Pawn";
	else if( t == Knight ) return "Knight";
	else if( t == Bishop ) return "Bishop";
	else if( t == Rook ) return "Rook";
	else if( t == Queen ) return "Queen";
	else if( t == King ) return "King";
	else if( t == Empty ) return "Empty";
	else { assert( 0 ); return "Unknown Type!"; }
}

string BitboardBinary( bitboard b )
{
	string r = "";
	for( int rank = 7; rank >= 0; rank-- )
	{
		for( int file = 0; file <= 7; file++ )
		{
			if( b & Mask[ MakeSquare( file, rank ) ] )
				r += "1";
			else
				r += "0";
		}
		r += "\n";
	}

	return r;
}

string CharBinary( char c )
{
	string r = "";
	for( int i = 7; i >= 0; i-- )
	{
		if( ( int )( c ) & ( 1 << i ) )
			r += "1";
		else
			r += "0";
	}

	return r;
}

bitboard GetIntermediateSquares( square Start, square End )
{
	direction Direction = Directions[ Start ][ End ];
	return Ray( Start, Direction ) & ~Mask[ Start ] & Ray( End, -Direction ) & ~Mask[ End ];
	//return Ray( Start + Direction, Direction ) ^ Ray( End, Direction );
}


square FlipSquare( square s )
{
	return MySquare[ Black ][ s ];
}

bool IsEmpty( bitboard b )
{
	return ( b == 0 );
}

square MostAdvanced( color c, bitboard b )
{
	if( c == White )
		return MSBi( b );
	else
		return LSBi( b );
}

bitboard Ray( square From, direction Direction )
{
	if( Direction == -9 ) return RayMinus9[ From ];
	if( Direction == -8 ) return RayMinus8[ From ];
	if( Direction == -7 ) return RayMinus7[ From ];
	if( Direction == -1 ) return RayMinus1[ From ];
	if( Direction == +1 ) return RayPlus1[ From ];
	if( Direction == +7 ) return RayPlus7[ From ];
	if( Direction == +8 ) return RayPlus8[ From ];
	if( Direction == +9 ) return RayPlus9[ From ];

	return 0;
}

bool IsCharDelim( char c, string Delims )
{
	for( unsigned i = 0; i < Delims.length(); i++ )
		if( c == Delims[ i ] ) return true;
	return false;
}

vector< string > Tokenize( string s, string Delims )
{
	vector< string > r;
	bool InQuotes = false;

	string Token = "";

	for( unsigned i = 0; i < s.length(); i++ )
	{
		if( s[ i ] == '\"' && ( i == 0 || s[ i - 1 ] != '\\' ) )
			InQuotes = !InQuotes;
		else if( !IsCharDelim( s[ i ], Delims ) || InQuotes )
			Token += s[ i ];
		else
		{
			if( Token.length() != 0 )
				r.push_back( Token );
			Token = "";
		}
	}

	if( Token.length() != 0 )
		r.push_back( Token );

	return r;
}

bitboard SquaresOfSameColor( square s )
{
	if( Mask[ s ] & LightSquareMask )
		return LightSquareMask;
	return DarkSquareMask;
}
