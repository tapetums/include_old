/************************************************************
    Module Name: MeteredSection.h
    Original Author: Dan Chou
    Modified  by: tapetums
    Description: Defines the metered section synchronization object
************************************************************/

#ifndef _METERED_SECTION_H_
#define _METERED_SECTION_H_

#define MAX_METSECT_NAMELEN 128

//---------------------------------------------------------------------------//

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

//---------------------------------------------------------------------------//
//
// Shared info needed for metered section
//
//---------------------------------------------------------------------------//

typedef struct _METSECT_SHARED_INFO
{
    BOOL   fInitialized;     // Is the metered section initialized?
    LONG   lSpinLock;        // Used to gain access to this structure
    LONG   lThreadsWaiting;  // Count of threads waiting
    LONG   lAvailableCount;  // Available resource count
    LONG   lMaximumCount;    // Maximum resource count
}
METSECT_SHARED_INFO, *LPMETSECT_SHARED_INFO;

// The opaque Metered Section data structure
typedef struct _METERED_SECTION
{
    HANDLE hEvent;           // Handle to a kernel event object
    HANDLE hFileMap;         // Handle to memory mapped file
    LPMETSECT_SHARED_INFO lpSharedInfo;
}
METERED_SECTION, *LPMETERED_SECTION;

//---------------------------------------------------------------------------//
//
// Interface functions
//
//---------------------------------------------------------------------------//

LPMETERED_SECTION CreateMeteredSectionA(LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName);
LPMETERED_SECTION CreateMeteredSectionW(LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName);

#ifndef _WIN32_WCE
LPMETERED_SECTION OpenMeteredSectionA(LPCSTR lpName);
LPMETERED_SECTION OpenMeteredSectionW(LPCWSTR lpName);
#endif

DWORD EnterMeteredSection(LPMETERED_SECTION lpMetSect, DWORD dwMilliseconds);
BOOL LeaveMeteredSection(LPMETERED_SECTION lpMetSect, LONG lReleaseCount, LPLONG lpPreviousCount);
void CloseMeteredSection(LPMETERED_SECTION lpMetSect);

//---------------------------------------------------------------------------//
//
// Macros for ANSI/UNICODE switch
//
//---------------------------------------------------------------------------//

#if defined(_UNICODE) || defined(UNICODE)
    #define CreateMeteredSection CreateMeteredSectionW
#else
    #define CreateMeteredSection CreateMeteredSectionA
#endif

#ifndef _WIN32_WCE
#if defined(_UNICODE) || defined(UNICODE)
    #define OpenMeteredSection   OpenMeteredSectionW
#else
    #define OpenMeteredSection   OpenMeteredSectionA
#endif
#endif

//---------------------------------------------------------------------------//

#ifdef __cplusplus
}
#endif // __cplusplus

//---------------------------------------------------------------------------//

#endif // _METERED_SECTION_H_