/*==========================================================================
  $File: $
  $Date: June, 18 2026
  $Revision: $
  $Creator: Casey Muratori David M.
  $Notice: (C) Copyright 2026 by Molly Rocket, Inc. All Rights Reserved. $
===========================================================================*/
#include <Windows.h>

// static can have different meanings
// internal: can define a function as being local to the file that it's in
// local : only exists within its function scope but retains its value
// global : globally accessible (automatically initialized to 0)
#define internal static  
#define local_persist static 
#define global_variable static

// a static variable  is:
//   Created only once
//   Keeps its value between function calls
//   Exists for the lifetime of the program
// TODO: This is a global for now
global_variable bool Running; 
global_variable BITMAPINFO BitmapInfo; // set all fields to 0
// BitmapMemory is what we'll receive back from Windows that we can draw into
// OUR PIXEL BUFFER 
global_variable void* BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

// DIB: Device Independent Bitmap.
// That's the name Windows uses to talk about things
// we can write into as bitmaps it can then display using Gdi32.lib
// this function resizes the DIB or initializes it
internal void Win32ResizeDIBSection(int Width, int Height)
{
  // TODO: Bulletproof this.
  // Maybe don't free first, free after, then free first if that fails.

  // TODO: Free our DIBSection
  if(BitmapHandle)
  {
    DeleteObject(BitmapHandle);
  }
  else
  {
    // Ask Windows to create a memory device context (something we use to draw)
    // that's compatible with our current application's screen
    HDC BitmapDeviceContext = CreateCompatibleDC(0);
  }
  // the description of the bitmap
  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader); // size of the headers
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32; // nb of bits per color= 2^32 colors
  BitmapInfo.bmiHeader.biCompression = BI_RGB;
  
  // Creates a bitmap that's suitable to use with our device context  
  BitmapHandle = CreateDIBSection(
				  BitmapDeviceContext,
				  &BitmapInfo, // BITMAPINFO*, the address of the struct is passed
				  DIB_RGB_COLORS,
				  &BitmapMemory, // void**, the address of the pointer is passed
				  0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
  // X = left (Windows convention)
  // Y = top  
  // Rectangle to rectangle copy
   StretchDIBits(DeviceContext,
		 X, Y, Width, Height, // destination: rectangle Windows wants us to fill
		 X, Y, Width, Height, // source: smae size as destination for now
		 BitmapMemory,
		 &BitmapInfo,
		 DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK Win32MainWindowCallback(HWND Window,
					 UINT Message,
					 WPARAM WParam,
					 LPARAM LParam)
{
  LRESULT Result = 0;
  switch(Message) 
  {
    case WM_SIZE:
      {
	//brakets are used to prevent variables from propagating
	RECT ClientRect;
	// the client is the drawing area of a window (borders and menu bars are not included)
	GetClientRect(Window, &ClientRect);
	int Height =  ClientRect.right - ClientRect.left;
	int Width =   ClientRect.bottom - ClientRect.top;
	// forces to match DIB to the window size - it creates the buffer we'll draw to
	Win32ResizeDIBSection(Width, Height);

      }break; // Casey prefers the break outside
    case WM_DESTROY:
      {
	// TODO: Handle this as an error - recreate window?
	Running = false;
      }break;
    case WM_CLOSE:
      {
	// exit the message loop, we can pass a parameter that GetMessage() will return
	// but in our game use case it won't be necessary
	// PostQuitMessage(0);	
	// TODO: Handle this with a message to the user?
	Running = false;
      }break;
    case WM_ACTIVATEAPP:
      {
	OutputDebugStringA("WM_ACTIVATEAPP\n");
      }break;
    case WM_PAINT: 
      {
	// copy to the client (DIB) what's in the buffer
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(Window, &Paint);
	// get the returned RECT struct from the PAINTSTRUCT
	RECT Rectangle = Paint.rcPaint;
	int X = Rectangle.left;
	int Y = Rectangle.top;
	int Height =  Rectangle.right - Rectangle.left;
	int Width =   Rectangle.bottom - Rectangle.top;
	
	Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
	
        EndPaint(Window, &Paint); 
      }break;
    default:
      {
	// Calls the default window procedure to provide default processing 
	// for any window messages that an application does not process
	Result = DefWindowProcA(Window,Message,WParam,LParam);
      }break;
  }
  return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance,
		     HINSTANCE PrevInstance,
		     LPSTR     CommandLine,
		     int       ShowCode) 
{

  // WNDCLASS is a struct (lookup tagWNDCLASS)
  WNDCLASS WindowClass = {}; // initialize to 0 every bytes
  
  // check if CS_OWNDC|CS_HREDRAW|CS_VREDRAW still matter per Casey
  //  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; <-- not needed

  // callback called when Windows needs to send a message to have it do something
  WindowClass.lpfnWndProc = Win32MainWindowCallback;  
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWidowClass";
  
  // returns an Atom but won't be needed 
  if(RegisterClassA(&WindowClass))
  {
    HWND WindowHandle =
      CreateWindowExA(
		     0,
		     WindowClass.lpszClassName,
		     "Handmade Hero",
		     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		     CW_USEDEFAULT,
		     CW_USEDEFAULT,
		     CW_USEDEFAULT,
		     CW_USEDEFAULT,
		     0,
		     0,
		     Instance,
		     0);
    if(WindowHandle)
      {
	// start a message loop
	Running = true;
	while(Running)
	  {
	    MSG Message;
	    BOOL MessageResult = GetMessageA(&Message,0, 0,0); //loops until it returns false
		if (MessageResult > 0)
		  { 
		    TranslateMessage(&Message);
		    // calls our callback MainWindowCallback() but Windows can call our callback function
		    // outside of this function
		    DispatchMessageA(&Message); 
		  }
		else // in case the handler is invalid
		  {
		    break;
		  }

	  }
      }
    else
      {
	// TODO: failed creating window
      }
  }
  else
  {
    // TODO: logging
  }
  
  return (0);
  
}

