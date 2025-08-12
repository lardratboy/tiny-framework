// tiny_framework.cpp : Defines the entry point for the application.
//
// LICENSE
//
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tiny_framework.h"
#include "TinyProgram.h"
#include "BPTDib.h"

// (GLOBAL-VARIABLES) ---------------------------------------------------------

SIZE g_Zoom = { 1, 1 };

BPT::CDIBSection m_DisplayDib;

bool g_bAppActive = false;

HWND g_hWnd = NULL;

MSG g_Msg;

DWORD g_dwNextTime;

DWORD g_dwTimeDelay = 33;

// (THE-GAME-DEFINES) ---------------------------------------------------------

extern "C" {

	DWORD g_OffscreenColorTable[ 256 ];

	BYTE g_Offscreen8bppBuffer[ (GAME_WIDTH * GAME_HEIGHT) ];

	unsigned int g_Keys = 0;

	// ------------------------------------------------------------------------

	//
	//	WindowsMessagePumpAndDisplayUpdate()
	//

	void WindowsMessagePumpAndDisplayUpdate()
	{
		g_Keys = 0;

		// Convert the game frame buffer to be windows dib friendly :)
		// --------------------------------------------------------------------

		BYTE * pDisplay = reinterpret_cast<BYTE *>( m_DisplayDib.GetBitmapBits() );

		if ( pDisplay ) {

			BYTE * pOffscreen = g_Offscreen8bppBuffer;

			int pitch = m_DisplayDib.Pitch();

			for ( int y = 0; y < GAME_HEIGHT; y++ ) {

				DWORD * pDst = (DWORD *)pDisplay;

				pDisplay += pitch;

				for ( int x = 0; x < GAME_WIDTH; x++ ) {

					*pDst = *(g_OffscreenColorTable + (*pOffscreen));

					++pDst;

					++pOffscreen;

				}

			}

			// debug stuff
			// ----------------------------------------------------------------

#if 0 
			if ( GAME_WIDTH >= 256 ) {

				memcpy( m_DisplayDib.GetBitmapBits(), (void *)g_OffscreenColorTable, sizeof(g_OffscreenColorTable) );

			}
#endif

		}

		// Mark the screen to refresh
		// --------------------------------------------------------------------

		InvalidateRect( g_hWnd, NULL, FALSE );

		// Run the message pump until time's up, if the quit message then
		// --------------------------------------------------------------------

		for ( ;; ) {

			// Process any windows messages, this is the highest priority!
			// ----------------------------------------------------------------

			for ( ;; ) {

				if ( g_bAppActive ) {

					if ( !PeekMessage( &g_Msg, NULL, 0, 0, PM_REMOVE ) ) {

						break;

					}
		
				} else {

					GetMessage( &g_Msg, NULL, 0, 0 ); 

				}
		
				if ( WM_QUIT == g_Msg.message ) {

					break;

				}

				TranslateMessage( &g_Msg );

				DispatchMessage( &g_Msg );

			}

			// ----------------------------------------------------------------

			if ( WM_QUIT == g_Msg.message ) {

				g_Keys = GAME_INPUT_QUIT_BIT;

				break;

			}

			// check to see if it's time to return to the game otherwise sleep
			// ----------------------------------------------------------------

			DWORD dwTime = timeGetTime();

			if ( g_dwNextTime <= dwTime ) {

				g_dwNextTime = dwTime + g_dwTimeDelay;

				break;

			}

			DWORD dwAmountFree = g_dwNextTime - dwTime;

			if ( 2 < dwAmountFree ) {

				Sleep( dwAmountFree - 1 );

			}

		}

		// Make sure that the window has updated
		// --------------------------------------------------------------------

		UpdateWindow( g_hWnd );

		// Get the current state of the the input
		// --------------------------------------------------------------------

		g_Keys |= (0x8000 & GetAsyncKeyState( VK_LEFT )) ? GAME_INPUT_L_BIT : 0;
		g_Keys |= (0x8000 & GetAsyncKeyState( VK_RIGHT )) ? GAME_INPUT_R_BIT : 0;
		g_Keys |= (0x8000 & GetAsyncKeyState( VK_UP )) ? GAME_INPUT_U_BIT : 0;
		g_Keys |= (0x8000 & GetAsyncKeyState( VK_DOWN )) ? GAME_INPUT_D_BIT : 0;
		g_Keys |= (0x8000 & GetAsyncKeyState( VK_SPACE )) ? GAME_INPUT_A_BIT : 0;

	}

};

// ------------------------------------------------------------------------

//
//	KickStart()
//

