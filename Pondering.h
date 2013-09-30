#include "Utilities.h"
#include "searcher.h"
#include "position.h"
#define WINAPI
typedef int HANDLE;

namespace pondering
{
	struct ponderInfo
	{
		position* Position;
		searcher* Searcher;
	};

	extern ponderInfo PonderInfo;

	extern bool PonderingEnabled;
	extern bool Pondering;
#ifndef __WIN32__
	extern pthread_attr_t PonderThread;
	extern pthread_t PonderThreadID;
#else
	extern HANDLE PonderThread;
	extern DWORD PonderThreadID;
#endif
	extern move PonderingMove;
	extern DWORD PonderedTime;

	DWORD Ponder( void* Info );
	void StopPondering();
	void StartPondering( position* Position, searcher* Searcher );
}
