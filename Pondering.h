#include "Utilities.h"
#include "searcher.h"
#include "position.h"

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
	extern HANDLE PonderThread;
	extern DWORD PonderThreadID;
	extern move PonderingMove;
	extern DWORD PonderedTime;

	DWORD WINAPI Ponder( void* Info );
	void StopPondering();
	void StartPondering( position* Position, searcher* Searcher );
}
