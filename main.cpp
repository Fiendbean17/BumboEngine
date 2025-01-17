// Exclude the min and max macros from Windows.h
#define NOMINMAX
#include "stdafx.h"
#include "resource.h"
#include "WinMainParameters.h"
#include "MatrixManager.h"
#include "SplashScreen.h"

using namespace WinMainParameters;

#define MAX_LOADSTRING 100
#define BUFSIZE 4096
TCHAR buf[MAX_PATH];

// Global Variables (are (usually) evil; this code comes from the Visual C++ Win32 Application project template):
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Global (To main.cpp only) Variables for width, height and the output display screens
static bool should_exit_G = false;
static int width_G = 79;
static int height_G = 35;
static BitmapDefinition bitmap_G(0, 160, 0);
static Matrix screen_matrix_G(width_G, height_G);

void createDirectory(HINSTANCE hInstance, TCHAR directory[MAX_PATH]);
void createMP3File(HINSTANCE hInstance, TCHAR directory[MAX_PATH], TCHAR file_name[MAX_PATH], int MP3_ID);
void deleteMP3File(TCHAR directory[MAX_PATH], TCHAR file_name[MAX_PATH]);
void deleteDirectory(TCHAR directory[MAX_PATH]);

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LAUNCHWIN32WINDOWFROMCONSOLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAUNCHWIN32WINDOWFROMCONSOLE));

	// Create Temporary Directory for audio files
	if (GetTempPath(MAX_PATH, buf) == 0)
		MessageBox(0, buf, _T("Failed to create audio"), 0);
	TCHAR directory[MAX_PATH];
	TCHAR folder_name[MAX_PATH] = L"\Wenlife\\";
	_stprintf_s(directory, MAX_PATH, _T("%s%s"), buf, folder_name);
	CreateDirectory(directory, NULL);
	std::wstring directory_w = directory; // w_string version of directory

	createDirectory(hInstance, directory);

	// Loading/Splash Screen
	SplashScreen splash(width_G, height_G, screen_matrix_G);
	GetMessage(&msg, NULL, 0, 0);
	RedrawWindow(msg.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN |
		RDW_ERASE | RDW_NOFRAME | RDW_UPDATENOW);

	// MAIN SOURCE PORT - Bumbo Engine -----------------------------------------------------
	MatrixManager grid(width_G, height_G, screen_matrix_G, 5, bitmap_G, std::string(directory_w.begin(), directory_w.end()));
	GetMessage(&msg, NULL, 0, 0);
	RedrawWindow(msg.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN |
		RDW_ERASE | RDW_NOFRAME | RDW_UPDATENOW);

	// Game loop
	while (true)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (should_exit_G || msg.message == WM_QUIT) { break; }

		// Update the game (loop)
		grid.evaluatePlayerInput();
		RedrawWindow(msg.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN |
			RDW_ERASE | RDW_NOFRAME | RDW_UPDATENOW);
	}

	deleteDirectory(directory);

	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAUNCHWIN32WINDOWFROMCONSOLE));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_LAUNCHWIN32WINDOWFROMCONSOLE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;

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
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	RECT wr = { 0, 0, 795, 545 };    // set the window size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the window size
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) \
                                 | ((Color << 16) & 0xff0000)

//-------------------------------------------------------------------------------
// ReplaceColor
//
// Author    : Dimitri Rochette drochette@coldcat.fr
// Specials Thanks to Joe Woodbury for his comments and code corrections
//
// Includes  : Only <windows.h>

