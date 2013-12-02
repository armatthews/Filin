#include "Utilities.h"
#include "position.h"
#include "searcher.h"
#include "move.h"

enum status { INPROGRESS = 0, WHITEWIN, DRAW, BLACKWIN };
enum reason { OUTOFTIME = 1, THREEFOLDREPETITION, FIFTYMOVERULE, CHECKMATE, STALEMATE, INSUFFICIENTMATERIAL };
enum inputStatus { INPUT_QUIT, INPUT_HANDLED, INPUT_INCORRECTARGUMENTS, INPUT_ERROR, INPUT_UNKNOWN };

class IOHandler
{
protected:
	position Position;
	searcher Searcher;

	struct
	{
		int TimeSetting;
		bool TimeLimited;
		color FilinsColor;
		bool XBoardMode;
		bool Post;
		// Pondering and Book bools are in their respective files. Perhaps they should be duplicated here?
	} Preferences;

	inputStatus ToggleOption( bool* Option, string State );
	inputStatus HandleInput( string s );
	inputStatus RunCommand( string Command, vector< string >& Parameters );
	bool GameIsOver();
	std::vector<move> GetMoveList();

	inputStatus EmitFeatures();
	inputStatus Go();
	inputStatus ListMoves();
	inputStatus DisplayEval();
	inputStatus ShowStatistics();
	inputStatus RunTest( vector< string >& Paramters );
	inputStatus Perft( vector< string >& Paramters );
	inputStatus Debug( vector< string >& Paramters );

	UINT64 Perft( unsigned int Depth );
	inputStatus Divide( unsigned int Depth );

public:
	IOHandler();
	void Begin();
};

#pragma once
