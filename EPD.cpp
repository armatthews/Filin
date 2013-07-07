#include <string>
#include "searcher.h"
#include "EPD.h"
using std::string;
using std::streambuf;
using std::ostringstream;

string::size_type FindNthOccurance( string String, string Pattern, unsigned int n )
{
	size_t Trimmed = 0;

	for( unsigned int i = 0; i < n - 1; i++ )
	{
		size_t Start = String.find( Pattern );

		if( Start == string::npos )
			return string::npos;

		string::size_type End = Start + Pattern.length();

		Trimmed += End;
		String = String.substr( End );
	}

	string::size_type Start = String.find( Pattern );
	if( Start == string::npos )
		return string::npos;

	return Start + Trimmed;
}

string RemoveQuotes( string s )
{
	if( s.length() < 2 )
		return s;

	if( ( s[ 0 ] == '\'' && s[ s.length() - 1 ] == '\'' ) || ( s[ 0 ] == '\"' && s[ s.length() - 1 ] == '\"' ) )
		return s.substr( 1, s.length() - 1 );

	return s;
}

position EPDPosition::GetPosition()
{
	return Position;
}

std::string EPDPosition::GetOperation( std::string op )
{
	return Operations[ op ];
}

std::vector< move > EPDPosition::GetBestMoves()
{
	return BestMoves;
}

bool EPDPosition::Load( string String )
{
	string::size_type EndOfFEN = FindNthOccurance( String, " ", 4 );
	string FEN = String.substr( 0, EndOfFEN ) + " 0 1";
	if( !Position.SetUpPosition( FEN ) )
		return false;

	string Ops = Trim( String.substr( EndOfFEN ) );

	while( Ops.find( ";" ) != string::npos )
	{
		string ThisOp = Ops.substr( 0, Ops.find( ";" ) );

		string::size_type Space = ThisOp.find( " " );
		if( Space == string::npos ) return false;

		string Key = ThisOp.substr( 0, Space );
		string Value = Trim( ThisOp.substr( Space ) );
		Operations[ Key ] = Value;

		Ops = Trim( Ops.substr( Ops.find( ";" ) + 1 ) );
	}

	for( map< string, string >::iterator it = Operations.begin(); it != Operations.end(); ++it )
	{
		if( it->first == "id" )
			ID = RemoveQuotes( it->second );
		else if( it->first == "bm" )
		{
			string Moves = it->second;
			while( 1 )
			{
				string::size_type NextSpace = Moves.find( " " );
				string SAN = Moves.substr( 0, NextSpace );
				move Move = move( SAN, &Position );
				if( Move == NullMove )
					Move = move( SAN );
				BestMoves.push_back( Move );
				Moves = Moves.substr( NextSpace + 1 );

				if( NextSpace == string::npos )
					break;
			}
		}
	}

	return true;
}

bool EPDPosition::IsCorrect( move Move )
{
	for( unsigned int i = 0; i < BestMoves.size(); i++ )
		if( BestMoves[ i ] == Move )
			return true;
	return false;
}

bool EPD::Load( string FileName )
{
	string Line;
	ifstream Stream( FileName.c_str() );
	Positions.clear();
	this->FileName = FileName;

	if( !Stream.is_open() )
		return false;

	while( !Stream.eof() )
	{
		getline( Stream, Line );
		Line = Trim( Line );

		if( Line == "" )
			continue;

		EPDPosition NewPosition;
		if( !NewPosition.Load( Line ) )
			return false;

		Positions.push_back( NewPosition );
	}

	return true;
}

TestSuite::TestSuite()
{
	TimeLimit = 1000;
}

void TestSuite::Run()
{
	searcher Searcher;
	Correct = 0;

	DWORD StartTime = timeGetTime();

	for( unsigned int i = 0; i < Positions.size(); i++ )
	{
		ostringstream output;
		streambuf* OldCout( cout.rdbuf() );
		cout.rdbuf( output.rdbuf() );

		Searcher.Search( &Positions[ i ].GetPosition(), 0, TimeLimit );

		string Output = output.str();
		cout.rdbuf( OldCout );

		line PV = Searcher.GetPrincipleVariation();
		if( PV.size() > 0 )
		{
			move Move = PV[ 0 ];
			if( Positions[ i ].IsCorrect( Move ) )
			{
				cout << "O" << " ";
				Correct++;
			}
			else
				cout << "X" << " ";

			cout << Correct << " / " << i + 1 << "\t" << Move.toString( &Positions[ i ].GetPosition() ) << "\t";
			vector< move > BestMoves = Positions[ i ].GetBestMoves();
			for( unsigned int j = 0; j < BestMoves.size(); j++ )
				cout << BestMoves[ j ].toString( &Positions[ i ].GetPosition() ) << " ";
			cout << "\n";
		}
		else
			cout << "[Error] No move found!\n";
	}

	float Percentage = 100.0f * ( float )( Correct ) / ( float )( Positions.size() );
	cout << FileName << " completed in " << ( timeGetTime() - StartTime ) << " ms.\n";
	cout << "Filin scored " << Correct << " out of " << Positions.size() << ", or " << Percentage << "%.\n";
}