//
// hBmp         : Source Bitmap
// cOldColor : Color to replace in hBmp
// cNewColor : Color used for replacement
// hBmpDC    : DC of hBmp ( default NULL ) could be NULL if hBmp is not selected
//
// Retcode   : HBITMAP of the modified bitmap or NULL for errors
//
//-------------------------------------------------------------------------------
HBITMAP ReplaceColor(HBITMAP hBmp, COLORREF cOldColor, COLORREF cNewColor, HDC hBmpDC)
{
	HBITMAP RetBmp = NULL;
	if (hBmp)
	{
		HDC BufferDC = CreateCompatibleDC(NULL);    // DC for Source Bitmap
		if (BufferDC)
		{
			HBITMAP hTmpBitmap = (HBITMAP)NULL;
			if (hBmpDC)
				if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
				{
					hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
					SelectObject(hBmpDC, hTmpBitmap);
				}

			HGDIOBJ PreviousBufferObject = SelectObject(BufferDC, hBmp);
			// here BufferDC contains the bitmap

			HDC DirectDC = CreateCompatibleDC(NULL); // DC for working
			if (DirectDC)
			{
				// Get bitmap size
				BITMAP bm;
				GetObject(hBmp, sizeof(bm), &bm);

				// create a BITMAPINFO with minimal initilisation 
				// for the CreateDIBSection
				BITMAPINFO RGB32BitsBITMAPINFO;
				ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
				RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				RGB32BitsBITMAPINFO.bmiHeader.biWidth = bm.bmWidth;
				RGB32BitsBITMAPINFO.bmiHeader.biHeight = bm.bmHeight;
				RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
				RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

				// pointer used for direct Bitmap pixels access
				UINT * ptPixels;

				HBITMAP DirectBitmap = CreateDIBSection(DirectDC,
					(BITMAPINFO *)&RGB32BitsBITMAPINFO,
					DIB_RGB_COLORS,
					(void **)&ptPixels,
					NULL, 0);
				if (DirectBitmap)
				{
					// here DirectBitmap!=NULL so ptPixels!=NULL no need to test
					HGDIOBJ PreviousObject = SelectObject(DirectDC, DirectBitmap);
					BitBlt(DirectDC, 0, 0,
						bm.bmWidth, bm.bmHeight,
						BufferDC, 0, 0, SRCCOPY);

					// here the DirectDC contains the bitmap

					// Convert COLORREF to RGB (Invert RED and BLUE)
					cOldColor = COLORREF2RGB(cOldColor);
					cNewColor = COLORREF2RGB(cNewColor);

					// After all the inits we can do the job : Replace Color
					for (int i = ((bm.bmWidth*bm.bmHeight) - 1); i >= 0; i--)
					{
						if (ptPixels[i] == cOldColor) ptPixels[i] = cNewColor;
					}
					// little clean up
					// Don't delete the result of SelectObject because it's 
					// our modified bitmap (DirectBitmap)
					SelectObject(DirectDC, PreviousObject);

					// finish
					RetBmp = DirectBitmap;
				}
				// clean up
				DeleteDC(DirectDC);
			}
			if (hTmpBitmap)
			{
				SelectObject(hBmpDC, hBmp);
				DeleteObject(hTmpBitmap);
			}
			SelectObject(BufferDC, PreviousBufferObject);
			// BufferDC is now useless
			DeleteDC(BufferDC);
		}
	}
	return RetBmp;
}

HBITMAP ReplaceAllColorsExcept(HBITMAP hBmp, COLORREF cExcludedColor, COLORREF cNewColor, HDC hBmpDC)
{
	HBITMAP RetBmp = NULL;
	if (hBmp)
	{
		HDC BufferDC = CreateCompatibleDC(NULL);    // DC for Source Bitmap
		if (BufferDC)
		{
			HBITMAP hTmpBitmap = (HBITMAP)NULL;
			if (hBmpDC)
				if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
				{
					hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
					SelectObject(hBmpDC, hTmpBitmap);
				}

			HGDIOBJ PreviousBufferObject = SelectObject(BufferDC, hBmp);
			// here BufferDC contains the bitmap

			HDC DirectDC = CreateCompatibleDC(NULL); // DC for working
			if (DirectDC)
			{
				// Get bitmap size
				BITMAP bm;
				GetObject(hBmp, sizeof(bm), &bm);

				// create a BITMAPINFO with minimal initilisation 
				// for the CreateDIBSection
				BITMAPINFO RGB32BitsBITMAPINFO;
				ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
				RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				RGB32BitsBITMAPINFO.bmiHeader.biWidth = bm.bmWidth;
				RGB32BitsBITMAPINFO.bmiHeader.biHeight = bm.bmHeight;
				RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
				RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

				// pointer used for direct Bitmap pixels access
				UINT * ptPixels;

				HBITMAP DirectBitmap = CreateDIBSection(DirectDC,
					(BITMAPINFO *)&RGB32BitsBITMAPINFO,
					DIB_RGB_COLORS,
					(void **)&ptPixels,
					NULL, 0);
				if (DirectBitmap)
				{
					// here DirectBitmap!=NULL so ptPixels!=NULL no need to test
					HGDIOBJ PreviousObject = SelectObject(DirectDC, DirectBitmap);
					BitBlt(DirectDC, 0, 0,
						bm.bmWidth, bm.bmHeight,
						BufferDC, 0, 0, SRCCOPY);

					// here the DirectDC contains the bitmap

					// Convert COLORREF to RGB (Invert RED and BLUE)
					cExcludedColor = COLORREF2RGB(cExcludedColor);
					cNewColor = COLORREF2RGB(cNewColor);

					// After all the inits we can do the job : Replace Color
					for (int i = ((bm.bmWidth*bm.bmHeight) - 1); i >= 0; i--)
					{
						if (ptPixels[i] != cExcludedColor) ptPixels[i] = cNewColor;
					}
					// little clean up
					// Don't delete the result of SelectObject because it's 
					// our modified bitmap (DirectBitmap)
					SelectObject(DirectDC, PreviousObject);

					// finish
					RetBmp = DirectBitmap;
				}
				// clean up
				DeleteDC(DirectDC);
			}
			if (hTmpBitmap)
			{
				SelectObject(hBmpDC, hBmp);
				DeleteObject(hTmpBitmap);
			}
			SelectObject(BufferDC, PreviousBufferObject);
			// BufferDC is now useless
			DeleteDC(BufferDC);
		}
	}
	return RetBmp;
}

