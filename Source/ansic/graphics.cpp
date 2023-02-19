// graphics.cpp

//#define TEST_TEXT

#ifdef __BORLANDC__
#pragma warn -8027
#endif

#include "graphics.h"
#include "sysvar.h"
#include "error.h"
//#include "cursor.h"
#include "key.h"
#include "util.h"

// Para depuracion
#include <iostream>
using std::cerr;
using std::endl;
#include <cassert>
#define ASSERT assert

#include <queue>

#ifdef BLASSIC_USE_SVGALIB

#include <unistd.h>
#include <sys/types.h>
#include <vga.h>
#include <vgagl.h>

char * font= NULL;

#endif

#ifdef BLASSIC_USE_X

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display * display= 0;
int screen;
Window window;
Pixmap pixmap;
bool pixmap_created= false;
GC gc, gcp;
XGCValues gcvalues, gcvaluesp;

XEvent x_event;

long eventusedmask= ExposureMask | KeyPressMask |
	ButtonPressMask | ButtonReleaseMask |
	PointerMotionMask | EnterWindowMask;
	
XColor xcBlack, xcBlue, xcGreen, xcCyan,
        xcRed, xcMagenta, xcBrown, xcLightGrey,
	xcDarkGrey, xcLightBlue, xcLightGreen, xcLightCyan,
        xcLightRed, xcLightMagenta, xcYellow, xcWhite;
typedef XColor * pcolor;

#endif

#ifdef BLASSIC_USE_WINDOWS

#include <process.h>
#include <windows.h>

ATOM atomClass;
HANDLE hEvent;
HWND window;
HDC hdc;
HBITMAP pixmap;
HDC hdcPixmap;
HPEN xcBlack, xcBlue, xcGreen, xcCyan,
        xcRed, xcMagenta, xcBrown, xcLightGrey,
	xcDarkGrey, xcLightBlue, xcLightGreen, xcLightCyan,
        xcLightRed, xcLightMagenta, xcYellow, xcWhite;
typedef HPEN * pcolor;

#endif

#if defined (BLASSIC_USE_WINDOWS) || defined (BLASSIC_USE_X)

pcolor pforeground, pbackground;

#endif

#include <string>
#include <algorithm>

void draw_string (const std::string & str);

namespace {

bool fSynchro= false;

class CriticalSection {
public:
        CriticalSection ()
        {
                #ifdef BLASSIC_USE_WINDOWS
                InitializeCriticalSection (& cs);
                #endif
        }
        ~CriticalSection ()
        {
                #ifdef BLASSIC_USE_WINDOWS
                DeleteCriticalSection (& cs);
                #endif
        }
        void enter ()
        {
                #ifdef BLASSIC_USE_WINDOWS
                EnterCriticalSection (& cs);
                #endif
        }
        void leave ()
        {
                #ifdef BLASSIC_USE_WINDOWS
                LeaveCriticalSection (& cs);
                #endif
        }
private:
        #ifdef BLASSIC_USE_WINDOWS
        CRITICAL_SECTION cs;
        #endif
};

class CriticalLock {
public:
        CriticalLock (CriticalSection & cs) :
                cs (cs)
        {
                cs.enter ();
        }
        ~CriticalLock ()
        {
                cs.leave ();
        }
private:
        CriticalSection & cs;
};

class QueueKey {
public:
        QueueKey ()
        {
        }
        void push (const std::string & str)
        {
                CriticalLock lock (cs);
                q.push (str);
        }
        std::string pop ()
        {
                CriticalLock lock (cs);
                std::string str= q.front ();
                q.pop ();
                return str;
        }
        bool empty ()
        {
                return q.empty ();
        }
private:
        std::queue <std::string> q;
        CriticalSection cs;
};

QueueKey queuekey;

bool inited= false;
bool window_created= false;
bool opaquemode= true;

int xmousepos, ymousepos;

#ifdef BLASSIC_USE_SVGALIB

bool svgalib= false;

#else

//const bool svgalib= false;

#endif

int screenwidth, screenheight;
int lastx, lasty;

#if defined BLASSIC_USE_X

int drawmode= GXcopy;

#elif defined BLASSIC_USE_WINDOWS

int drawmode= R2_COPYPEN;

#endif

void reinit_pixmap ()
{
        #ifdef BLASSIC_USE_WINDOWS
        RECT r = { 0, 0, screenwidth, screenheight };
        FillRect (hdcPixmap, & r, (HBRUSH) GetStockObject (WHITE_BRUSH) );
        #endif

        #ifdef BLASSIC_USE_X
	XSetForeground (display, gcp, WhitePixel (display, screen) );
	XFillRectangle (display, pixmap, gcp, 0, 0, screenwidth, screenheight);
	XSetForeground (display, gcp, BlackPixel (display, screen) );
        #endif
}

void reinit_window ()
{
        #ifdef BLASSIC_USE_WINDOWS
        //HDC hdc= GetDC (window);
        BitBlt (hdc, 0, 0, screenwidth, screenheight, hdcPixmap, 0, 0, SRCCOPY);
        //ReleaseDC (window, hdc);
        #endif

        #ifdef BLASSIC_USE_X
	XSetFunction (display, gc, GXcopy);
	XCopyArea (display, pixmap, window, gc,
		0, 0, screenwidth, screenheight, 0, 0);
	XSetForeground (display, gc, BlackPixel (display, screen) );
	XSetForeground (display, gc, pforeground->pixel);
	XFlush (display);
	XSetFunction (display, gc, drawmode);
        #endif
}

#ifdef BLASSIC_USE_WINDOWS

const UINT
        WM_USER_CREATE_WINDOW= WM_USER + 3,
        WM_USER_DESTROY_WINDOW= WM_USER + 4;

HANDLE hthread= NULL;
DWORD idthread= 0;

LRESULT APIENTRY windowproc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        static int width, height;
        switch (uMsg) {
        case WM_SIZE:
                width= LOWORD (lParam);
                height= HIWORD (lParam);
                return TRUE;
        case WM_ERASEBKGND:
                return TRUE;
        case WM_PAINT:
                {
                //err << "WM_PAINT " << width << ", " << height << endl;
                PAINTSTRUCT paintstruct;
                HDC hdc= BeginPaint (hwnd, & paintstruct);
                BitBlt (hdc, 0, 0, width, height, hdcPixmap, 0, 0, SRCCOPY);
                EndPaint (hwnd, & paintstruct);
                }
                return FALSE;
        case WM_KEYDOWN:
                {
                        WORD k= (WORD) wParam;
                        //std::string str= string_from_virtual_key (k);
                        std::string str= string_from_key (k);
                        if (! str.empty () )
                        {
                                queuekey.push (str);
                                return TRUE;
                        }
                }
                return FALSE;
        case WM_CHAR:
                {
                        char c= (char) wParam;
                        queuekey.push (std::string (1, c) );
                }
                return TRUE;
        case WM_MOUSEMOVE:
                xmousepos= LOWORD (lParam);
                ymousepos= HIWORD (lParam);
                return TRUE;
        case WM_LBUTTONDOWN:
                queuekey.push (strCLICK);
                return TRUE;
        case WM_RBUTTONDOWN:
        	queuekey.push (strSCLICK);
        	return TRUE;
        case WM_LBUTTONUP:
                queuekey.push (strRELEASE);
                return TRUE;
        case WM_RBUTTONUP:
        	queuekey.push (strSRELEASE);
        	return TRUE;
        case WM_DESTROY:
                SetEvent (hEvent);
                return TRUE;
        default:
                return DefWindowProc (hwnd, uMsg, wParam, lParam);
        }
}

