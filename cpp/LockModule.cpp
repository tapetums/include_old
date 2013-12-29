// LockModule.cpp

#include "LockModule.h"

//---------------------------------------------------------------------------//

extern ULONG g_cLocks = 0;

//---------------------------------------------------------------------------//

void __stdcall LockModule()
{
     InterlockedIncrement(&g_cLocks);
}

//---------------------------------------------------------------------------//

void __stdcall UnlockModule()
{
    InterlockedDecrement(&g_cLocks);
}

//---------------------------------------------------------------------------//

// LockModule.cpp