void KickStart() {

	for ( ;; ) {

		g_Keys = 0;

		// clear the buffer to 0
		// ----------------------------------------------------------------

		memset( g_Offscreen8bppBuffer, 0, sizeof(g_Offscreen8bppBuffer) );

		// clear the palette
		// ----------------------------------------------------------------

		memset( g_OffscreenColorTable, 0, sizeof(g_OffscreenColorTable) );

		// Run the game
		// ----------------------------------------------------------------

		timeBeginPeriod( 1 );

		g_dwNextTime = timeGetTime();

#if 1

		MASM_TinyProgram();

#else

		TinyProgram();

#endif

		timeEndPeriod( 1 );

		// Ask the user if they want to restart
		// ----------------------------------------------------------------

		if ( WM_QUIT == g_Msg.message ) {

			break;

		}

		if ( IDNO == MessageBox(
			g_hWnd, "Would you like to play again?", "Game Over", MB_YESNO | MB_ICONQUESTION
			) ) {

			PostMessage( g_hWnd, WM_CLOSE, 0, 0 );

			for ( ;; ) {

				GetMessage( &g_Msg, NULL, 0, 0 ); 
		
				if ( WM_QUIT == g_Msg.message ) break;

				TranslateMessage( &g_Msg );

				DispatchMessage( &g_Msg );

			}

			break;

		}

	}

}

// ----------------------------------------------------------------------------

//
//	ChangeWindowZoom()
//

void ChangeWindowZoom( HWND hWnd, const int zx, const int zy )
{
	g_Zoom.cx = zx;
	g_Zoom.cy = zy;

	RECT rect = { 0, 0, 0, 0 };

	AdjustWindowRectEx(
		&rect, GetWindowLong(g_hWnd,GWL_STYLE), (NULL != GetMenu(g_hWnd)),
		GetWindowLong(g_hWnd,GWL_EXSTYLE)
	);

	rect.right += GAME_WIDTH * g_Zoom.cx;
	rect.bottom += GAME_HEIGHT * g_Zoom.cy;

	SetWindowPos(
		g_hWnd, NULL, 0, 0, 
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER
	);

	InvalidateRect( hWnd, NULL, false );

	UpdateWindow( hWnd );

}

// ----------------------------------------------------------------------------

//
//	WndProc()
//

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message ) {

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);

		if ( m_DisplayDib.GetHBitmap() ) {

			BPT::BLITFX blitFX(
				(BPT::BLITFX::WIDTH_SPECIFIED | BPT::BLITFX::HEIGHT_SPECIFIED),
				GAME_WIDTH * g_Zoom.cy, GAME_HEIGHT * g_Zoom.cy
			);

			BPT::T_BlitAt( hdc, 0, 0, m_DisplayDib, &blitFX );

		} else {

			PatBlt(
				hdc, 
				ps.rcPaint.left, ps.rcPaint.top, 
				ps.rcPaint.right - ps.rcPaint.left, 
				ps.rcPaint.bottom - ps.rcPaint.top,
				BLACKNESS
			);

		}
		EndPaint(hWnd, &ps);
		}
		break;

	case WM_CHAR:
		switch ( wParam ) {

		case 'z':
		case 'Z':
			{
				int zoom = 
					((GAME_WIDTH*(g_Zoom.cx+1)) < GetSystemMetrics(SM_CXFULLSCREEN)) ? (g_Zoom.cx + 1) : 1;

				ChangeWindowZoom( hWnd, zoom, zoom );
			}
			break;

		default:
			break;

		}
		break;

	case WM_ACTIVATEAPP:
		g_bAppActive = (TRUE == wParam);
		break;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;

	case WM_ERASEBKGND:
		return TRUE;
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}
	return 0;
}

// ----------------------------------------------------------------------------

//
//	_tWinMain()
//

int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{

	// register windows class 
	// ------------------------------------------------------------------------

	char * szWindowClass = "BPT::0E3F5633-54A9-4e9f-B53E-A4B27B823F02";

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_ICON1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; // (LPCTSTR)IDC_TEST256;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	if ( !RegisterClassEx(&wcex) ) {

		return 0;

	}

	// create our display dib
	// ------------------------------------------------------------------------

	if ( !m_DisplayDib.Create( GAME_WIDTH, GAME_HEIGHT, (HDC)0, 32, 0xff0000, 0xff00, 0xff ) ) {

		return 0;

	}

	// create the window
	// ------------------------------------------------------------------------

	g_hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass, "Tiny framework 0.001", WS_POPUP | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL
	);

	if ( !g_hWnd ) {

		return 0;

	}

	// deal with sizing the window to the desired client size
	// ------------------------------------------------------------------------

	RECT rect = { 0, 0, 0, 0 };

	AdjustWindowRectEx(
		&rect, GetWindowLong(g_hWnd,GWL_STYLE), (NULL != GetMenu(g_hWnd)),
		GetWindowLong(g_hWnd,GWL_EXSTYLE)
	);

	rect.right += GAME_WIDTH * g_Zoom.cx;
	rect.bottom += GAME_HEIGHT * g_Zoom.cy;

	SIZE wndSize;

	wndSize.cx = rect.right - rect.left;
	wndSize.cy = rect.bottom - rect.top;

	SetWindowPos(
		g_hWnd, NULL,
		(GetSystemMetrics(SM_CXFULLSCREEN) - wndSize.cx)/2,
		(GetSystemMetrics(SM_CYFULLSCREEN) - wndSize.cy)/2,
		wndSize.cx, wndSize.cy,
		SWP_NOZORDER | SWP_NOOWNERZORDER
	);

	ShowWindow( g_hWnd, nCmdShow );

	UpdateWindow( g_hWnd );

	// Finally start our game
	// ------------------------------------------------------------------------

	KickStart();

	return (int) g_Msg.wParam;
}
