#include <iostream>
#include "TranspositionTable.h"
#include "position.h"
#include "searcher.h"
using std::cout;

element::element()
{
	Reset();
}

void element::Reset()
{
	this->Key = 0;

	this->BestMove = NullMove;

	this->Type = NONE;

	this->Value = 0;
	this->Depth = 0;
}

transpositionTable::transpositionTable()
{
	Generation = 0;
	Data = NULL;
}

transpositionTable::transpositionTable( unsigned int Size )
{
	Generation = 0;
	Data = NULL;
	Resize( Size );
}

transpositionTable::~transpositionTable()
{
	if(Data != NULL)
		delete[] Data;
	Data = NULL;
}

transpositionTable::transpositionTable( const transpositionTable& o )
{
	Generation = o.Generation;
	NumberOfEntries = o.NumberOfEntries;
	IndexMask = o.IndexMask;

	assert( NumberOfEntries > 0 );
	Data = new element[ 3 * NumberOfEntries ];
	memcpy( Data, o.Data, 3 * NumberOfEntries * sizeof( element ) );
}

bool transpositionTable::operator=( const transpositionTable& o )
{
	assert(o.NumberOfEntries > 0);
	Generation = o.Generation;
	NumberOfEntries = o.NumberOfEntries;
	IndexMask = o.IndexMask;

	if( Data != NULL )
		delete[] Data;
	Data = new element[ 3 * NumberOfEntries ];
	memcpy( Data, o.Data, 3 * NumberOfEntries * sizeof( element ) );

	return true;
}

void transpositionTable::Resize( unsigned int Size )
{
	if( Data != NULL )
		delete[] Data;

	int NumberOfEntries = 1;
	for( ; NumberOfEntries * sizeof( element ) * 3 < Size * 1024 * 1024; NumberOfEntries *= 2 );

	NumberOfEntries /= 2;
	if( NumberOfEntries < 1 ) NumberOfEntries = 1;

	assert( NumberOfEntries > 0);
	Data = new element[ 3 * NumberOfEntries ];
	this->NumberOfEntries = NumberOfEntries;
	this->IndexMask = this->NumberOfEntries - 1;

	Clear();
}

void transpositionTable::NextGeneration()
{
	Generation++;
}

void transpositionTable::Clear()
{
	for( unsigned int i = 0; i < 3 * this->NumberOfEntries; i++ )
		Data[ i ].Reset();
}

move transpositionTable::BestMove( zobrist Zobrist ) const
{
	element* Element;

	for( int i = 0; i < 3; i++ )
	{
		Element = &Data[ 3 * ( Zobrist & IndexMask ) + i ];
		if( Element->Key == Zobrist && Element->Type != NONE )
			return Element->BestMove;
	}

	return NullMove;
}

BoundType transpositionTable::Probe( zobrist Zobrist, int DistanceFromRoot, int DepthRemaining, int& Alpha, int Beta ) const
{	
	element* Element = NULL;
	for( int i = 0; i < 3; i++ )
	{
		Element = &Data[ 3 * ( Zobrist & IndexMask ) + i ];
		if( Element->Key == Zobrist )
			break;
	}

	if( DepthRemaining > Element->Depth || Element->Key != Zobrist )
		return NONE;

	int Value = Element->Value;
	if( Value > MATESCORE - 300 ) Value -= DistanceFromRoot - 1;
	if( Value < -MATESCORE + 300 ) Value += DistanceFromRoot - 1;

	if( Element->Type == EXACT )
	{
		Alpha = Value;
		return EXACT;
	}

	if( Element->Type == LOWER && Value > Alpha )
	{
		Alpha = Value;
		if( Value >= Beta )
		{
			Alpha = Beta;
			return LOWER;
		}
	}

	if( Element->Type == UPPER && Value <= Alpha )
	{
		return UPPER;
	}

	return NONE;
}