void thread_create_window (int width, int height)
{
        HDC hdcscreen= CreateDC ("DISPLAY", NULL, NULL, NULL);
        if (hdcscreen == NULL)
                return;
        pixmap= CreateCompatibleBitmap (hdcscreen, width, height);
        if (pixmap == NULL)
        {
                DeleteDC (hdcscreen);
                return;
        }
        hdcPixmap= CreateCompatibleDC (hdcscreen);
        DeleteDC (hdcscreen);
        if (hdcPixmap == NULL)
        {
                DeleteObject (pixmap);
                pixmap= NULL;
                return;
        }
        SelectObject (hdcPixmap, pixmap);
        reinit_pixmap ();

        window= CreateWindow (
                LPCTSTR (atomClass),
                "blassic",
                WS_VISIBLE,
                0, 0,
                width + GetSystemMetrics (SM_CXDLGFRAME) * 2,
                height + GetSystemMetrics (SM_CYDLGFRAME) * 2 +
                        GetSystemMetrics (SM_CYCAPTION),
                NULL,
                NULL,
                GetModuleHandle (0),
                NULL);
        if (window)
        {
                SetActiveWindow (window);
                hdc= GetDC (window);
                window_created= true;
        }
        else
        {
                DeleteDC (hdcPixmap);
                DeleteObject (pixmap);
                pixmap= NULL;
        }
}

// Testing new method of create and destroy windows.

struct ThreadParams {
        int width, height;
};

unsigned WINAPI threadproc (void * arg)
{
        MSG msg;
        ThreadParams * tp= reinterpret_cast <ThreadParams *> (arg);
        thread_create_window (tp->width, tp->height);
        // Ensure the message queue exist before the main thread continues.
        // Perhpas unnecesary with the new method, but...
        PeekMessage (& msg, NULL, 0, UINT (-1), PM_NOREMOVE);
        SetEvent (hEvent);
        if (! window_created)
                return 0;
        while (GetMessage (& msg, NULL, 0, 0) )
        {
                switch (msg.message)
                {
                case WM_USER_CREATE_WINDOW:
                        //thread_create_window (msg.wParam, msg.lParam);
                        SetEvent (hEvent);
                        break;
                case WM_USER_DESTROY_WINDOW:
                        if (DestroyWindow (window) == 0)
                                cerr << "Error al destruir: " <<
                                        GetLastError () << endl;
                        break;
                default:
                        TranslateMessage (& msg);
                        DispatchMessage (& msg);
                }
        }
        return 0;
}

