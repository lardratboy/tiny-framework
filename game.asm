;; ============================================================================
;; LICENSE
;;
;; This software is provided "as is" without express or implied
;; warranty, and with no claim as to its suitability for any purpose.
;; ============================================================================
;;

.586
.MMX
.model flat, c
;;PAGE ,128

;; ----------------------------------------------------------------------------
;; Implmentation options (MUST MATCH TINYPROGRAM.H...)
;; ----------------------------------------------------------------------------

GAME_WIDTH					equ		(320)
GAME_HEIGHT					equ		(240)
GAME_INPUT_QUIT_BIT			equ		0x00000001
GAME_INPUT_L_BIT			equ		0x00000002
GAME_INPUT_R_BIT			equ		0x00000004
GAME_INPUT_U_BIT			equ		0x00000008
GAME_INPUT_D_BIT			equ		0x00000010
GAME_INPUT_A_BIT			equ		0x00000020

;; ============================================================================
.data
;; ============================================================================

;; ----------------------------------------------------------------------------
;; Deal with the data section imports
;; ----------------------------------------------------------------------------

EXTRN g_Keys:DWORD
EXTRN g_OffscreenColorTable:DWORD
EXTRN g_Offscreen8bppBuffer:BYTE

;; ----------------------------------------------------------------------------
;; data section
;; ----------------------------------------------------------------------------

;; ============================================================================
.code
;; ============================================================================

;; ----------------------------------------------------------------------------
;; Deal with the function imports
;; ----------------------------------------------------------------------------

EXTRN WindowsMessagePumpAndDisplayUpdate:NEAR

;; ----------------------------------------------------------------------------

;;
;;	MASM_TinyProgram
;;

MASM_TinyProgram proc

	;; general setup
	;; ------------------------------------------------------------------------

	pushad											;; store everything!

	;; ------------------------------------------------------------------------

start_of_frame:

	;; Handle input 
	;; ------------------------------------------------------------------------

	mov			edi, g_Keys							;; handle input
	shr			edi, 1								;; get quit key
	jc			goodbye								;; application is quitting

	;; ------------------------------------------------------------------------
	;; THIS CALL MUST BE MADE!!!!!!
	;; since we don't have simple VGA, we ask windows to display our
	;; framebuffer.  Also since machines are insanely fast this adds
	;; a delay as well.
	;; ------------------------------------------------------------------------

	pushad											;; store all regs
	call		WindowsMessagePumpAndDisplayUpdate	;; call the framework
	popad											;; restore all regs
	jmp			start_of_frame						;; next frame...

	;; general shutdown
	;; ------------------------------------------------------------------------

goodbye:

	popad											;; restore everything!
	ret			0									;; get out of dodge
	
MASM_TinyProgram	endp

end