void transpositionTable::Store( zobrist Zobrist, int DistanceFromRoot, int DepthRemaining, BoundType Type, int Value, move Move )
{
	element* Replace = &Data[ 3 * ( Zobrist & IndexMask ) ];

	for( int i = 0; i < 3; i++ )
	{
		element* Element = &Data[ 3 * ( Zobrist & IndexMask )  + i ];
		if( Element->Key == Zobrist )
		{
			if( Element->Depth > DepthRemaining && Element->Type == EXACT )
				return;

			if( Move == NullMove )
				Move = Element->BestMove;
			Replace = Element;
			break;
		}
		else if( Replace->Generation == Generation )
		{
			if( Element->Generation != Generation || Element->Depth < Replace->Depth )
				Replace = Element;
		}
		else if( Element->Generation != Generation && Element->Depth < Replace->Depth )
			Replace = Element;
	}

	if( Value > MATESCORE - 300 ) Value += DistanceFromRoot - 1;
	if( Value < -MATESCORE + 300 ) Value -= DistanceFromRoot - 1;

	Replace->Depth = DepthRemaining;
	Replace->Generation = Generation;
	Replace->Type = Type;
	Replace->Value = Value;
	Replace->BestMove = Move;
	Replace->Key = Zobrist;

	assert( Replace->Depth == DepthRemaining );
	assert( Replace->Type == Type );
	assert( Replace->Value == Value );
	assert( Replace->BestMove == Move );
	assert( Replace->Key == Zobrist );

	assert( DepthRemaining >= 0 );
}

void transpositionTable::StorePV( position* Position, line PV, int DepthRemaining, int Value )
{
	for( int i = 0; i <= DepthRemaining / Ply( 1 ); i++ )
	{
		move Move = ( i < PV.size() ) ? PV[ i ] : NullMove;

		Store( Position->Zobrist, i, DepthRemaining - Ply( i ), EXACT, Value, Move );
		if( Move != NullMove )
			Position->MakeMove( PV[ i ] );
		else
			break;
		Value = -Value;
	}

	for( int i = 0; i < PV.size() && i <= DepthRemaining / Ply( 1 ); i++ )
		Position->TakeBack();
}

void transpositionTable::DebugProbe( zobrist Zobrist ) const
{
	printf( "Looking up 0x%I64x. Mask is 0x%I64x.\n Transposition table dump for bin %I64x:\n", Zobrist, IndexMask, ( Zobrist & IndexMask ) );

	element* Element = NULL;
	for( int i = 0; i < 3; i++ )
	{
		Element = &Data[ 3 * ( Zobrist & IndexMask ) + i ];
		cout << "Element " << i + 1 << ":\n";
		printf( "\tKey:\t0x%I64x%s\n", Element->Key, ( Element->Key == Zobrist ) ? "*" : "" );
		cout << "\tGeneration:\t" << Element->Generation << "\n";
		cout << "\tDepth:\t" << Element->Depth << "\n";
		cout << "\tType:\t" << Element->Type << "\n";
		cout << "\tValue:\t" << Element->Value << "\n";
		cout << "\tMove:\t" << Element->BestMove.toString() << "\n";
	}

	int Used[ 3 ] = { 0, 0, 0 };

	for( unsigned int i = 0; i < NumberOfEntries; i++ )
	{
		if( Data[ 3 * i + 0 ].Key != 0 ) Used[ 0 ]++;
		if( Data[ 3 * i + 1 ].Key != 0 ) Used[ 1 ]++;
		if( Data[ 3 * i + 2 ].Key != 0 ) Used[ 2 ]++;
	}

	cout << "There are " << NumberOfEntries << " entries.\n";
	float Percentages[ 3 ];
	for( int i = 0; i < 3; i++ )
	{
		Percentages[ i ] = 100.0f * ( float )Used[ i ] / ( float )NumberOfEntries;
		cout << Percentages[ i ] << "% of bin #" << i + 1 << " is used.\n";
	}
}

int transpositionTable::StartDepth( zobrist Zobrist, int& StartValue )
{
	element* Element = &Data[ Zobrist & IndexMask ];
	if( Element->Key == Zobrist && Element->Type == EXACT && Element->BestMove != NullMove )
	{
		int Value = Element->Value;
		if( Value > MATESCORE - 300 ) Value -= 0 - 1;
		if( Value < -MATESCORE + 300 ) Value += 0 - 1;
		StartValue = Value;

		return Element->Depth / Ply( 1 ) + 1;
	}

	return 0;
}