void create_thread (int width, int height)
{
        ThreadParams tp= { width, height };
        hthread= HANDLE (_beginthreadex (NULL, 0, threadproc,
                & tp, 0, (unsigned int *) (& idthread) ) );
        if (hthread == NULL)
        {
                cerr << "Error al crear thread para graficos" << endl;
                throw ErrBlassicInternal;
        }
        WaitForSingleObject (hEvent, INFINITE);
        if (! window_created)
        {
                WaitForSingleObject (hthread, INFINITE);
                CloseHandle (hthread);
                idthread= 0;
                hthread= NULL;
        }
}

void destroy_thread ()
{
        if (idthread)
        {
                BOOL r= PostThreadMessage (idthread, WM_QUIT, 0, 0);
                if (r == 0)
                {
                        TerminateThread (hthread, 0);
                }
                else
                {
                        WaitForSingleObject (hthread, INFINITE);
                }
                CloseHandle (hthread);
                idthread= 0;
                hthread= NULL;
        }
}

#endif // WINDOWS

void create_window (int width, int height)
{
        #ifdef BLASSIC_USE_WINDOWS
        if (hthread == NULL)
        {
                create_thread (width, height);
                if (hthread == NULL)
                        throw ErrBlassicInternal;
        }
        BOOL r= PostThreadMessage (idthread, WM_USER_CREATE_WINDOW,
                WPARAM (width), LPARAM (height) );
        if (r == 0)
        {
                cerr << "Error al comunicarse con thread de graficos. "
                        "GetLastError =" << GetLastError () <<
                        endl;
                destroy_thread ();
                throw ErrBlassicInternal;
        }
        WaitForSingleObject (hEvent, INFINITE);
        if (! window_created)
        {
                cerr << "Error al crear ventana" << endl;
                throw ErrBlassicInternal;
        }
        #endif

        #ifdef BLASSIC_USE_X
	window= XCreateSimpleWindow (display,
		RootWindow (display, screen),
		0, 0, width, height, 5,
		BlackPixel (display, screen),
		WhitePixel (display, screen) );
	window_created= true;

	int depth= DefaultDepth (display, DefaultScreen (display) );
	pixmap= XCreatePixmap (display, window,
		width, height, depth);
	pixmap_created= true;

	gc= XCreateGC (display, window, 0, & gcvalues);
	gcp= XCreateGC (display, pixmap, 0, & gcvaluesp);

	//XSetForeground (display, gcp, WhitePixel (display, screen) );
	//XFillRectangle (display, pixmap, gcp, 0, 0, width, height);
	//XSetForeground (display, gcp, BlackPixel (display, screen) );
	reinit_pixmap ();

	XSetStandardProperties (display, window,
			"blassic", "blassic", None,
			0, 0, NULL);

	XSelectInput (display, window, eventusedmask);

	XMapWindow (display, window);

	graphics::idle ();
        #endif
}

void destroy_window ()
{
        #ifdef BLASSIC_USE_WINDOWS
        PostThreadMessage (idthread, WM_USER_DESTROY_WINDOW, 0, 0);
        WaitForSingleObject (hEvent, INFINITE);
        DeleteDC (hdcPixmap);
        DeleteObject (pixmap);
        window= 0;
        window_created= false;
        destroy_thread ();
        #endif

        #ifdef BLASSIC_USE_X
	XDestroyWindow (display, window);
	window_created= false;
	XFreePixmap (display, pixmap);
	pixmap_created= false;
	XFlush (display);
	graphics::idle ();
        #endif
}

#ifdef BLASSIC_USE_WINDOWS

struct assign_color {
	pcolor pxc;
	int r, g, b;
};

class assign_elem {
public:
	void operator () (const assign_color & elem) const
	{
		* elem.pxc= CreatePen (PS_SOLID, 1,
			RGB (elem.r, elem.g, elem.b) );
	}
};

void init_wincolors ()
{
        #ifdef __BORLANDC__
        // With const Borland can't expand the for_each.
        typedef assign_color const_assign_color;
        #else
        typedef const assign_color const_assign_color;
        #endif
        static const_assign_color table_colors []= {
	        { &xcBlack,        0, 0, 0 },
        	{ &xcBlue,         0, 0, 42 * 4 },
	        { &xcGreen,        0, 42 * 4, 0 },
        	{ &xcCyan,         0, 42 * 4, 42 * 4 },
	        { &xcRed,          42 * 4, 0, 0 },
        	{ &xcMagenta,      42 * 4, 0, 42 * 4 },
	        { &xcBrown,        42 * 4, 21 * 4, 0 },
        	{ &xcLightGrey,    42 * 4, 42 * 4, 42 * 4 },

	        { &xcDarkGrey,     21 * 4, 21 * 4, 21 * 4 },
        	{ &xcLightBlue,    21 * 4, 21 * 4, 255 },
	        { &xcLightGreen,   21 * 4, 255, 21 * 4 },
        	{ &xcLightCyan,    21 * 4, 255, 255 },
	        { &xcLightRed,     255, 21 * 4, 21 * 4 },
        	{ &xcLightMagenta, 255, 21 * 4, 255 },
	        { &xcYellow,       255, 255, 21 * 4 },
        	{ &xcWhite,        255, 255, 255 }
	};
	std::for_each (table_colors,
		table_colors + util::dim_array (table_colors),
		assign_elem () );
}

