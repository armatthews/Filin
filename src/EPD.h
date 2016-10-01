#include <vector>
#include <map>
#include <string>
#include "Utilities.h"
#include "position.h"
#include "move.h"

class EPDPosition
{
private:
	position Position;
	std::map< std::string, std::string > Operations;

	std::vector< move > BestMoves;
	std::string ID;

public:
	bool Load( std::string String );
	bool IsCorrect( move Move );

	position GetPosition();
	std::string GetOperation( std::string op );
	std::vector< move > GetBestMoves();
};

class EPD
{
protected:
	std::vector< EPDPosition > Positions;
	std::string FileName;

public:
	bool Load( std::string FileName );
};

class TestSuite : public EPD
{
private:
	unsigned int Correct;
public:	
	unsigned long TimeLimit;

	TestSuite();
	void Run();

	// Time/Node Limits
	// Output lines
	// HTML File generator?
};

#pragma once
