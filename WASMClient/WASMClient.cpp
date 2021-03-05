// WASMClient.cpp : Defines the entry point for the application.
//
#define WIN32_LEAN_AND_MEAN

#include "framework.h"
#include <Windows.h>
#include <wchar.h>
#include "WASMClient.h"
#include "WASMIF.h"
#include "Default.h"

#define MAX_LOADSTRING 100

// Logging function. We'll use FSUIPCs logging function for this (eventually!)
extern void MyWriteLog(const char* fmt, ...);

// Global Variables:
HINSTANCE hInst;                                // current instance
CHAR szTitle[MAX_LOADSTRING];                  // The title bar text
CHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
ULONGLONG systemStartTime;
int     quit = 0;
WASMIF* wasmPtr = nullptr;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    GetLvar(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SetLvar(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SetHvar(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ExecCCode(HWND, UINT, WPARAM, LPARAM);
// Hook for centering MessageBox on parent window rather than screen 
HHOOK hCBTHook = 0;
LRESULT CALLBACK CBTMsgBox(int, WPARAM, LPARAM);
void CenterPosition(HWND);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    systemStartTime = GetTickCount64();

    // Initialize global strings
    LoadStringA(hInstance, IDS_APP_TITLE, (LPSTR)&szTitle, MAX_LOADSTRING);
    LoadStringA(hInstance, IDC_WASMCLIENT, (LPSTR)&szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WASMCLIENT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WASMCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_WASMCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 600, 200, 0, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

/*
 * Example of a wrapper logging function: use when you want to delagate to
 * your existing logger rather than using the built-in one.
 * This example simply delegates to the the MyLogError function (see MyLogStuff.cpp).
 * which does not have any levels so this is ignored, but you can check the
 * first characters of the string to determine the level if needed
 */
void MyLogFunction(const char* logString)
{
    if (strstr(logString, "[INFO]")) {
        // We can strip the log-level from the string and use the underlying
        // providers log-level here
        MyWriteLog(logString);
    }
    else if (strstr(logString, "[INFO]")) {
        MyWriteLog(logString);
    }
    else if (strstr(logString, "[DEBUG]")) {
        MyWriteLog(logString);
    }
    else if (strstr(logString, "[TRACE]")) {
        MyWriteLog(logString);
    }
    else if (strstr(logString, "[ALWAYS]")) {
        MyWriteLog(logString);
    }
    else if (strstr(logString, "[ERROR]")) {
        MyWriteLog(logString);
    }
    else if (strstr(logString, "[ALARM]")) {
        MyWriteLog(logString);
    }
    else
        MyWriteLog(logString);
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_FILE_START:
            {
                HMENU hMenu = GetMenu(hWnd);
                // Set-up defaults.
                Default* defaults = Default::GetInstance("WASMClient");

                int startEventNo = defaults->GetStartEventNo();
                if (!startEventNo) startEventNo = EVENT_START_NO;
                int lvarUpdateFrequecy = defaults->GetLvarUpdateFrequency(); // 6Hz (default)
                LogLevel logLevel = defaults->GetLogLevel();

                if (!wasmPtr) wasmPtr = WASMIF::GetInstance(hWnd);
//              if (!wasmPtr) wasmPtr = WASMIF::GetInstance(hWnd, &MyLogFunction);
//              if (!wasmPtr) wasmPtr = WASMIF::GetInstance(hWnd, EVENT_START_NO, &MyLogFunction);
                wasmPtr->setLogLevel(logLevel);
                if (lvarUpdateFrequecy)
                    wasmPtr->setLvarUpdateFrequency(lvarUpdateFrequecy);
                if (wasmPtr->start()) {
                    EnableMenuItem(hMenu, ID_FILE_STOP, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_FILE_START, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_GETLVAR, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_SETLVAR, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_LISTLVARS, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_RELOADLVARS, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_CREATE_LVARFILE, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_SETHVAR, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_LISTHVARS, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_CONTROL_EXEC_CCODE, MF_BYCOMMAND | MF_ENABLED);
                }
                break;
            }
            case ID_FILE_STOP:
            {
                HMENU hMenu = GetMenu(hWnd);
                if (wasmPtr) wasmPtr->end();
                wasmPtr = nullptr;
                EnableMenuItem(hMenu, ID_FILE_STOP, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_FILE_START, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_CONTROL_GETLVAR, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_CONTROL_SETLVAR, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_CONTROL_LISTLVARS, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_CONTROL_RELOADLVARS, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_CONTROL_CREATE_LVARFILE, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_CONTROL_SETHVAR, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_CONTROL_LISTHVARS, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_CONTROL_EXEC_CCODE, MF_BYCOMMAND | MF_DISABLED);
                break;
            }
            case ID_CONTROL_EXEC_CCODE:
                // Display Execute Calculator Code Dialog
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EXEC_CCODE), hWnd, ExecCCode);
                break;
            case ID_CONTROL_GETLVAR:
                // Display Get LVAR Dialog
                DialogBox(hInst, MAKEINTRESOURCE(IDD_GETLVAR), hWnd, GetLvar);
                break;

                break;
            case ID_CONTROL_SETLVAR:
                // Display Set LVAR Dialog
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETLVAR), hWnd, SetLvar);
                break;
            case ID_CONTROL_LISTLVARS:
                if (wasmPtr) wasmPtr->listLvars();
                break;
            case ID_CONTROL_RELOADLVARS:
                if (wasmPtr) wasmPtr->reload();
                break;
            case ID_CONTROL_CREATE_LVARFILE:
                if (wasmPtr) wasmPtr->createAircraftLvarFile();
                break;
            case ID_CONTROL_SETHVAR:
                // Display Set LVAR Dialog
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETHVAR), hWnd, SetHvar);
                break;
            case ID_CONTROL_LISTHVARS:
                if (wasmPtr) wasmPtr->listHvars();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for Get Lvar dialog box.
