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

// DIB: Device Independent Bitmap.
// That's the name Windows uses to talk about things we can write into
internal void ResizeDIBSection()
{
}

LRESULT CALLBACK MainWindowCallback(HWND Window,
				    UINT Message,
				    WPARAM WParam,
				    LPARAM LParam)
{
  LRESULT Result = 0;
  switch(Message) 
  {
    case WM_SIZE:
      { //brakets are used to prevent variables from propagating
	OutputDebugStringA("WM_SIZE\n");
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
	// paints
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(Window, &Paint);
	// get the returned RECT struct from the PAINTSTRUCT
	RECT Rectangle = Paint.rcPaint;
	int X = Rectangle.left;
	int Y = Rectangle.top;
	int Height =  Rectangle.right - Rectangle.left;
	int Width =   Rectangle.bottom - Rectangle.top;

	// we're playing with the background color
	// static here means we want the variable to persist akin to a global variable but within this case's scope
	// only use when debugging
	// it initializes only once
	local_persist DWORD Operation = WHITENESS;
	// The PatBlt function paints the specified rectangle using the brush 
	// that is currently selected into the specified device context. 
	// The brush color and the surface color or colors are combined by using the specified raster operation.
	PatBlt(DeviceContext, X, Y, Width, Height, Operation);

	// Toggle operation for next call
	Operation = (Operation == WHITENESS) ? BLACKNESS : WHITENESS;

        EndPaint(Window, &Paint); 

        return 0L; 
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
  WindowClass.lpfnWndProc = MainWindowCallback;  
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

