/************************************************************
    Module Name: MeteredSection.c
    Original Author: Dan Chou
    Bug Fixed by: しゅう
    Bug Fixed by: JVR
    Modified  by: tapetums
    Description: Implements the metered section synchronization object

    See Also:
    http://msdn.microsoft.com/en-us/library/ms810428.aspx
    http://msdn.microsoft.com/ja-jp/library/cc429052.aspx
    http://anpcf.blog71.fc2.com/blog-entry-7.html
    http://s-project.appspot.com/notes_vc.html
    http://jibaravr.blog51.fc2.com/blog-category-19.html
************************************************************/

#include <windows.h>
#include <strsafe.h>

#include "MeteredSection.h"

//---------------------------------------------------------------------------//
//
// Internal function declarations
//
//---------------------------------------------------------------------------//

BOOL InitMeteredSection
(
    LPMETERED_SECTION lpMetSect,
    LONG              lInitialCount,
    LONG              lMaximumCount,
    LPCWSTR           lpName,
    BOOL              bOpenOnly
);
BOOL CreateMetSectEvent
(
    LPMETERED_SECTION lpMetSect,
    LPCWSTR           lpName,
    BOOL              bOpenOnly
);
BOOL CreateMetSectFileView
(
    LPMETERED_SECTION lpMetSect,
    LONG              lInitialCount,
    LONG              lMaximumCount,
    LPCWSTR           lpName,
    BOOL              bOpenOnly
);
void GetMeteredSectionLock
(
    LPMETERED_SECTION lpMetSect
);
void ReleaseMeteredSectionLock
(
    LPMETERED_SECTION lpMetSect
);

//---------------------------------------------------------------------------//
//
// Interface functions
//
//---------------------------------------------------------------------------//

LPMETERED_SECTION CreateMeteredSectionA
(
    LONG   lInitialCount,
    LONG   lMaximumCount,
    LPCSTR lpName
)
{
    WCHAR lpNameW[MAX_PATH];

    // lpName を MBCS から UTF-16 に変換
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpName, -1, lpNameW, MAX_PATH);

    return CreateMeteredSectionW(lInitialCount, lMaximumCount, lpNameW);
}

//---------------------------------------------------------------------------//

