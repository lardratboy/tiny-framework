#include "StdAfx.h"
#include "tinyprogram.h"

// ----------------------------------------------------------------------------
// LICENSE
//
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
// ----------------------------------------------------------------------------

extern "C" {

	// ------------------------------------------------------------------------

	//
	//	TINY::Program()
	//

	__declspec(naked) void TinyProgram() {

		_asm {

			;; general setup
			;; ----------------------------------------------------------------

			pushad											;; store everything!

			;; ----------------------------------------------------------------

		start_of_frame:

			;; Handle input 
			;; ----------------------------------------------------------------

			mov			edi, g_Keys							;; handle input
			shr			edi, 1								;; get quit key
			jc			goodbye								;; application is quitting

			;; ----------------------------------------------------------------
			;; THIS CALL MUST BE MADE!!!!!!
			;; since we don't have simple VGA, we ask windows to display our
			;; framebuffer.  Also since machines are insanely fast this adds
			;; a delay as well.
			;; ----------------------------------------------------------------

			pushad											;; store all regs
			call		WindowsMessagePumpAndDisplayUpdate	;; call the framework
			popad											;; restore all regs
			jmp			start_of_frame						;; next frame...

			;; general shutdown
			;; ----------------------------------------------------------------

		goodbye:

			popad											;; restore everything!
			ret			0									;; get out of dodge

		}

	}

}; // extern "C"


