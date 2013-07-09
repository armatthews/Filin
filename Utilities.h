#include <string>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#ifdef NT_i386
  #include <intrin.h>
  #include <windows.h>
#endif
#include "Constants.h"
#include "Log.h"
#undef assert
#ifdef _DEBUG
#define assert( _Expression ) if( !( _Expression ) ) { std::string s = string( "Assertion failed in " ) + __FILE__ + string( " on line " ) + itoa( __LINE__ ) + ".\n"; Log( s ); cout << s; _wassert( _CRT_WIDE( #_Expression ), _CRT_WIDE( __FILE__ ), __LINE__ ); }
#else
#define assert( _Expression )
#endif

using std::string;
using std::stringstream;
using std::cout;
using std::cin;
using std::vector;
using std::map;
using std::fstream;
using std::ifstream;
using std::ofstream;


#ifndef max
	template <class T>
	T max( T a, T b )
	{
		if( a > b )
			return a;
		else
			return b;
	}
#endif

#ifndef min
	template <class T>
	T min( T a, T b )
	{
		if( a < b )
			return a;
		else
			return b;
	}
#endif

#define Swap( a, b ) if( a != b ) { (a) ^= (b); (b) ^= (a); (a) ^= (b); }
#define FlipColor( c ) ( ( c == White ) ? Black : White )

#define RankDistance( a, b ) abs( Rank( ( a ) ) - Rank( ( b ) ) )
#define FileDistance( a, b ) abs( File( ( a ) ) - File( ( b ) ) )
#define Distance( a, b ) max( RankDistance( ( a ), ( b ) ), FileDistance( ( a ), ( b ) ) )

#define Rank( s ) ( ( s ) / 8 )
#define File( s ) ( ( s ) % 8 )

#define ColorToString( c ) ( ( std::string )( ( ( c ) == White ) ? "White" : "Black" ) )
#define MakeSquare( f, r ) ( ( f ) + 8 * ( r ) )

std::string itoa( const int i );
std::string itoa( const unsigned int i );
std::string itoa( const bitboard i );
std::string strlwr( std::string s );
std::string Trim( std::string s );
square LSBi( bitboard a );
square MSBi( bitboard a );
int LSBi( byte b );
int MSBi( byte b );
int Population( bitboard b );

std::string SquareToString( square s );
std::string TypeToString( type t );
std::string BitboardBinary( bitboard b );
std::string CharBinary( char b );

bool IsCharDelim( char c, std::string Delims );
std::vector< std::string > Tokenize( std::string s, std::string Delims );

std::string DirectionToString( direction d );

square FlipSquare( square s );
bool IsEmpty( bitboard b );
square MostAdvanced( color c, bitboard b );
bitboard Ray( square From, direction Direction );
bitboard SquaresOfSameColor( square s );
bitboard GetIntermediateSquares( square Start, square End );

#ifndef NT_i386
void ZeroMemory(void* destination, int length);
unsigned int timeGetTime();
unsigned int timeBeginPeriod(unsigned int);
unsigned int timeEndPeriod(unsigned int);
#endif
#pragma once
