/*==========================================================================
  $File: $
  $Date: June, 18 2026
  $Revision: $
  $Creator: Casey Muratori David M.
  $Notice: (C) Copyright 2026 by Molly Rocket, Inc. All Rights Reserved. $
===========================================================================*/
#include <Windows.h>
#include <stdint.h>

// WM_SIZE
//     |
//     v
// Create pixel buffer
//     |
//     v
// BitmapMemory exists

// WM_PAINT
//     |
//     v
// Copy BitmapMemory to screen
////////////////////////////////////////////////
// in computer graphics a screen coordinates is:
// (0,0) -----------------> x
// |
// |
// |
// v
// y

// static can have different meanings
// internal: can define a function as being local to the file that it's in
// local : only exists within its function scope but retains its value
// global : globally accessible (automatically initialized to 0)
// a static variable  is:
//   Created only once
//   Keeps its value between function calls
//   Exists for the lifetime of the program
#define internal static  
#define local_persist static 
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// uint8 is an alias for unsigned 8-bit integer
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer{
  BITMAPINFO Info; // set all fields to 0
  void* Memory; // OUR PIXEL BUFFER: is what we'll receive back from Windows that we can draw into
  int Width;
  int Height;
  int Pitch;
};
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;

struct win32_window_dimension
{
  int Width;
  int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
  {
    win32_window_dimension Result;
    RECT ClientRect;
    // the client is the drawing area of a window (borders and menu bars are not included)
    // get size of the window
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return(Result);
  }

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset)
{
  // we want to move through the buffer 1 byte at a time
  uint8* Row = (uint8*) Buffer.Memory;

  for(int Y = 0; Y < Buffer.Height; ++Y)
  {
    uint32* Pixel = (uint32*) Row;
    for(int X = 0; X < Buffer.Width; ++X)
      {
	/*
	  Pixel in memory (hex): 00 00 00 00
	  RR GG BB xx
	  Little ENDIAN architecture (smaller byte first): BBGGRRxx
	  But when loading 32-bit integers, the architecture is actually
	  // in big ENDIAN: xxRRGGBB
	 */
	
	// uint8 means we just want the first byte of X (255 values possible)
	// small types get promoted to int in memory
	uint8 Blue = (X + BlueOffset); 
	uint8 Green = (Y + GreenOffset);
	uint8 Red = 0;

	// Green gets moved 8 bits left inside the 4-bytes
	// We dereference Pixels to write to where it's pointing
	*Pixel++ = (Red | (Green << 8) | Blue);
	// 00000000 00000000 00000000 00000000 | 00000000 00000000 00000000 000000
	// Pixel++ is a post increment operator similar to ++Pixel after assigning a color
	// so *Pixel++ is a shorthand for *Pixel = ...; ++Pixel or Pixel=Pixel+1;
	// the compiler silently does: Pixel=Pixel + 1*sizeof(uint32) (which is 4bytes)
      }
    // if Row was a uint32 the compiler would silently multiply Pitch by 4 bytes
    // that's why we declared Row as a uint8
    Row += Buffer.Pitch;
  }
  
}

// DIB: Device Independent Bitmap.
// That's the name Windows uses to talk about things
// we can write into as bitmaps it can then display using Gdi32.lib
// this function resizes the DIB or initializes it
internal void
Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height)
{
  // TODO: Bulletproof this.
  // Maybe don't free first, free after, then free first if that fails.
  if(Buffer->Memory)
    {
      // releases the bitmap memory pages obtained from VirtualAlloc
      VirtualFree(Buffer->Memory, 0, MEM_RELEASE); 
    }

  Buffer->Width = Width;
  Buffer->Height = Height;
  int BytesPerPixel= 4;  // pixel size is 4 bytes (RGB+padding)
  
  // the description of the bitmap
  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader); // size of the headers
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  // NOTE(david): if biHeight is negative then StretchDIBits will create a
  // top-down image which is preferable
  // top-down as opposed to bottom-up, means the first 3 bytes of the image are the color for
  // the top left pixel in the bitmap, not the bottom left
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height; 
  Buffer->Info.bmiHeader.biPlanes = 1;
  // nb of bits per color= 2^32 colors, 8-bit red, 8-bit green, 8-bit blue, 8-bit for padding
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
  // VirtualAlloc() =>  allocates a certain number of memory pages
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  // Pitch is the size of a row, 1 pixel = 4 bytes
  // used to get to the nex row
  Buffer->Pitch = Buffer->Width * BytesPerPixel;

}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
			   int WindowWidth, int WindowHeight, win32_offscreen_buffer Buffer)
{
  // TODO: Aspect ratio correction
  //WindowWidth / WindowHeight;
  // TODO: play with stretch modes
  StretchDIBits(DeviceContext,
		0, 0, WindowWidth, WindowHeight, // source
		0, 0, Buffer.Width, Buffer.Height, // destination: rectangle Windows wants us to fill
		Buffer.Memory,
		&Buffer.Info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
			UINT Message,
			WPARAM WParam,
			LPARAM LParam)
{
  LRESULT Result = 0;
  switch(Message) 
  {
    
    case WM_SIZE:
      {	//brakets are used to prevent variables from propagating
      }break; // Casey prefers the break outside
      
    case WM_DESTROY:
      {
	// TODO: Handle this as an error - recreate window?
	GlobalRunning = false;
      }break;
    case WM_CLOSE:
      {
	// exit the message loop, we can pass a parameter that GetMessage() will return
	// but in our game use case it won't be necessary
	// PostQuitMessage(0);	
	// TODO: Handle this with a message to the user?
	GlobalRunning = false;
      }break;
    case WM_ACTIVATEAPP:
      {
	OutputDebugStringA("WM_ACTIVATEAPP\n");
      }break;
      
    case WM_PAINT: 
      {
	OutputDebugStringA("WM_PAINT\n");
	// copy to the client (DeviceContext) what's in the buffer (DIB)
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(Window, &Paint);
	win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer);
        EndPaint(Window, &Paint);
	
      }break;

  default:
      {
	OutputDebugStringA("DEFAULT\n");
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
  WNDCLASS WindowClass = {}; // initializes every byte to 0 
  
  // forces to match DIB to the window size
  // Initialize the back buffer
  Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
	
  // CS_HREDRAW|CS_VREDRAW indicate Windows it needs to repaint the whole window when it's resized
  WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  // callback for when Windows needs to send a message to have it do something
  WindowClass.lpfnWndProc = Win32MainWindowCallback;  
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  
  // returns an Atom but won't be needed 
  if(RegisterClassA(&WindowClass))
  {
    HWND Window =
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
    if(Window)
      {
	// NOTE: since we specified CS_OWNDC, we can just get one device context and use it forever
	// because we are not sharing it with anyone.
	// asks Windows to give us back control by giving us a handle for us to issue draw calls to
	HDC DeviceContext = GetDC(Window);	
	int XOffset = 0;
	int YOffset = 0;
	
	// start a message loop
	GlobalRunning = true;
	while(GlobalRunning)
	{
	  MSG Message;
	  
	  while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	  {
	      if(Message.message == WM_QUIT)
	      {
		GlobalRunning = false;
	      }
	      
	      TranslateMessage(&Message);
	      // calls our callback MainWindowCallback() but Windows can call our callback function
	      // outside of this function
	      DispatchMessageA(&Message); 
	  }

	  // Create the bitmap
	  RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);

	  // gets the dimension of the drawable region of the window, left and top are always 0
	  win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	  // display our buffer
	  Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer);
	  ReleaseDC(Window, DeviceContext);
	  
	  ++XOffset;
	  YOffset +=2;
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