INT_PTR CALLBACK GetLvar(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char logBuffer[512];

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: {
        // Populate Combo Boxes
        if (wasmPtr) {
            char A[MAX_VAR_NAME_SIZE+6];
            unordered_map<int, string > lvars;
            wasmPtr->getLvarList(lvars);
            for (int i = 0; i < lvars.size(); i++) {
               sprintf(A, "%03d - %s", i, lvars.at(i).c_str());
               SendMessage(GetDlgItem(hDlg, IDC_GETLVAR_COMBO1), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
               SendMessage(GetDlgItem(hDlg, IDC_GETLVAR_COMBO2), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
            }
            // Send the CB_SETCURSEL message to display an initial item in the selection field  
            SendMessage(GetDlgItem(hDlg, IDC_GETLVAR_COMBO1), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(GetDlgItem(hDlg, IDC_GETLVAR_COMBO2), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
           case ID_GETLVAR_CANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
          case IDC_GETLVAR_BYID: {
              HWND hCombo = GetDlgItem(hDlg, IDC_GETLVAR_COMBO1);
              int nIndex = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
              char selection[MAX_VAR_NAME_SIZE + 6];
              SendMessage(hCombo, CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)selection);
              // Id is in first 3 characters
              selection[3] = 0;
              int selectedId = atoi(selection);
              double value = wasmPtr->getLvar(selectedId);
              sprintf(logBuffer, "LVAR by id %d: %f", selectedId, value);
              hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
              MessageBox(hDlg, logBuffer, "LVARE value by ID", MB_OK | MB_ICONINFORMATION);
              UnhookWindowsHookEx(hCBTHook);
//              LOG_DEBUG(logBuffer);
            break;
          }
          case IDC_GETLVAR_BYNAME: {
              HWND hCombo = GetDlgItem(hDlg, IDC_GETLVAR_COMBO2);
              int nIndex = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
              char selection[MAX_VAR_NAME_SIZE + 6];
              char lvarName[MAX_VAR_NAME_SIZE];
              SendMessage(hCombo, CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)selection);
              // Remove Id in first 6 characters
              strcpy(lvarName, selection + 6);
              double value = wasmPtr->getLvar(lvarName);
              sprintf(logBuffer, "LVAR by name %s: %f", lvarName, value);
              hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
              MessageBox(hDlg, logBuffer, "LVAR value by Name", MB_OK | MB_ICONINFORMATION);
              UnhookWindowsHookEx(hCBTHook);

//              LOG_DEBUG(logBuffer);
              break;
          }
        }
    }
    }
    return (INT_PTR)FALSE;
}

// Message handler for Set Lvar dialog box.
INT_PTR CALLBACK SetLvar(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    char logBuffer[512];
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG: {
            // Populate Combo Box
            if (wasmPtr) {
                char A[MAX_VAR_NAME_SIZE + 6];
                unordered_map<int, string > lvars;
                wasmPtr->getLvarList(lvars);
                for (int i = 0; i < lvars.size(); i++) {
                    sprintf(A, "%03d - %s", i, lvars.at(i).c_str());
                    SendMessage(GetDlgItem(hDlg, IDC_SETLVAR_COMBO), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
                }
                // Send the CB_SETCURSEL message to display an initial item in the selection field  
                SendMessage(GetDlgItem(hDlg, IDC_SETLVAR_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
            }
            return (INT_PTR)TRUE;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_SETLVAR_DONE:
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR)TRUE;
        
                case IDC_SETLVAR_GO: {
                    HWND hCombo = GetDlgItem(hDlg, IDC_SETLVAR_COMBO);
                    int nIndex = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                    char selection[MAX_VAR_NAME_SIZE + 6];
                    SendMessage(hCombo, CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)selection);
                    // Id is in first 3 characters
                    selection[3] = 0;
                    short selectedId = atoi(selection);
                    HWND hEdit = GetDlgItem(hDlg, IDC_SETLVAR_VALUE);
                    char valueBuffer[64];

                    GetWindowText(hEdit, valueBuffer, 64); valueBuffer[63] = '\0';
                    sprintf(logBuffer, "Setting lvar value as string: '%s'", valueBuffer);
                    LOG_DEBUG(logBuffer);
                    wasmPtr->setLvar(selectedId, valueBuffer);
                    break;
                }
            }
        }
    }
    return (INT_PTR)FALSE;
}

