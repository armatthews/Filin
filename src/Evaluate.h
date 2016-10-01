#include "Utilities.h"
#include "position.h"

struct pawnScore
{
	zobrist Hash;

	int MiddleGameScore;
	int EndGameScore;

	byte KingSideDefects[ 2 ];
	byte QueenSideDefects[ 2 ];
	byte DFileDefects[ 2 ];
	byte EFileDefects[ 2 ];

	byte All[ 2 ];
	byte Passed[ 2 ];
	byte Hidden[ 2 ];
	byte Candidates[ 2 ];

	byte OpenFiles;

	pawnScore();
};

typedef int phase;
const phase MiddleGame = 0;
const phase EndGame = 1;

struct evaluation
{
	pawnScore PawnScore;
	square KingSquare[ 2 ];
	int GamePhase;
	int Material;
	int MajorPieceBalance;
	int MinorPieceBalance;

	int RootCastlingPrivileges;
	int CurrentCastlingPrivileges;

	bool Dangerous[ 2 ];
	bool CanWin[ 2 ];

	int PawnsLeft[ 2 ];
	int PieceMaterial[ 2 ];
	int SimpleMaterial[ 2 ];

	int Tropism[ 2 ];

	int Scores[ 2 ];
	int FinalScore;

	evaluation();
};

class evaluator
{
private:
	evaluation Evaluation;
	position* Position;
	bool Done;

	pawnScore LastPawnScore;

public:
	bool Verbose;
	vector< string > VerboseEval;

	int Evaluate( position* InPosition, int RootCastlingFlags, int Alpha = -INFINITE, int Beta = +INFINITE, evaluation* OutEvaluation = NULL );
	static int PieceMaterial( const position* const InPosition );

	evaluator();

private:

	void CalculateMaterial();
	bool EvaluateWinningChances( color c );
	void EvaluateMate( color Winner );
	void EvaluateWinnability();
	void EvaluateDevelopment( color c );
	void EvaluatePieces();
	int EvaluateDraws( int Score );

	void EvaluatePawns( color c );
	void EvaluateKnights( color c );
	void EvaluateBishops( color c );
	void EvaluateRooks( color c );
	void EvaluateQueens( color c );
	void EvaluateKings( color c );

	int EvaluateKingsFile( color c, int file );
	void EvaluatePassedPawns( color c );
	void EvaluatePassedPawnRaces( color c );

	void InitEval( position* InPosition, int RootCastlingFlags );
	void InitFullEval();
	void FinishEval( evaluation* Out );
};

#pragma once