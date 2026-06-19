/*==========================================================================
  $File: $
  $Date: June, 18 2026
  $Revision: $
  $Creator: Casey Muratori David M.
  $Notice: (C) Copyright 2026 by Molly Rocket, Inc. All Rights Reserved. $
===========================================================================*/
#include <Windows.h>

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
	OutputDebugStringA("WM_DESTROY\n");
      }break;
    case WM_CLOSE:
      {
	OutputDebugStringA("WM_CLOSE\n");
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
	static DWORD Operation = WHITENESS;
	// The PatBlt function paints the specified rectangle using the brush 
	// that is currently selected into the specified device context. 
	// The brush color and the surface color or colors are combined by using the specified raster operation.
	PatBlt(DeviceContext, X, Y, Height, Width, Operation); 
	if(Operation == WHITENESS)
	  {
	    Operation = BLACKNESS;
	  }
	else
	  {
	    Operation = WHITENESS;
	  }
        EndPaint(Window, &Paint); 
        return 0L; 
      }break;
    default:
      {
	// OutputDebugStringA("default\n");
	Result=DefWindowProc(Window,Message,WParam,LParam);
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
  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; 
  WindowClass.lpfnWndProc = MainWindowCallback; // callback called when Windows needs to send a message to have it do something
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWidowClass";
  
  // returns an Atom but won't be needed 
  if(RegisterClass(&WindowClass))
  {
    HWND WindowHandle =
      CreateWindowEx(
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
	MSG Message;
	for(;;)
	  {
	    BOOL MessageResult = GetMessage(&Message,0, 0,0); //loops until it returns false
		if (MessageResult > 0)
		  { 
		    TranslateMessage(&Message); 
		    DispatchMessage(&Message); 
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

