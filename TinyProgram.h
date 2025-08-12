#pragma once

// ----------------------------------------------------------------------------
// LICENSE
//
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
// ----------------------------------------------------------------------------

#define GAME_WIDTH					(320)
#define GAME_HEIGHT					(240)

#define GAME_INPUT_QUIT_BIT			0x00000001
#define GAME_INPUT_L_BIT			0x00000002
#define GAME_INPUT_R_BIT			0x00000004
#define GAME_INPUT_U_BIT			0x00000008
#define GAME_INPUT_D_BIT			0x00000010
#define GAME_INPUT_A_BIT			0x00000020

// ----------------------------------------------------------------------------

extern "C" {

	// (SIMULATED-HARDWARE) ---------------------------------------------------

	extern DWORD g_OffscreenColorTable[ 256 ];

	extern BYTE g_Offscreen8bppBuffer[ (GAME_WIDTH * GAME_HEIGHT) ];

	extern unsigned int g_Keys;

	// (DISPLAY-UPDATE) -------------------------------------------------------

	void WindowsMessagePumpAndDisplayUpdate();

};

// ----------------------------------------------------------------------------

extern "C" {

	void TinyProgram();
	void MASM_TinyProgram();

}; // extern "C"


