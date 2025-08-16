# tiny framework

A minimal Win32 framework for creating simple 2D games or graphical applications.

## Framework Elements

The framework is composed of the following key elements:

*   **`tiny_framework.h` / `tiny_framework.cpp`**: This is the core of the framework. It handles the creation of the main window, the message loop, and input processing. It also manages the display, using a Device-Independent Bitmap (DIB) as a framebuffer for rendering.

*   **`TinyProgram.h`**: This header file defines the interface between the framework and the application logic. It specifies the screen dimensions, input constants, and declares the main application function (`TinyProgram` or `MASM_TinyProgram`).

*   **`TinyProgram.cpp`**: This file provides a basic C++ implementation of the main application loop (`TinyProgram`). This is where the developer would typically write their game logic.

*   **`game.asm`**: This file provides an assembly language implementation of the main application loop (`MASM_TinyProgram`). This is an alternative to `TinyProgram.cpp` for those who prefer to write their application in assembly.

*   **`BPTDib.h`**: This file contains the definition for the `BPT::CDIBSection` class, which is used to manage the DIB framebuffer.

## How it Works

The framework sets up a Win32 window and then calls the `TinyProgram` (or `MASM_TinyProgram`) function in a loop. The `TinyProgram` function is responsible for:

1.  Handling user input by checking the `g_Keys` variable.
2.  Drawing graphics into the offscreen buffer (`g_Offscreen8bppBuffer`) and setting the color palette (`g_OffscreenColorTable`).
3.  Calling `WindowsMessagePumpAndDisplayUpdate()` to update the display and process window messages.

This framework is designed to be a simple starting point for Win32 graphics programming, with a focus on low-level control and minimal dependencies.