// Message handler for Set Hvar dialog box.
INT_PTR CALLBACK SetHvar(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: {
        // Populate Combo Box
        if (wasmPtr) {
            char A[MAX_VAR_NAME_SIZE + 6];
            unordered_map<int, string > hvars;
            wasmPtr->getHvarList(hvars);
            for (int i = 0; i < hvars.size(); i++) {
                sprintf(A, "%03d - %s", i, hvars.at(i).c_str());
                SendMessage(GetDlgItem(hDlg, IDC_SETHVAR_COMBO), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
            }
            // Send the CB_SETCURSEL message to display an initial item in the selection field  
            SendMessage(GetDlgItem(hDlg, IDC_SETHVAR_COMBO), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
        case ID_SETHVAR_DONE:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case IDC_SETHVAR_GO: {
            HWND hCombo = GetDlgItem(hDlg, IDC_SETHVAR_COMBO);
            int nIndex = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
            char selection[MAX_VAR_NAME_SIZE + 6];
            SendMessage(hCombo, CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)selection);
            // Id is in first 3 characters
            selection[3] = 0;
            short selectedId = atoi(selection);
            wasmPtr->setHvar(selectedId);
            break;
        }
        }
    }
    }
    return (INT_PTR)FALSE;
}

// Message handler for Set Hvar dialog box.
INT_PTR CALLBACK ExecCCode(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
        {
            return (INT_PTR)TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_CCODE_CANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                 return (INT_PTR)TRUE;
                case IDC_EXECUTE:
                {
                    HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_CCODE);
                    char ccode[MAX_CALC_CODE_SIZE];

                    GetWindowText(hEdit, ccode, MAX_CALC_CODE_SIZE);

                    wasmPtr->executeCalclatorCode(ccode);

                    SetWindowText(hEdit, _T(""));
                    break;
                }
            }
        }
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK CBTMsgBox(int code, WPARAM wParam, LPARAM lParam)
{
    if (HCBT_ACTIVATE != code || -1 == code)
    {
        // only when HCBT_ACTIVATE 
        return CallNextHookEx(hCBTHook, code, wParam, lParam);
    }

    HWND hWnd = (HWND)wParam;
    UnhookWindowsHookEx(hCBTHook);
    CenterPosition(hWnd);

    return CallNextHookEx(hCBTHook, code, wParam, lParam);
}

void CenterPosition(HWND hDlg)
{
    RECT rcDlg;
    RECT rcOwner;

    //center
    HWND hWndOwner = ::GetWindow(hDlg, GW_OWNER);
    GetWindowRect(hDlg, &rcDlg);
    GetWindowRect(hWndOwner, &rcOwner);

    int iLeft = rcOwner.left + ((rcOwner.right - rcOwner.left)
        - (rcDlg.right - rcDlg.left)) / 2;

    int iTop = rcOwner.top + ((rcOwner.bottom - rcOwner.top)
        - (rcDlg.bottom - rcDlg.top)) / 2;

    SetWindowPos(hDlg, hWndOwner, iLeft, iTop, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED);
}