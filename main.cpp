#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include "IOHandler.h"
#pragma comment( lib, "winmm.lib" )

/*********************************************************************************************************************************************************************************************************************
*	Suite		Max		3/1	3/2I	3/2II	3/2III	3/2IVa	3/2IVb	3/3Ia	3/3Ib	3/3II	3/3III	3/4I	3/4IIa	3/4IIb	3/4III	4/9	4/13	4/14	5/4	5/6	5/10	7/14
* --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*	TESTWCSAC	100		55	54		49		28		31		33		28		32		42		37		38		51		47		48		61	60		60		63	64	64		66
*	TESTECM		100		36	35		28		18		16		17		17		15		26		18		16		29		28		30		36	36		36		38	37	42		43
*	TESTWAC		100		73	64		63		32		33		35		32		33		62		34		46		67		66		64		81	80		80		80	82	84		84
*	WAC			300		250	228		238		159		162		167		155		162		222		175		203		239		238		236		261	259		255		260	263	266		267
*	ECM			879		398	376		329		171		175		186		169		168		297		204		236		364		347		342		402	403		407		415	429	452		449
*	WCSAC		1001	*	*		696		502		515		544		494		522		673		571		627		736		711		703		756	764		763		763	771	779		777
*	BWTC		1001	907	837		930		699		720		752		704		726		915		*		823		899		891		889		928	931		932		929	939	939		942
*	CCC1		11		*	*		*		*		*		*		*		*		*		*		*		*		*		*		*	6		6		6	6	7		
*	CCC2		11		*	*		*		*		*		*		*		*		*		*		*		*		*		*		*	2		2		1	2	2		
*	CCC3		45		*	*		*		*		*		*		*		*		*		*		*		*		*		*		*	24		24		25	27	27		
*	CCC4		4		*	*		*		*		*		*		*		*		*		*		*		*		*		*		*	2		2		2	2	2		
*********************************************************************************************************************************************************************************************************************/

/*
* 3/1 - No PVS, futility or aspiration - Note broken LMR w/o re-search
* 3/2I - PVS but no futility and no aspiration - Note broken LMR w/o re-search
* 3/2II - No PVS, No Futility, No Aspiration, No Null, No TT
* 3/2III - Straight up AB. SEE > 0 QS. No Null, Futility, Aspiration, PVS, TT, Killers, Extensions (except +1 check at root), Reductions. Laptop.
* 3/2IVa - Straight up AB w/ PVS. SEE > 0 QS. No Null, Futility, Aspiration, TT, Killers, Extensions (except +1 check at root), Reductions. Laptop.
* 3/2IVb - Straight up AB w/ PVS. SEE > 0 QS. No Null, Futility, Aspiration, TT, Killers, Extensions (except +1 check at root), Reductions.
* 3/3Ia - Straight up AB w/ NS. SEE > 0 QS. No Null, Futility, Aspiration, TT, Killers, Extensions (except +1 check at root), Reductions. Laptop.
* 3/3Ib - Straight up AB w/ NS. SEE > 0 QS. No Null, Futility, Aspiration, TT, Killers, Extensions (except +1 check at root), Reductions.
* 3/3II - Straight up AB w/ PVS. SEE > 0 QS. No Null, Futility, Aspiration, TT, Killers. Check Extension and LMR Enabled.
* 3/3III - AB w/ PVS. SEE > 0 QS. No Null, Futility, Aspiration, TT, Extensions (except +1 check at root), Reductions.
* 3/4I - AB w/ PVS. SEE > 0 QS. No futility or Aspiration, Extensions (except +1 check at root), Reductions.
* 3/4IIa - AB w/ PVS. SEE > 0 QS. No futility or Aspiration. Check Extension and LMR Enabled.
* 3/4IIb - AB w/ PVS. SEE > 0 QS. No futility or Aspiration. Check Extension and LMR Enabled. Laptop
* 3/4III - AB w/ PVS. SEE > 0 QS. No Aspiration. Futility, Check Extension and LMR Enabled.
* 4/9 - AB w/ PVS. SEE > 0 QS. No Aspiration. Futility, Check Extension and LMR Enabled. Evaluation tuned up.
* 4/13 - Evaluation tweaks. PVS implementation modified a bit.
* 4/14 - Moved losing captures before non-captures.
* 5/4 - Fixed bug in LMR code.
* 5/6 - Incremental move generator.
* 5/10 - Implemented pawn hash. Also changed Population function. Since most bitboards are sparse, a naive implementation seems fastest.
* 7/14 - Optimized Perft function. Added NextType variable to LegalMoveGenerator.
*/
void handler(int sig) {
	void* array[10];
	int size = backtrace(array, 10);
	cerr << "Error: signal " << sig << ":\n";
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

int main()
{
	cerr << "Filin v0.1" << endl;
	signal(SIGSEGV, handler);

	timeBeginPeriod(1);
	srand(timeGetTime());
	ClearLog();
	Log("Starting up...\n");

	IOHandler IOHandler;
	IOHandler.Begin();

	Log("Exiting...\n");
	timeEndPeriod(1);

	return 0;
}
