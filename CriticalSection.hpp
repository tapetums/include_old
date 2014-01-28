// CriticalSection.hpp

#pragma once

//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//

class CriticalSection
{
public:
    CriticalSection()  { ::InitializeCriticalSection(&cs); }
    ~CriticalSection() { ::DeleteCriticalSection(&cs); }
    void __stdcall lock()   { ::EnterCriticalSection(&cs); }
    void __stdcall unlock() { ::LeaveCriticalSection(&cs); }

private:
    CRITICAL_SECTION cs;
};

//---------------------------------------------------------------------------//

// CriticalSection.hpp