LPMETERED_SECTION CreateMeteredSectionW
(
    LONG    lInitialCount,
    LONG    lMaximumCount,
    LPCWSTR lpName
)
{
    LPMETERED_SECTION lpMetSect;

    // Verify the parameters
    if ((lMaximumCount < 1)             ||
        (lInitialCount > lMaximumCount) ||
        (lInitialCount < 0)             ||
        ((lpName) && (wcslen(lpName) > MAX_METSECT_NAMELEN)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    // Allocate memory for the metered section
    lpMetSect = (LPMETERED_SECTION)malloc(sizeof(METERED_SECTION));

    // If the memory for the metered section was allocated okay, initialize it
    if (lpMetSect)
    {
        if (!InitMeteredSection(lpMetSect, lInitialCount, lMaximumCount, lpName, FALSE))
        {
            CloseMeteredSection(lpMetSect);
            lpMetSect = NULL;
        }
    }
    return lpMetSect;
}

//---------------------------------------------------------------------------//

#ifndef _WIN32_WCE

//---------------------------------------------------------------------------//

LPMETERED_SECTION OpenMeteredSectionA
(
    LPCSTR lpName
)
{
    WCHAR lpNameW[MAX_PATH];

    // lpName を MBCS から UTF-16 に変換
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpName, -1, lpNameW, MAX_PATH);

    return OpenMeteredSectionW(lpNameW);
}

//---------------------------------------------------------------------------//

LPMETERED_SECTION OpenMeteredSectionW
(
    LPCWSTR lpName
)
{
    LPMETERED_SECTION lpMetSect = NULL;

    if (lpName)
    {
        lpMetSect = (LPMETERED_SECTION)malloc(sizeof(METERED_SECTION));

        // If the memory for the metered section was allocated okay
        if (lpMetSect)
        {
            if (!InitMeteredSection(lpMetSect, 0, 0, lpName, TRUE))
            {
                // Metered section failed to initialize
                CloseMeteredSection(lpMetSect);
                lpMetSect = NULL;
            }
        }
    }
    return lpMetSect;
}

//---------------------------------------------------------------------------//

#endif

//---------------------------------------------------------------------------//

DWORD EnterMeteredSection
(
    LPMETERED_SECTION lpMetSect,
    DWORD             dwMilliseconds
)
{
    while (TRUE)
    {
        GetMeteredSectionLock(lpMetSect);

        // We have access to the metered section, everything we do now will be atomic
        if (lpMetSect->lpSharedInfo->lAvailableCount >= 1)
        {
            lpMetSect->lpSharedInfo->lAvailableCount--;
            ReleaseMeteredSectionLock(lpMetSect);
            return WAIT_OBJECT_0;
        }

        // Couldn't get in. Wait on the event object
        lpMetSect->lpSharedInfo->lThreadsWaiting++;
        ResetEvent(lpMetSect->hEvent);
        ReleaseMeteredSectionLock(lpMetSect);
        if (WAIT_TIMEOUT == WaitForSingleObject(lpMetSect->hEvent, dwMilliseconds))
        {
            return WAIT_TIMEOUT;
        }
    }
}

//---------------------------------------------------------------------------//

BOOL LeaveMeteredSection
(
    LPMETERED_SECTION lpMetSect,
    LONG              lReleaseCount,
    LPLONG            lpPreviousCount
)
{
    int iCount;
    GetMeteredSectionLock(lpMetSect);

    // Save the old value if they want it
    if (lpPreviousCount)
    {
        *lpPreviousCount = lpMetSect->lpSharedInfo->lAvailableCount;
    }

    // We have access to the metered section, everything we do now will be atomic
    if ((lReleaseCount < 0) ||
        (lpMetSect->lpSharedInfo->lAvailableCount+lReleaseCount >
        lpMetSect->lpSharedInfo->lMaximumCount))
    {
        ReleaseMeteredSectionLock(lpMetSect);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    lpMetSect->lpSharedInfo->lAvailableCount += lReleaseCount;

    // Set the event the appropriate number of times
    lReleaseCount =  min(lReleaseCount,lpMetSect->lpSharedInfo->lThreadsWaiting);
    if (lpMetSect->lpSharedInfo->lThreadsWaiting)
    {
        for (iCount = 0; iCount < lReleaseCount; iCount++)
        {
            lpMetSect->lpSharedInfo->lThreadsWaiting--;
        /// SetEvent(lpMetSect->hEvent);
        }
/// }
/// ReleaseMeteredSectionLock(lpMetSect);
/// ここから追加
        ReleaseMeteredSectionLock(lpMetSect);
        SetEvent(lpMetSect->hEvent);
    }
    else
    {
        ReleaseMeteredSectionLock(lpMetSect);
    }
/// ここまで追加
    return TRUE;
}

//---------------------------------------------------------------------------//

void CloseMeteredSection
(
    LPMETERED_SECTION lpMetSect
)
{
    if (lpMetSect)
    {
        // Clean up
        if (lpMetSect->lpSharedInfo) UnmapViewOfFile(lpMetSect->lpSharedInfo);
        if (lpMetSect->hFileMap)     CloseHandle(lpMetSect->hFileMap);
        if (lpMetSect->hEvent)       CloseHandle(lpMetSect->hEvent);
        free(lpMetSect);
    }
}

//---------------------------------------------------------------------------//

BOOL InitMeteredSection
(
    LPMETERED_SECTION lpMetSect,
    LONG              lInitialCount,
    LONG              lMaximumCount,
    LPCWSTR           lpName,
    BOOL              bOpenOnly
)
{
/// ここから追加
    lpMetSect->hEvent       = NULL;
    lpMetSect->hFileMap     = NULL;
    lpMetSect->lpSharedInfo = NULL;
/// ここまで追加

    // Try to create the event object
    if (CreateMetSectEvent(lpMetSect, lpName, bOpenOnly))
    {
        // Try to create the memory mapped file
        if (CreateMetSectFileView(lpMetSect, lInitialCount, lMaximumCount, lpName, bOpenOnly))
        {
            return TRUE;
        }
    }

    // Error occured, return FALSE so the caller knows to clean up
    return FALSE;
}

//---------------------------------------------------------------------------//

BOOL CreateMetSectEvent
(
    LPMETERED_SECTION lpMetSect,
    LPCWSTR           lpName,
    BOOL              bOpenOnly
)
{
    WCHAR sz[MAX_PATH];
    if (lpName)
    {
    /// _stprintf(sz, _T("DKC_MSECT_EVT_%s"), lpName);
        StringCchPrintfW(sz, MAX_PATH, L"DKC_MSECT_EVT_%s", lpName);

#ifndef _WIN32_WCE
        if (bOpenOnly)
        {
        /// lpMetSect->hEvent = OpenEventW(0, FALSE, sz);
            lpMetSect->hEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, sz);
        }
        else
        {
#endif
            // Create an auto-reset named event object
            lpMetSect->hEvent = CreateEventW(NULL, FALSE, FALSE, sz);
#ifndef _WIN32_WCE
        }
#endif
    }
    else
    {
        // Create an auto-reset unnamed event object
        lpMetSect->hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    }
    return (lpMetSect->hEvent ? TRUE : FALSE);
}

//---------------------------------------------------------------------------//

BOOL CreateMetSectFileView
(
    LPMETERED_SECTION lpMetSect,
    LONG              lInitialCount,
    LONG              lMaximumCount,
    LPCWSTR           lpName,
    BOOL              bOpenOnly
)
{
    WCHAR sz[MAX_PATH];
    DWORD dwLastError; 

    if (lpName)
    {
    /// _stprintf(sz, _T("DKC_MSECT_MMF_%s"), lpName);
        StringCchPrintfW(sz, MAX_PATH, L"DKC_MSECT_MMF_%s", lpName);

#ifndef _WIN32_WCE
        if (bOpenOnly)
        {
        /// lpMetSect->hFileMap = OpenFileMappingW(0, FALSE, sz);
            lpMetSect->hFileMap = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, sz);
        }
        else
        {
#endif
            // Create a named file mapping
            lpMetSect->hFileMap = CreateFileMappingW
            (
                INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                0, sizeof(METSECT_SHARED_INFO), sz
            );
#ifndef _WIN32_WCE
        }
#endif
    }
    else
    {
        // Create an unnamed file mapping
        lpMetSect->hFileMap = CreateFileMappingW
        (
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            0, sizeof(METSECT_SHARED_INFO), NULL
        );
    }

    // Map a view of the file
    if (lpMetSect->hFileMap)
    {
        dwLastError = GetLastError();

        lpMetSect->lpSharedInfo = (LPMETSECT_SHARED_INFO)MapViewOfFile
        (
            lpMetSect->hFileMap, FILE_MAP_WRITE, 0, 0, 0
        );
        if (lpMetSect->lpSharedInfo)
        {
            if (!bOpenOnly && dwLastError != ERROR_ALREADY_EXISTS)
            {
                lpMetSect->lpSharedInfo->lSpinLock       = 0;
                lpMetSect->lpSharedInfo->lThreadsWaiting = 0;
                lpMetSect->lpSharedInfo->lAvailableCount = lInitialCount;
                lpMetSect->lpSharedInfo->lMaximumCount   = lMaximumCount;
                InterlockedExchange(&(lpMetSect->lpSharedInfo->fInitialized), TRUE);
            }
            else
            {    // Already exists; wait for it to be initialized by the creator
                while (!lpMetSect->lpSharedInfo->fInitialized) Sleep(0);
            }
            return TRUE;
        }
    }
    return FALSE;
}

//---------------------------------------------------------------------------//

void GetMeteredSectionLock
(
    LPMETERED_SECTION lpMetSect
)
{
    // Spin and get access to the metered section lock
    while (InterlockedExchange(&(lpMetSect->lpSharedInfo->lSpinLock), 1) != 0)
    {
        Sleep(0);
    }
}

//---------------------------------------------------------------------------//

void ReleaseMeteredSectionLock
(
    LPMETERED_SECTION lpMetSect
)
{
    InterlockedExchange(&(lpMetSect->lpSharedInfo->lSpinLock), 0);
}

//---------------------------------------------------------------------------//