#endif

#ifdef BLASSIC_USE_X

struct assign_color {
	pcolor pxc;
	const char * name;
};

class assign_elem {
public:
	assign_elem (Colormap & cm) : cm (cm) { }
	void operator () (const assign_color & elem)
	{
		XColor xc;
		XAllocNamedColor (display, cm,
			elem.name, elem.pxc, & xc);
	}
private:
	Colormap & cm;
};

void init_xcolors ()
{
        static const assign_color table_colors []= {
	        { &xcBlack,        "black" },
        	{ &xcBlue,         "blue" },
	        { &xcGreen,        "green" },
        	{ &xcCyan,         "cyan" },
	        { &xcRed,          "red" },
        	{ &xcMagenta,      "magenta" },
	        { &xcBrown,        "#7F7F00" },
        	{ &xcLightGrey,    "#3F3F3F" },

	        { &xcDarkGrey,     "darkgrey" },
        	{ &xcLightBlue,    "#0000FF" },
	        { &xcLightGreen,   "#00FF00" },
        	{ &xcLightCyan,    "#00FFFF" },
	        { &xcLightRed,     "#FF0000" },
        	{ &xcLightMagenta, "#FF00FF" },
	        { &xcYellow,       "yellow" },
        	{ &xcWhite,        "white" }
	};
	Colormap cm= DefaultColormap (display, screen);
	std::for_each (table_colors,
		table_colors + util::dim_array (table_colors),
		assign_elem (cm) );
}

#endif

} // namespace

void graphics::initialize (const char * progname)
{
	#ifdef BLASSIC_USE_SVGALIB
	if (geteuid () == 0) {
		std::string prog (progname);
		std::string::size_type l= prog.size ();
		if (l > 3 && prog.substr (l - 3) == "vga") {
			vga_init ();
			inited= true;
			svgalib= true;
			return;
		}
		else
			if (getuid () != 0)
				seteuid (getuid () );
	}
	#endif

	#ifdef BLASSIC_USE_X
	if (getenv ("DISPLAY") != NULL)
	{
		display= XOpenDisplay (0);
		if (display)
		{
			inited= true;
			screen= DefaultScreen (display);
			init_xcolors ();
		}
		else
			cerr << "Error al abrir display." << endl;
	}
	else
		cerr << "Sin soporte de graficos." << endl;
	#endif

        #ifdef  BLASSIC_USE_WINDOWS
        WNDCLASS wndclass;
        wndclass.style= CS_NOCLOSE | CS_OWNDC;
        wndclass.lpfnWndProc= windowproc;
        wndclass.cbClsExtra= 0;
        wndclass.cbWndExtra= 0;
        wndclass.hInstance= GetModuleHandle (0);
        wndclass.hIcon= 0;
        wndclass.hCursor= LoadCursor (NULL, IDC_ARROW);
        wndclass.hbrBackground= HBRUSH (GetStockObject (WHITE_BRUSH) );
        wndclass.lpszMenuName= 0;
        wndclass.lpszClassName= progname;
        atomClass= RegisterClass (& wndclass);
        if (atomClass == 0)
                cerr << "Error al registrar clase" << endl;
        else
        {
                inited= true;
                init_wincolors ();
        }
        hEvent= CreateEvent (NULL, FALSE, FALSE, NULL);
        // Event automatic, initial nonsignaled
        //create_thread ();
        #endif
}

void graphics::uninitialize ()
{
	if (! inited) return;

	#ifdef BLASSIC_USE_SVGA
	if (svgalib)
		//vga_setmode (TEXT);
		setmode (0);
	#endif

	#ifdef BLASSIC_USE_X
	if (display) {
		if (window_created)
			destroy_window ();
		XCloseDisplay (display);
	}
	#endif

        #ifdef BLASSIC_USE_WINDOWS
        //destroy_thread ();
        if (window_created)
                destroy_window ();

        if (atomClass)
                UnregisterClass (LPCTSTR (atomClass),
                        GetModuleHandle (0) );
        #endif
}

