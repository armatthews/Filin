#include "Pondering.h"

namespace pondering
{
	bool PonderingEnabled = false;
	bool Pondering = false;
	HANDLE PonderThread;
	DWORD PonderThreadID;
	move PonderingMove;

	DWORD PonderedTime = 0;

	ponderInfo PonderInfo;

	DWORD WINAPI Ponder( void* Info )
	{
		//cout << "Starting...\n";
		PonderedTime = 0;
		DWORD Start = timeGetTime();

		assert( Info != NULL );

		position Position = *( ( ponderInfo* )( Info ) )->Position;
		searcher* Searcher = ( ( ponderInfo* )( Info ) )->Searcher;

		move PonderMove = NullMove;
		line PV = Searcher->GetPrincipleVariation();
		//if( PV.size() > 1 )
		//{
		//	cout << "From PV: " << PV[ 1 ].toString() << "\n";
		//	PonderMove = PV[ 1 ];
		//}

		if( PonderMove == NullMove )
		{
			Searcher->Search( &Position, 6, 0 );
			line PV = Searcher->GetPrincipleVariation();
			if( PV.size() == 0 )
				return 0;
			//cout << "From Puzzling: " << PV[ 0 ].toString() << "\n";
			PonderMove = PV[ 0 ];
		}

		cout << "Hint: " << PonderMove.toString( &Position ) << "\n";
		Position.MakeMove( PonderMove );

		Searcher->Search( &Position, 0, 0 );

		Position.TakeBack();

		Pondering = false;
		return 0;
	}

	void StopPondering()
	{
		if( !Pondering )
			return;

		while( Pondering )
		{
			PonderInfo.Searcher->Stop();
			Sleep( 1 );
		}
		//cout << "Stopping...\n";
		CloseHandle( PonderThread );
	}

	void StartPondering( position* Position, searcher* Searcher )
	{
		if( PonderingEnabled )
		{
			StopPondering();

			PonderInfo.Position = Position;
			PonderInfo.Searcher = Searcher;

			Pondering = true;
			PonderThread = CreateThread( NULL, 0, Ponder, &PonderInfo, 0, &PonderThreadID );
		}
	}
}