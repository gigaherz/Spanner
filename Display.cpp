
#include <Windows.h>
#include <mmsystem.h>

HWND hWnd;
BITMAPINFO bmi;

unsigned int X=0;
unsigned int Y=0;

int button;

void *buffer;

#define D3DX_PI 3.1415926535897932384626433832795f

#define WIDTH 640
#define HEIGHT 480

VOID Render()
{
	//render into buffer here

	HDC winDC=GetDC(hWnd);

	RECT r={0,HEIGHT,WIDTH,HEIGHT+200};
	FillRect(winDC,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));

	SetDIBitsToDevice(
		winDC,
		0,0,WIDTH,HEIGHT,
		0,0,0,HEIGHT,
		buffer,
		&bmi,
		DIB_RGB_COLORS
);


	ReleaseDC(hWnd,winDC);

	/*frames++;
	
	int tickdiff=GetTickCount()-ticks;
	if(tickdiff>=1000)
	{
		static char m1[100];
		float fps=(frames*1000.0)/tickdiff;
		sprintf(m1,"%d frames rendered in %d ms = %1.3f fps.", frames,tickdiff,fps);
		SetWindowText(hWnd,m1);
		frames=0;
		ticks=GetTickCount();
	}*/
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_DESTROY:
		//Cleanup();
		PostQuitMessage( 0 );
		return 0;

	case WM_KEYDOWN:
		//Cleanup();
		PostQuitMessage( 0 );
		return 0;

	case WM_MOUSEMOVE:

		if(wParam)
		{
			X=((unsigned int)lParam)&0xFFFF;
			Y=((unsigned int)lParam>>16);
		}


	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

INT WINAPI RealMain() // HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	// Register the window class
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"DrawingWnd", NULL };
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx( &wc );

	// Create the application's window
	hWnd = CreateWindow( "DrawingWnd", "Span Rendering",
		WS_OVERLAPPEDWINDOW, 0, 0, WIDTH+32, HEIGHT+32,
		GetDesktopWindow(), NULL, wc.hInstance, NULL );

	bmi.bmiHeader.biSize=sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biBitCount=32;
	bmi.bmiHeader.biWidth=WIDTH;
	bmi.bmiHeader.biHeight=HEIGHT;
	bmi.bmiHeader.biCompression=BI_RGB;
	bmi.bmiHeader.biClrUsed=0;
	bmi.bmiHeader.biClrImportant=0;
	bmi.bmiHeader.biPlanes=1;
	bmi.bmiHeader.biSizeImage=0;

	// Show the window
	ShowWindow( hWnd, SW_SHOWDEFAULT );
	UpdateWindow( hWnd );

	// Enter the message loop
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
	while( msg.message!=WM_QUIT )
	{
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
			Render();
	}

	UnregisterClass( "DrawingWnd", wc.hInstance );
	return 0;
}