void graphics::idle ()
{
	if (! window_created)
		return;

        #ifdef BLASSIC_USE_X
	bool do_copy= false;
	while (XCheckWindowEvent (display, window, eventusedmask, & x_event) )
		switch (x_event.type)
		{
		case Expose:
			do_copy= true;
			break;
		case KeyPress:
			{
				XKeyEvent & xk= x_event.xkey;
				char buffer [10];
				KeySym ks;
				//XComposeStatus xcs;
				int r= XLookupString (& xk, buffer, sizeof (buffer - 1),
					& ks, /*& xcs*/ NULL);
				if (r > 0)
				{
					buffer [r]= '\0';
					queuekey.push (std::string (buffer) );
				}
				else
					queuekey.push (string_from_key (ks) );

				#if 0
				buffer [r]= '\0';
				cout << "State: " << xk.state <<
					" Keycode: " << xk.keycode;
				cout << " String: [";
				for (int i= 0; i < r; ++i)
				{
					char c= buffer [i];
					if (c < 32) cout << '^' << char (c + 'A' - 1);
					else cout << c;
				}
				cout << "] Keysym: " << ks;
				cout << endl;
				#endif

			}
			break;
		case ButtonPress:
			{
				XButtonEvent & xbpe= x_event.xbutton;
				//cerr << "ButtonPress event, button=" <<
				//	xbpe.button <<
				//	endl;
				switch (xbpe.button)
				{
				case 1:
					queuekey.push (strCLICK);
					break;
				case 3:
					queuekey.push (strSCLICK);
					break;
				default:
					;
				}
			}
			break;
		case ButtonRelease:
			{
				XButtonEvent & xbpe= x_event.xbutton;
				//cerr << "ButtonRelease event, button=" <<
				//	xbpe.button <<
				//	endl;
				switch (xbpe.button)
				{
				case 1:
					queuekey.push (strRELEASE);
					break;
				case 3:
					queuekey.push (strSRELEASE);
					break;
				default:
					;
				}
			}
			break;
		case MotionNotify:
			{
				XMotionEvent & xme= x_event.xmotion;
				//cerr << "MotionNotify event " <<
				//	xme.x << ", " << xme.y <<
				//	endl;
				xmousepos= xme.x;
				ymousepos= xme.y;
			}
			break;
		case EnterNotify:
			{
				XCrossingEvent  & xce= x_event.xcrossing;
				//cerr << "EnterNotify event" << endl;
				xmousepos= xce.x;
				ymousepos= xce.y;
			}
			break;
		default:
			//cerr << "Another event." << endl;
			;
		}
	if (do_copy && pixmap_created)
	{
		#if 0
		XCopyArea (display, pixmap, window, gc,
			0, 0, screenwidth, screenheight, 0, 0);
		XFlush (display);
		#else
		//XSetFunction (display, gc, GXcopy);
		reinit_window ();
		//XSetFunction (display, gc, drawmode);
		#endif
		//cerr << "Copied." << endl;
	}
        #endif

        #ifdef BLASSIC_USE_WINDOWS
	//Sleep (0);
        #endif

}

#include "charset.h"

