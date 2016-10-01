#include <cstdlib>
#include "IOHandler.h"
#include "Pondering.h"
#include "EPD.h"

IOHandler::IOHandler()
{
	Preferences.TimeSetting = 5000;
	Position.SetUpPosition( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );

	//Position.SetUpPosition( "1rb2rk1/p4ppp/1p1qp1n1/3n2N1/2pP4/2P3P1/PPQ2PBP/R1B1R1K1 w - - 0 1" ); // TSCP bench
	//Position.SetUpPosition( "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1" ); // Fine #70 -- Transposition Table Test
}

void IOHandler::Begin()
{
	Log( "Beginning input loop\n" );
	cout.setf( std::ios::unitbuf );
	cin.setf( std::ios::unitbuf );

	char input [ 512 ];
	while( 1 )
	{
		cin.getline( input, 511 );

		if( strlen( input ) == 0 || !strcmp( input, "\n" )  )
			continue;

		int Result = HandleInput( input );

		if( Result == INPUT_HANDLED )
			;
		else if( Result == INPUT_INCORRECTARGUMENTS )
			printf( "Incorrect arguments for command \"%s\"\n", input );
		else if( Result == INPUT_ERROR )
			printf( "Error in command \"%s\"\n", input );
		else if( Result == INPUT_UNKNOWN )
			printf( "Unknown command: \"%s\"\n", input );
		else if( Result == INPUT_QUIT )
			break;
	}
}

inputStatus IOHandler::HandleInput( string s )
{
	Log( "Input: " + s + "\n" );
	
	pondering::StopPondering();

	inputStatus r = INPUT_UNKNOWN;

	move InputMove = move( s, &Position );
	if( InputMove == NullMove )
		InputMove = move( s );	

	if( InputMove != NullMove )
	{
		Position.MakeMove( InputMove );
		if( Position.ToMove == Preferences.FilinsColor )
			Go();
		r = INPUT_HANDLED;
	}
	else
	{
		vector< string > Parameters = Tokenize( s, " \t\n" );
		string Command = ( Parameters.size() > 0 ) ? Parameters[ 0 ] : "";
		Parameters.erase( Parameters.begin() );
		r = RunCommand( Command, Parameters );
	}

	pondering::StartPondering( &Position, &Searcher );
	return r;
}