// Creates Directory
void createDirectory(HINSTANCE hInstance, TCHAR directory[MAX_PATH]) {

	// Names don't actually matter.. could be "Hello.mp3" just there for readability
	createMP3File(hInstance, directory, L"watchtower.mp3", 152);
	createMP3File(hInstance, directory, L"achilles.mp3", 153);
	createMP3File(hInstance, directory, L"wind.mp3", 156);
	createMP3File(hInstance, directory, L"major.mp3", 157);
}

// Creates an MP3 file in the provided directory
void createMP3File(HINSTANCE hInstance, TCHAR directory[MAX_PATH], TCHAR file_name[MAX_PATH], int MP3_ID) {
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(MP3_ID), _T("MP3"));
	HGLOBAL hRes = ::LoadResource(hInstance, hResInfo);
	LPVOID memRes = ::LockResource(hRes);
	DWORD sizeRes = ::SizeofResource(hInstance, hResInfo);

	TCHAR file_path[MAX_PATH] = L"";
	_stprintf_s(file_path, MAX_PATH, _T("%s%s"), directory, file_name);

	HANDLE hFile = ::CreateFile(file_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwWritten = 0;
	::WriteFile(hFile, memRes, sizeRes, &dwWritten, NULL);
	::CloseHandle(hFile);
}

// Deletes MP3 File from provided directory
void deleteMP3File(TCHAR directory[MAX_PATH], TCHAR file_name[MAX_PATH]) {
	TCHAR file_path[MAX_PATH] = L"";
	_stprintf_s(file_path, MAX_PATH, _T("%s%s"), directory, file_name);

	DeleteFile(file_path);
}

void deleteDirectory(TCHAR directory[MAX_PATH])
{
	deleteMP3File(directory, L"Bee_Gees_-_Stayin_Alive.mp3");
	deleteMP3File(directory, L"Earth_Wind_and_Fire_-_September.mp3");
	deleteMP3File(directory, L"Guns_n_Roses_-_Nightrain.mp3");
	deleteMP3File(directory, L"Initial D - Deja Vu.mp3");
	deleteMP3File(directory, L"James_Brown_-_Sex_Machine.mp3");
	deleteMP3File(directory, L"Jimi_Hendrix_-_All_Along_the_Watchtower.mp3");
	deleteMP3File(directory, L"Led_Zeppelin_-_Achilles_Last_Stand.mp3");
	deleteMP3File(directory, L"Led_Zeppelin_-_Immigrant_Song.mp3");
	deleteMP3File(directory, L"Men_At_Work_-_Who_Can_It_Be_Now.mp3");
	deleteMP3File(directory, L"Neil_Diamond_-_Solitary_Man.mp3");
	deleteMP3File(directory, L"Peter_Schilling_-_Major_Tom.mp3");
	deleteMP3File(directory, L"Pink_Floyd_-_Another_Brick_in_the_Wall.mp3");
	deleteMP3File(directory, L"The_Police_-_Message_in_a_Bottle.mp3");
	deleteMP3File(directory, L"The_Who_-_Behind_Blue_Eyes.mp3");
	deleteMP3File(directory, L"Van_Halen_-_Ain't_Talkin_Bout_Love.mp3");
	deleteMP3File(directory, L"falling.mp3");
	deleteMP3File(directory, L"step.mp3");
	RemoveDirectory(directory);
}