namespace {

void activecolor (pcolor pxc)
{
	#ifdef BLASSIC_USE_X
	XSetForeground (display, gcp, pxc->pixel);
	XSetForeground (display, gc, pxc->pixel);
	#endif

	#ifdef BLASSIC_USE_WINDOWS
	//HDC hdc= GetDC (window);
	SelectObject (hdc, * pxc);
	SelectObject (hdcPixmap, *pxc);
	//ReleaseDC (window, hdc);
	#endif

}

void textscroll ()
{
	#ifdef BLASSIC_USE_SVGALIB
	if (svgalib)
	{
		// PENDIENTE
		return;
	}
	#endif

	#ifdef BLASSIC_USE_X
	int h= screenheight - 8;
	//unsigned long white= WhitePixel (display, screen),
	//	black= BlackPixel (display, screen);

	//XSetForeground (display, gcp, white);
	XSetFunction (display, gcp, GXcopy);
	XCopyArea (display, pixmap, pixmap, gcp,
		0, 8, screenwidth, h, 0, 0);
	activecolor (pbackground);
	XFillRectangle (display, pixmap, gcp,
		0, h, screenwidth, 8);
	//XSetForeground (display, gcp, black);
	activecolor (pforeground);
	XSetFunction (display, gcp, drawmode);

	#if 0
	XSetForeground (display, gc, white);
	XCopyArea (display, window, window, gc,
		0, 8, screenwidth, h, 0, 0);
	XFillRectangle (display, window, gc,
		0, h, screenwidth, 8);
	XSetForeground (display, gc, black);
	#else
	if (! fSynchro)
		reinit_window ();
	#endif

	#endif

        #ifdef BLASSIC_USE_WINDOWS
        RECT r = { 0, screenheight - 8, screenwidth, screenheight };

        int h= screenheight - 8;
        BitBlt (hdcPixmap, 0, 0, screenwidth, h,
                hdcPixmap, 0, 8, SRCCOPY);
        //HBRUSH hbrush= (HBRUSH) GetStockObject (WHITE_BRUSH);
	LOGPEN logpen;
	GetObject (* pbackground, sizeof (LOGPEN), & logpen);
	HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
        FillRect (hdcPixmap, & r, hbrush);
	DeleteObject (hbrush);

        #if 1
	if (! fSynchro)
		reinit_window ();
        #else
        //HDC hdc= GetDC (window);
        BitBlt (hdc, 0, 0, screenwidth, h,
                hdc, 0, 8, SRCCOPY);
        FillRect (hdc, & r, hbrush);
        //ReleaseDC (window, hdc);
        #endif

        #endif

}

#ifdef BLASSIC_USE_WINDOWS

inline void do_plot_win (HDC hdc, int x, int y)
{
	if (! fSynchro)
	{
		MoveToEx (hdc, x, y, 0);
		LineTo (hdc, x + 1, y);
	}
	MoveToEx (hdcPixmap, x, y, 0);
	LineTo (hdcPixmap, x + 1, y);
}

#endif

inline void do_plot (int x, int y)
{
	#ifdef BLASSIC_USE_SVGALIB

	if (svgalib)
	{
		vga_drawpixel (x, y);
		return;
	}

	#endif

	#ifdef BLASSIC_USE_X

	if (! fSynchro)
		XDrawPoint (display, window, gc, x, y);
	XDrawPoint (display, pixmap, gcp, x, y);

	#endif

        #ifdef BLASSIC_USE_WINDOWS

        //HDC hdc= GetDC (window);
        //SetROP2 (hdc, drawmode);
        //MoveToEx (hdc, x, y, 0);
        //LineTo (hdc, x + 1, y);
        //SetROP2 (hdcPixmap, drawmode);
        //MoveToEx (hdcPixmap, x, y, 0);
        //LineTo (hdcPixmap, x + 1, y);
        do_plot_win (hdc, x, y);
        //ReleaseDC (window, hdc);

        #endif
}

void print (int col, int row, unsigned char c)
{
	static unsigned char mask [8]= { 128, 64, 32, 16, 8, 4, 2, 1 };
	int x= col * 8;
	int y= row * 8;
	charset::chardata & data= charset::data [c];
        #ifdef BLASSIC_USE_WINDOWS
        //HDC hdc= GetDC (window);
        //SetROP2 (hdc, drawmode);
        //SetROP2 (hdcPixmap, drawmode);
        #endif
	for (int i= 0, yi= y; i < 8; ++i, ++yi)
	{
                unsigned char c= data [i];
		for (int j= 0, xj= x; j < 8; ++j, ++xj)
		{
			bool f= (c & mask [j] ) != 0;
			if (f || opaquemode)
			{
				pcolor pc= f ? pforeground : pbackground;
				activecolor (pc);
                                #ifdef BLASSIC_USE_WINDOWS
                                do_plot_win (hdc, xj, yi);
                                #else
				do_plot (xj, yi);
                                #endif
			}
		}
	}
	activecolor (pforeground);
        #ifdef BLASSIC_USE_WINDOWS
        //ReleaseDC (window, hdc);
        #endif
}

int tcol, trow;
int maxtcol, maxtrow;

const int text_mode= 0, user_mode= -1;

int actualmode= text_mode;

void setmaxtext ()
{
        maxtcol= screenwidth / 8;
        maxtrow= screenheight / 8;
}

void setmode (int width, int height, int mode)
{
	sysvar::set16 (sysvar::GraphicsWidth, short (width) );
	sysvar::set16 (sysvar::GraphicsHeight, short (height) );
	screenwidth= width;
	screenheight= height;

	#ifdef BLASSIC_USE_SVGALIB

	if (svgalib) {
		if (mode == user_mode)
			throw ErrFunctionCall;
		if (actualmode != text_mode)
		{
			free (font);
		}
		vga_setmode (mode);
		if (mode != text_mode) {
                        setmaxtext ();
			gl_setcontextvga (mode);
			//font= new char [256 * 8 * 8 * BYTESPERPIXEL];
			font= (char *) malloc (2 * 256 * 8 * 8 * BYTESPERPIXEL);
			gl_expandfont (8, 8, 15, gl_font8x8, font);
			gl_setfont (8, 8, font);

			#ifdef TEST_TEXT
			//cout << "Preparado" << endl;
			static char prueba []= "Prueba";
			gl_write (10, 10, prueba);
			#endif

			//cout << "Listo" << endl;
		}
		actualmode= mode;
		return;
	}

	#endif

	if (mode != actualmode)
	{
		if (window_created)
			destroy_window ();
		if (mode != text_mode)
			create_window (width, height);
	}
	else
	{
		if (mode != text_mode)
		{
                        graphics::setcolor (0);
                        graphics::setdrawmode (0);
			reinit_pixmap ();
			reinit_window ();
		}

		#ifdef _Windows
		else
			if (actualmode != text_mode)
			// REVISAR: ESTO NO PARECE VALIDO.
				destroy_thread ();
		#endif
	}

        actualmode= mode;

	if (mode != text_mode)
	{
                setmaxtext ();
		tcol= trow= 0;
		#if defined (BLASSIC_USE_WINDOWS) || defined (BLASSIC_USE_X)
		pforeground= & xcBlack;
		pbackground= & xcWhite;
		#endif
	}

	#ifdef TEST_TEXT
	if (mode != 0)
	{
		XSetBackground (display, gcp, xcWhite.pixel);
		XSetBackground (display, gc, xcWhite.pixel);
		draw_string ("Prueba");
	}
	#endif
}

} // namespace

void graphics::cls ()
{

	#ifdef BLASSIC_USE_SVGALIB

	if (svgalib)
	{
		// PENDIENTE
		return;
	}

	#endif

        tcol= trow= 0;
	//reinit_pixmap ();

	#ifdef BLASSIC_USE_WINDOWS

	RECT r= { 0, 0, screenwidth, screenheight };
	//HDC hdc= GetDC (window);
        LOGPEN logpen;
        GetObject (* pbackground, sizeof (LOGPEN), & logpen);
        HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
	if (! fSynchro)
		FillRect (hdc, & r, hbrush);
        FillRect (hdcPixmap, & r, hbrush);
        DeleteObject (hbrush);
	//ReleaseDC (window, hdc);

	#endif

	#ifdef BLASSIC_USE_X

	//XSetForeground (display, gc, WhitePixel (display, screen) );
	activecolor (pbackground);
	XSetFunction (display, gc, GXcopy);
	XSetFunction (display, gcp, GXcopy);
	if (! fSynchro)
		XFillRectangle (display, window, gc,
			0, 0, screenwidth, screenheight);
	XFillRectangle (display, pixmap, gcp,
		0, 0, screenwidth, screenheight);
	//XSetForeground (display, gc, BlackPixel (display, screen) );
	activecolor (pforeground);
	XSetFunction (display, gc, drawmode);
	XSetFunction (display, gcp, drawmode);

	#endif

}