inputStatus IOHandler::RunCommand( string Command, vector< string >& Parameters )
{
	if( Command == "" )
		return INPUT_UNKNOWN;
	else if( Command == "accepted" )
		;
	else if( Command == "analyze" )
		;
	else if( Command == "book" )
	{
		if( Parameters.size() > 0 )
			return ToggleOption( &USE_BOOK, Parameters[ 0 ] );
		else
			return INPUT_INCORRECTARGUMENTS;
	}
	else if( Command == "computer" )
		;
	else if( Command == "draw" )
		;
	else if( Command == "debug" )
		return Debug( Parameters );
	else if( Command == "divide" )
	{
		if( Command.size() > 0 )
		{
			Searcher.ThinkingPosition = Position;
			return Divide( atoi( Parameters[ 0 ].c_str() ) );
		}
		else
			return INPUT_INCORRECTARGUMENTS;
	}
	else if( Command == "easy" )
	{
		pondering::PonderingEnabled = false;
		pondering::StopPondering();
	}
	else if( Command == "evaluate" )
		return DisplayEval();
	else if( Command == "fen" )
		cout << Position.FEN() << "\n";
	else if( Command == "force" )
		Preferences.FilinsColor = Empty;
	else if( Command == "go" )
	{
		Preferences.FilinsColor = Position.ToMove;
		Go();
	}
	else if( Command == "hard" )
		pondering::PonderingEnabled = true;
	else if( Command == "ics" )
		;
	else if( Command == "level" )
		;
	else if( Command == "move" )
	{
		if( Parameters.size() > 0 )
			return HandleInput( Parameters[ 0 ] );
		else
			return INPUT_INCORRECTARGUMENTS;
	}
	else if( Command == "moves" ) {
		return ListMoves();
	}
	else if( Command == "name" )
		;
	else if( Command == "new" )
	{
		Position.SetUpPosition( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
		Preferences.FilinsColor = Black;
		Preferences.TimeLimited = false;
	}
	else if( Command == "nopost" )
		;
	else if( Command == "otim" )
		;
	else if( Command == "pause" )
		;
	else if( Command == "perft" )
	{
		Searcher.ThinkingPosition = Position;
		return Perft( Parameters );
	}
	else if( Command == "pgn" )
	{
		position Position = this->Position;
		int NumberOfMoves = Position.MoveHistory.size();
		std::vector< takeback > MoveHistory = Position.MoveHistory;

		for( int i = 0; i < NumberOfMoves; i++ )
			Position.TakeBack();

		for( int i = 0; i < NumberOfMoves; i++ )
		{
			if( i % 2 == 0 )
				cout << ( i / 2 ) + 1 << ".";

			cout << " " << MoveHistory[ i ].toString( &Position );
			Position.MakeMove( MoveHistory[ i ] );
			
			if( i % 2 == 1 )
				cout << "\n";
		}
	}
	else if( Command == "ping" )
		printf( "pong %d\n", atoi( Parameters[ 0 ].c_str() ) );
	else if( Command == "playother" )
		Preferences.FilinsColor = Position.Enemy;
	else if( Command == "pondering" )
	{
		if( Parameters.size() > 0 )
			return ToggleOption( &pondering::PonderingEnabled, Parameters[ 0 ] );
	}
	else if( Command == "post" )
		;
	else if( Command == "probe" )
		Searcher.TTDebugProbe( Position.Zobrist );
	else if( Command == "protover" )
	{
		if( Parameters.size() > 0 )
		{
			int Version = atoi( Parameters[ 0 ].c_str() );
			if( Version >= 2 )
				return EmitFeatures();
			else
			{
				cout << "Unsupported XBoard Version!\n";
				return INPUT_ERROR;
			}
		}
		else
			return INPUT_INCORRECTARGUMENTS;
	}
	else if( Command == "quit" )
		return INPUT_QUIT;
	else if( Command == "random" )
		;
	else if( Command == "rating" )
		;
	else if( Command == "rejected" )
		;
	else if( Command == "remove" )
	{
		Position.TakeBack();
		Position.TakeBack();
	}
	else if( Command == "reset" )
		Searcher.Reset( true );
	else if( Command == "result" )
		;
	else if( Command == "resume" )
		;
	else if( Command == "reverse" )
	{
		position Reversed;
		Position.Reverse( Reversed );
		Position = Reversed;
		cout << Position.toString() << "\n\n";
	}
	else if( Command == "sd" )
		;
	else if( Command == "setboard" )
	{
		position OldPosition = Position;
		string FEN = "";
		for( unsigned int i = 0; i < Parameters.size(); i++ )
		{
			FEN += Parameters[ i ];
			if( i != Parameters.size() - 1 )
				FEN += " ";
		}
		if( !Position.SetUpPosition( FEN ) )
		{
			cout << "Invalid position: \"" << FEN << "\"\n";
			Position = OldPosition;
		}
	}
	else if( Command == "showboard" )
		cout << Position.toString() << "\n";
	else if( Command == "st" )
	{
			Preferences.TimeSetting = atof( Parameters[ 0 ].c_str() ) * 1000;
			Preferences.TimeLimited = true;
	}
	else if( Command == "stats" )
		return ShowStatistics();
	else if( Command == "takeback" )
		return HandleInput( "undo" );
	else if( Command == "test" )
		return RunTest( Parameters );
	else if( Command == "time" )
	{
		if( !Preferences.TimeLimited )
			Preferences.TimeSetting = atoi( Parameters[ 0 ].c_str() ) / 3;
	}
	else if( Command == "undo" )
	{
		Position.TakeBack();
		Preferences.FilinsColor = FlipColor( Preferences.FilinsColor );
	}
	else if( Command == "xboard" )
		;
	else if( Command == "zobrist" )
		printf( "0x%llx\n", Position.Zobrist );
	else
		return INPUT_UNKNOWN;

	return INPUT_HANDLED;
}

inputStatus IOHandler::ToggleOption( bool* Option, string State )
{
	if( State == "on" )
		*Option = true;
	else if( State == "off" )
		*Option = false;
	else
		return INPUT_UNKNOWN;

	return INPUT_HANDLED;
}

vector<move> IOHandler::GetMoveList()
{
	Searcher.ThinkingPosition = Position;
	moveSorter MoveGenerator( &Searcher, 0 );

	move Move;
	vector<move> MoveList;
	while( ( Move = MoveGenerator.GetNextMove() ) != NullMove ) {	
		MoveList.push_back( Move );
	}
	return MoveList;
}


inputStatus IOHandler::ListMoves()
{
	vector<move> MoveList = GetMoveList();
	if( MoveList.size() == 0 )
	{
		cout << "No moves possible!\n\n";
		return INPUT_HANDLED;
	}

	for( int i = 0; i < MoveList.size(); i++ )
	{
		move Move = MoveList[ i ];
		cout << Move.toString( &Position ) << " ";
		if( i % 5 == 4 )
			cout << "\n";
	}
	cout << "\n";
	return INPUT_HANDLED;
}

inputStatus IOHandler::Perft( vector< string >& Command )
{
	if( Command.size() == 0 )
		return INPUT_INCORRECTARGUMENTS;

	int Depth = atoi( Command[ 0 ].c_str() );
	if( Depth < 0 )
		return INPUT_INCORRECTARGUMENTS;

	DWORD StartTime = timeGetTime();
	UINT64 Result = Perft( Depth );
	DWORD EndTime = timeGetTime();

	cout << "Perft " << Depth << ": " << Result << "\n";
	cout << "Time taken: " << ( EndTime - StartTime ) << " milliseconds\n";

	return INPUT_HANDLED;
}

UINT64 IOHandler::Perft( unsigned int d )
{
	if( d <= 0 )
		return 1;

	UINT64 r = 0;
	Generator MoveGenerator( &Searcher.ThinkingPosition );

	if( d == 1 )
	{
		move Move;
		while( MoveGenerator.GetNextMove() != NullMove )
			r++;
		return r;
	}

	move Move;
	while( ( Move = MoveGenerator.GetNextMove() ) != NullMove )
	{
		Searcher.ThinkingPosition.MakeMove( Move );
#ifdef _DEBUG
		if( Position.InCheck( Position.Enemy ) )
		{
			Position.TakeBack();
			cout << "ERROR:\n";
			cout << Position.FEN() << "\n";
			cout << Position.toString() << "\n" << Move.toString( &Position ) << "\n";
			assert( 0 );
			system( "PAUSE" );
		}
#endif
		r += Perft( d - 1 );
		Searcher.ThinkingPosition.TakeBack();	
	}

	return r;
}

inputStatus IOHandler::Divide( unsigned int Depth )
{
	Generator MoveGenerator( &Searcher.ThinkingPosition );
	move Move;
	UINT64 Total = 0;

	while( ( Move = MoveGenerator.GetNextMove() ) != NullMove )
	{
		Searcher.ThinkingPosition.MakeMove( Move );
		UINT64 divide = Perft( Depth - 1 );
		Searcher.ThinkingPosition.TakeBack();
		cout << Move.toString( &Position ) << ": " << divide << "\n";
		Total += divide;
	}

	cout << "Total: " << Total << "\n";

	return INPUT_HANDLED;
}

inputStatus IOHandler::DisplayEval()
{
	Searcher.Evaluator.Verbose = true;
	cout << "Evaluation: " << Searcher.Evaluator.Evaluate( &Position, Position.Castling ) << "\n";
	for( unsigned int i = 0; i < Searcher.Evaluator.VerboseEval.size(); i++ )
		cout << Searcher.Evaluator.VerboseEval[ i ];
	Searcher.Evaluator.VerboseEval.clear();
	Searcher.Evaluator.Verbose = false;

	return INPUT_HANDLED;
}

inputStatus IOHandler::ShowStatistics()
{
	cout << "Nodes searched: " << Searcher.Statistics.NodesVisited << "\n";
	cout << "Quiescent nodes searched: " << Searcher.Statistics.QuiescentNodesVisited << "\n";
	cout << "Nodes per second: " << Searcher.Statistics.NodesVisited / Searcher.Statistics.SearchTime << " kN/s\n";
	cout << "Check extensions: " << Searcher.Statistics.CheckExtensionsDone << "\n";
	cout << "Late move reductions: " << Searcher.Statistics.LateMoveReductionsDone << "\n";
	cout << "Reduction Researches: " << Searcher.Statistics.ReductionResearches << "\n";
	cout << "Null move attempts: " << Searcher.Statistics.NullMoveAttempts << "\n";
	cout << "Null move hits: " << Searcher.Statistics.NullMoveSuccesses << "\n";
	for( int i = 0; i < 5; i++ )
	{
		cout << "Move #" << i + 1 << " fail highs: " << Searcher.Statistics.FailHighsByMove[ i ] << "\n";
	}
	int total = 0;
	for( int i = 5; i < 50; i++ )
		total += Searcher.Statistics.FailHighsByMove[ i ];
	cout << "Other fail highs: " << total << "\n";

	return INPUT_HANDLED;
}

inputStatus IOHandler::RunTest( vector< string >& Parameters )
{
	TestSuite Test;
	Test.TimeLimit = 2000;

	if( Parameters.size() == 0 || ( Parameters[ 0 ] != "full" && Parameters[ 0 ] != "long" && Parameters[ 0 ] != "short" ) )
		return INPUT_INCORRECTARGUMENTS;

	if(Test.Load( "Test Suites//TESTWCSAC.epd" ) )
		Test.Run();

	if( Test.Load( "Test Suites//TESTECM.epd" ) )
		Test.Run();

	if (Test.Load( "Test Suites//TESTWAC.epd" ) )
		Test.Run();

	if( Parameters[ 0 ] == "full" || Parameters[ 0 ] == "long" )
	{
		Test.Load( "Test Suites//WAC.epd" );
		Test.Run();

		Test.Load( "Test Suites//ECM.epd" );
		Test.Run();

		Test.Load( "Test Suites//WCSAC.epd" );
		Test.Run();

		Test.Load( "Test Suites//BWTC.epd" );
		Test.Run();
	}

	if( Parameters[ 0 ] == "full" )
	{
		Test.TimeLimit = 60000;
		Test.Load( "Test Suites//CCC-1.epd" );
		Test.Run();

		Test.Load( "Test Suites//CCC-2.epd" );
		Test.Run();

		Test.Load( "Test Suites//CCC-3.epd" );
		Test.Run();

		Test.Load( "Test Suites//CCC-4.epd" );
		Test.Run();
	}

	return INPUT_HANDLED;
}

inputStatus IOHandler::Debug( vector< string >& Parameters )
{
	int Count = atoi( Parameters[ 0 ].c_str() );
	DWORD StartTime = timeGetTime();
	move Move = move( E2, E4, Empty );

	for( int i = 0; i < Count; i++ )
	{
		Position.MakeMove( Move );
		Position.TakeBack();
	}
	DWORD EndTime = timeGetTime();
	cout << "Took " << ( EndTime - StartTime ) / 1000.f << " seconds.\n";
	return INPUT_HANDLED;
}

inputStatus IOHandler::EmitFeatures()
{
	printf( "feature %s=%s\n", "done", "0" );
	printf( "feature %s=%s\n", "ping", "1" );
	printf( "feature %s=%s\n", "setboard", "1" );
	printf( "feature %s=%s\n", "playother", "1" );
	printf( "feature %s=%s\n", "san", "1" );
	printf( "feature %s=%s\n", "usermove", "0" );
	printf( "feature %s=%s\n", "time", "1" );
	printf( "feature %s=%s\n", "draw", "1" );
	printf( "feature %s=%s\n", "sigint", "0" );
	printf( "feature %s=%s\n", "sigterm", "0" );
	printf( "feature %s=%s\n", "reuse", "1" );
	printf( "feature %s=%s\n", "analyze", "0" );
	printf( "feature %s=%s\n", "myname", "\"Filin 5.12.3\"" );
	printf( "feature %s=%s\n", "variants", "\"normal\"" );
	printf( "feature %s=%s\n", "colors", "0" );
	printf( "feature %s=%s\n", "ics", "1" );
	printf( "feature %s=%s\n", "name", "1" );
	printf( "feature %s=%s\n", "pause", "1" );
	printf( "feature %s=%s\n", "done", "1" );

	return INPUT_HANDLED;
}

bool IOHandler::GameIsOver()
{
	bool IsOver = false;
	if( Position.IsDraw( 3 ) )
	{
		if( Position.Fifty >= 100 )
			cout << "1/2-1/2 {50 move rule}\n";
		else
			cout << "1/2-1/2 {3 fold repetition}\n";
		IsOver = true;
	}
	else if( Position.BothPieces == ( Position.Pieces[ White ][ King ] | Position.Pieces[ Black ][ King ] ) )
	{
		cout << "1/2-1/2 {insufficient material}\n";
		IsOver = true;
	}
	else
	{
		Generator MoveGenerator( &Position );
		move Move = MoveGenerator.GetNextMove();
		bool NoMovesExist = ( Move == NullMove );

		if( NoMovesExist )
		{
			if( Position.InCheck( Position.ToMove ) )
			{
				if( Position.ToMove == Black )
					cout << "1-0 {white mates}\n";
				else
					cout << "0-1 {black mates}\n";
			}
			else
				cout << "1/2-1/2 {stalemate}\n";

			IsOver = true;
		}
	}

	return IsOver;
}

inputStatus IOHandler::Go()
{
	if( GameIsOver() )
		return INPUT_HANDLED;

	int Score = Searcher.Search( &Position, 0, Preferences.TimeSetting );

	if( Searcher.GetPrincipleVariation().size() == 0 )
	{
		Log( "Error! No PV from " + Position.FEN() + "\n" );
		return INPUT_ERROR;
	}

	move Move = Searcher.GetPrincipleVariation()[ 0 ];
	cout << "move " << Move.toString( &Position ) << "\n";
	Position.MakeMove( Move );

	return INPUT_HANDLED;
}