bool LoadAndBlitBitmap(int resource_ID, HDC hWinDC, int position_x, int position_y)
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	// Load the bitmap image file
	HBITMAP hBitmap;
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(resource_ID));
	// Verify that the image was loaded
	if (hBitmap == NULL)
	{
		::MessageBox(NULL, __T("LoadImage Failed"), __T("Error"), MB_OK);
		return false;
	}

	// Create a device context that is compatible with the window
	HDC hLocalDC;
	hLocalDC = ::CreateCompatibleDC(hWinDC);
	// Verify that the device context was created
	if (hLocalDC == NULL)
	{
		::MessageBox(NULL, __T("CreateCompatibleDC Failed"), __T("Error"), MB_OK);
		return false;
	}

	// Changes color (But since Bitmap is so pixelated, this is the only way we can include all colors
	HBITMAP hBitmapColored = bitmap_G.usesOriginalColors() ? hBitmap : ReplaceAllColorsExcept(hBitmap, 0x000000, bitmap_G.getRGBA().getHex(), hLocalDC);

	// Get the bitmap's parameters and verify the get
	BITMAP qBitmap;
	int iReturn = GetObject(reinterpret_cast<HGDIOBJ>(hBitmapColored), sizeof(BITMAP),
		reinterpret_cast<LPVOID>(&qBitmap));
	if (!iReturn)
	{
		::MessageBox(NULL, __T("GetObject Failed"), __T("Error"), MB_OK);
		return false;
	}

	// Select the loaded bitmap into the device context
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hLocalDC, hBitmapColored);
	if (hOldBmp == NULL)
	{
		::MessageBox(NULL, __T("SelectObject Failed"), __T("Error"), MB_OK);
		return false;
	}

	/*if (std::get<2>(bitmap_G).getHex() < 255)
	{
		// setting up the blend function
		BLENDFUNCTION bStruct;
		bStruct.BlendOp = AC_SRC_OVER;
		bStruct.BlendFlags = 0;
		bStruct.SourceConstantAlpha = std::get<2>(bitmap_G).getHex();
		bStruct.AlphaFormat = 0;

		// Blit a transparent version of the dc onto the window's dc
		BOOL qTransBlit = ::AlphaBlend(hWinDC, position_x, 0, 475, 425, hLocalDC, 0, 0, qBitmap.bmWidth, qBitmap.bmHeight, bStruct);
		if (!qTransBlit)
		{
			::MessageBox(NULL, __T("Blit Failed"), __T("Error"), MB_OK);
			return false;
		}
	}
	else
	{//*/
	// Blit the dc which holds the bitmap onto the window's dc
	SetStretchBltMode(hWinDC, HALFTONE);
	BOOL qRetBlit = ::StretchBlt(hWinDC, position_x, position_y, 475, 425,
		hLocalDC, 0, 0, qBitmap.bmWidth, qBitmap.bmHeight, SRCCOPY);
	if (!qRetBlit)
	{
		::MessageBox(NULL, __T("Blit Failed"), __T("Error"), MB_OK);
		return false;
	}
	//}

	// Unitialize and deallocate resources
	::SelectObject(hLocalDC, hOldBmp);
	::DeleteDC(hLocalDC);
	::DeleteObject(hBitmap);
	::DeleteObject(hBitmapColored);
	return true;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HFONT font;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		if (hDC != NULL) {
			RECT rect;
			GetClientRect(hWnd, &rect);

			// Create bitmap image (To write everything on the window to)
			HDC hDCMem = CreateCompatibleDC(hDC);
			HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
			SelectObject(hDCMem, hBitmap);

			FillRect(hDCMem, &rect, (HBRUSH)(COLOR_BTNFACE + 1));
			FillRect(hDCMem, &rect, (HBRUSH)(COLOR_BACKGROUND + 1));

			// Load Complex Ascii-styled Image from provided file path
			if (bitmap_G.shouldShowBitmap())
				LoadAndBlitBitmap(bitmap_G.getResourceID(), hDCMem, bitmap_G.getXOffset(), bitmap_G.getYOffset());

			COLORREF blackTextColor = 0x00000000;
			SetBkMode(hDCMem, OPAQUE);
			SetBkColor(hDCMem, blackTextColor);
			for (int i = 0; i < height_G; i++)
				for (int j = 0; j < width_G; j++)
					if (screen_matrix_G[i][j] != ' ')
					{
						COLORREF whiteTextColor = screen_matrix_G[i][j].getColor();
						if (SetTextColor(hDCMem, whiteTextColor) == CLR_INVALID)
						{
							PostQuitMessage(1);
						}
						char c = screen_matrix_G[i][j];
						ExtTextOutA(hDCMem, j * 10, i * 15, ETO_CLIPPED, &rect, &c, 1, NULL);
					}

			// Copy window image/bitmap to screen
			BitBlt(hDC, 0, 0, rect.right, rect.bottom, hDCMem, 0, 0, SRCCOPY);

			DeleteObject(hBitmap);
			DeleteDC(hDCMem);

			EndPaint(hWnd, &ps);
		}
	} return 0;

	case WM_ERASEBKGND:
		return 1;
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		should_exit_G = true;
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