void graphics::setmode (int width, int height)
{
	::setmode (width, height, user_mode);
}

void graphics::setmode (int mode)
{
	if (! inited)
		throw ErrFunctionCall;
	lastx= lasty= 0;

	int width, height;
	switch (mode) {
	case 1:
		width= 320; height= 200; break;
	case 2:
		width= 640; height= 200; break;
	case 5:
		width= 320; height= 200; break;
	case 10:
		width= 640; height= 480; break;
	case 11:
		width= 800; height= 600; break;
	default:
                width= 0; height= 0;
	}
	if (mode != 0 && width == 0)
		throw ErrFunctionCall;

	::setmode (width, height, mode);
}

bool graphics::ingraphicsmode ()
{
        return actualmode != text_mode;
}

namespace {

inline pcolor mapcolor (int color)
{
	switch (color)
	{
	case  0: return & xcBlack;
	case  1: return & xcBlue;
	case  2: return & xcGreen;
	case  3: return & xcCyan;
	case  4: return & xcRed;
	case  5: return & xcMagenta;
	case  6: return & xcBrown;
	case  7: return & xcLightGrey;
	case  8: return & xcDarkGrey;
	case  9: return & xcLightBlue;
	case 10: return & xcLightGreen;
	case 11: return & xcLightCyan;
	case 12: return & xcLightRed;
	case 13: return & xcLightMagenta;
	case 14: return & xcYellow;
	case 15: return & xcWhite;
	default: return & xcBlack;
	}
}

} // namespace

void graphics::setcolor (int color)
{
	if (! inited) return;

        #ifdef BLASSIC_USE_SVGALIB

	if (svgalib) {
		vga_setcolor (color);
		return;
	}

	#endif

	pcolor pxc= mapcolor (color);

	#if 0

	#ifdef BLASSIC_USE_X
	XSetForeground (display, gcp, pxc->pixel);
	XSetForeground (display, gc, pxc->pixel);
	#endif

	#ifdef BLASSIC_USE_WINDOWS
	//HDC hdc= GetDC (window);
	SelectObject (hdc, * pxc);
	SelectObject (hdcPixmap, *pxc);
	//ReleaseDC (window, hdc);
	#endif

	#else

	activecolor (pxc);

	#endif

	#if defined (BLASSIC_USE_WINDOWS) || defined (BLASSIC_USE_X)
	pforeground= pxc;
	#endif
}

void graphics::setbackground (int color)
{
	if (! inited) return;
	pcolor pxc= mapcolor (color);
	pbackground= pxc;
}

void graphics::settransparent (int transpmode)
{
	if (! inited) return;
	opaquemode= ! (transpmode & 1);
}

void graphics::setdrawmode (int mode)
{
        #if defined BLASSIC_USE_X
        int modes []= { GXcopy, GXinvert };
        #elif defined BLASSIC_USE_WINDOWS
        int modes []= { R2_COPYPEN, R2_NOT };
        #endif

        if (mode < 0 || size_t (mode) >= util::dim_array (modes) )
                return;
        drawmode= modes [mode];

        #ifdef BLASSIC_USE_X
	XSetFunction (display, gc, drawmode);
	XSetFunction (display, gcp, drawmode);
        #endif

        #ifdef BLASSIC_USE_WINDOWS
        //HDC hdc= GetDC (window);
        SetROP2 (hdc, drawmode);
        //ReleaseDC (window, hdc);
        SetROP2 (hdcPixmap, drawmode);
        #endif
}

void graphics::line (int x, int y)
{
	if (! inited) return;
	const int prevx= lastx, prevy= lasty;
	lastx= x; lasty= y;

	#ifdef BLASSIC_USE_SVGALIB
	if (svgalib)
	{
		vga_drawline (prevx, prevy, x, y);
		return;
	}
	#endif

	#ifdef BLASSIC_USE_X
	if (! fSynchro)
		XDrawLine (display, window, gc, prevx, prevy, x, y);
	XDrawLine (display, pixmap, gcp, prevx, prevy, x, y);
	XFlush (display);
	#endif

        #ifdef BLASSIC_USE_WINDOWS
        //HDC hdc= GetDC (window);

        //SetROP2 (hdc, drawmode);
	if (! fSynchro)
	{
		MoveToEx (hdc, prevx, prevy, 0);
		LineTo (hdc, x, y);
	}

        //ReleaseDC (window, hdc);

        //SetROP2 (hdcPixmap, drawmode);
        MoveToEx (hdcPixmap, prevx, prevy, 0);
        LineTo (hdcPixmap, x, y);
        #endif

	idle ();
}

