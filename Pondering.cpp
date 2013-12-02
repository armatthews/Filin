#ifndef __WIN32__
#include <pthread.h>
#include <unistd.h>
#endif
#include "Pondering.h"

namespace pondering
{
	bool PonderingEnabled = false;
	bool Pondering = false;
#ifndef __WIN32__
	pthread_attr_t PonderThread;
	pthread_t PonderThreadID;
#else
	HANDLE PonderThread;
	DWORD PonderThreadID;
#endif
	move PonderingMove;

	DWORD PonderedTime = 0;

	ponderInfo PonderInfo;

	THREADRETTYPE WINAPI Ponder( void* Info )
	{	
		PonderedTime = 0;
		DWORD Start = timeGetTime();

		assert( Info != NULL );

		position Position = *( ( ponderInfo* )( Info ) )->Position;
		searcher* Searcher = ( ( ponderInfo* )( Info ) )->Searcher;
		string StartFEN = Position.FEN();

		move PonderMove = NullMove;
		if(Searcher->ThinkingPosition.FEN() == Position.FEN())
		{
			// This code causes Filin to sometimes return a bogus move,
			// and thus crash. Either verify that the move returned is legal,
			// or ensure that it doesn't return bogus moves.
			//line PV = Searcher->GetPrincipleVariation();
			//if( PV.size() > 1 )
			//	PonderMove = PV[ 1 ];
		}

		if( PonderMove == NullMove )
		{
			Searcher->Search( &Position, 6, 0 );
			line PV = Searcher->GetPrincipleVariation();
			if( PV.size() == 0 )
			{
				Pondering = false;
				return THREADRETVALUE;
			}
			PonderMove = PV[ 0 ];	
		}	
		
		cout << "Hint: " << PonderMove.toString( &Position ) << "\n";
		Position.MakeMove( PonderMove );
		Searcher->Search( &Position, 0, 0 );
		Position.TakeBack();

		string EndFEN = Position.FEN();
		assert( StartFEN == EndFEN );
		Pondering = false;
		return THREADRETVALUE;
	}

	void StopPondering()
	{
		if( !Pondering )
			return;

		while( Pondering )
		{
			PonderInfo.Searcher->Stop();
		#ifndef __WIN32__
			usleep(1);
		#else
			Sleep( 1 );
		#endif
		}
	#ifndef __WIN32__
		pthread_attr_destroy( &PonderThread );
	#else
		CloseHandle( PonderThread );
	#endif
	}

	void StartPondering( position* Position, searcher* Searcher )
	{
		if( PonderingEnabled )
		{
			StopPondering();

			PonderInfo.Position = Position;
			PonderInfo.Searcher = Searcher;

			Pondering = true;
		#ifndef __WIN32__	
			pthread_attr_init(&PonderThread);
			//pthread_attr_setstacksize(&PonderThread, 120*1024);
			//pthread_attr_setdetachstate(&PonderThread, PTHREAD_CREATE_DETACHED);
			pthread_create( &PonderThreadID, &PonderThread, Ponder, (void*)&PonderInfo );
		#else
			PonderThread = CreateThread( NULL, 0, Ponder, &PonderInfo, 0, &PonderThreadID );
		#endif	
		}
	}
}
