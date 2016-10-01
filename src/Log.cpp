#include "Utilities.h"

void ClearLog()
{
	ofstream f( "FilinLog.txt" );
	f.close();
}

void Log( string text )
{
	ofstream f( "FilinLog.txt", std::ios_base::out | std::ios_base::app );
	f << text;
	f.close();
}