void graphics::move (int x, int y)
{
	lastx= x; lasty= y;
	idle ();
}

void graphics::plot (int x, int y)
{
	lastx= x; lasty= y;

	#if 0

	#ifdef BLASSIC_USE_SVGALIB

	if (svgalib) {
		vga_drawpixel (x, y);
		return;
	}

	#endif

	#ifdef BLASSIC_USE_X

	if (! fSynchro)
		XDrawPoint (display, window, gc, x, y);
	XDrawPoint (display, pixmap, gcp, x, y);

	#endif

        #ifdef BLASSIC_USE_WINDOWS

        //HDC hdc= GetDC (window);

        //SetROP2 (hdc, drawmode);
	if (! fSynchro)
	{
		MoveToEx (hdc, x, y, 0);
		LineTo (hdc, x + 1, y);
	}

        //ReleaseDC (window, hdc);

        //SetROP2 (hdcPixmap, drawmode);
        MoveToEx (hdcPixmap, x, y, 0);
        LineTo (hdcPixmap, x + 1, y);

        #endif

	#else

	do_plot (x, y);

	#endif

	idle ();
}

namespace {

void line_to_point (const graphics::Point & p)
{
	graphics::line (p.x, p.y);
}

} // namespace

void graphics::plot (std::vector <Point> & points)
{
	ASSERT (points.size () != 0);
	lastx= points [0].x;
	lasty= points [0].y;
	std::for_each (points.begin () + 1, points.end (), line_to_point);
}

graphics::Point graphics::getlast ()
{
	return Point (lastx, lasty);
}

std::string graphics::getkey ()
{
        idle ();
        if (queuekey.empty () )
                return std::string ();
        #if 0
        std::string str= queuekey.front ();
        queuekey.pop ();
        return str;
        #else
        return queuekey.pop ();
        #endif
}

#ifdef TEST_TEXT

void draw_string (const std::string & str)
{
	XDrawImageString (display, window, gc, 10, 10,
		str.c_str (), str.size () );
	XDrawImageString (display, pixmap, gcp, 10, 10,
		str.c_str (), str.size () );
}

#endif

namespace {

int zone= 8;

inline void do_charout (char c)
{
	switch (c)
	{
	case '\n':
                tcol= 0;
                if (++trow >= maxtrow)
		{
			textscroll ();
			trow= maxtrow - 1;
		}
		return;
	case '\r':
		tcol= 0;
		return;
	case '\t':
		if (tcol >= (maxtcol / zone) * zone)
		{
			//cerr << "Fin de linea" << endl;
			for ( ; tcol < maxtcol; ++tcol)
				print (tcol, trow, ' ');
			tcol= 0;
			if (++trow >= maxtrow)
			{
				textscroll ();
				trow= maxtrow - 1;
			}
		}
		else
		{
			do {
				print (tcol, trow, ' ');
				++tcol;
			} while (tcol % zone);
		}
		return;
	}
        print (tcol, trow, c);
        if (++tcol >= maxtcol)
        {
                tcol= 0;
                if (++trow >= maxtrow)
		{
			textscroll ();
			trow= maxtrow - 1;
		}
        }
}

} // namespace

size_t graphics::getlinewidth ()
{
	return maxtcol;
}

void graphics::charout (char c)
{
	do_charout (c);
	idle ();
}

void graphics::stringout (const std::string & str)
{
	std::for_each (str.begin (), str.end (), do_charout);
	idle ();
}

void graphics::locate (int row, int col)
{
        trow= row - 1;
        tcol= col - 1;
}

void graphics::tab (size_t n)
{
	int col (n - 1);
	if (tcol >= col)
	{
		do {
			do_charout (' ');
		} while (tcol > 0);
	}
        if (col >= maxtcol)
                throw ErrFunctionCall;
	do {
		do_charout (' ');
	} while (tcol < col);
}

void graphics::movecharforward (size_t n)
{
	for (size_t i= 0; i < n; ++i)
	{
		if (++tcol >= maxtcol)
		{
			tcol= 0;
			if (++trow >= maxtrow)
			{
				textscroll ();
				trow= maxtrow - 1;
			}
		}
	}
}

void graphics::movecharback (size_t n)
{
	for (size_t i=0; i < n; ++i)
	{
		if (tcol > 0)
			--tcol;
		else
		{
			if (trow > 0)
			{
				tcol= maxtcol - 1;
				--trow;
			}
		}
	}
}

void graphics::movecharup (size_t n)
{
	trow-= n;
}

void graphics::movechardown (size_t n)
{
	trow+= n;
}

void graphics::definesymbol (int symbol, const unsigned char (& byte) [8] )
{
	if (symbol < 0 || symbol > 255)
		throw ErrFunctionCall;
	memcpy (charset::data + symbol, byte, sizeof (byte) );
}

void graphics::synchronize (bool mode)
{
	fSynchro= mode;
}

void graphics::synchronize ()
{
	reinit_window ();
}

int graphics::xmouse () { return xmousepos; }
int graphics::ymouse () { return ymousepos; }

// Fin de graphics.cpp
