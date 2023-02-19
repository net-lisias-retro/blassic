// graphics.cpp
// Revision 21-may-2003

#ifdef __BORLANDC__
#pragma warn -8027
#endif

#include "graphics.h"
#include "sysvar.h"
#include "error.h"
#include "var.h"
//#include "cursor.h"
#include "key.h"
#include "charset.h"
#include "util.h"
#include "trace.h"

// Para depuracion
#include <iostream>
using std::cerr;
using std::endl;
#include <cassert>
#define ASSERT assert

#include <map>
#include <queue>
#include <cmath>

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

pcolor pforeground, pbackground,
	default_foreground, default_background;

#endif

#include <string>
#include <algorithm>

// Character set

charset::chardata charset::data [256];

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

const int text_mode= 0, user_mode= -1;

int actualmode= text_mode;

inline void requiregraphics ()
{
	if (actualmode == text_mode)
		throw ErrNoGraphics;
}

#ifdef BLASSIC_USE_SVGALIB

bool svgalib= false;

#else

//const bool svgalib= false;

#endif

int screenwidth, screenheight;
int lastx, lasty;

#if defined BLASSIC_USE_X

static const int
	drawmode_copy= GXcopy,
	drawmode_xor= GXxor,
	drawmode_and= GXand,
	drawmode_or= GXor,
	drawmode_invert= GXinvert;

#elif defined BLASSIC_USE_WINDOWS

static const int
	drawmode_copy= R2_COPYPEN,
	drawmode_xor= R2_XORPEN,
	// Revisar los valores para and y or.
	drawmode_and= R2_MASKPEN,
	drawmode_or= R2_MERGEPEN,
	drawmode_invert= R2_NOT;

#endif

int drawmode= drawmode_copy;

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
	XSetFunction (display, gc, drawmode_copy);
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
	#if 1
	window= XCreateSimpleWindow (display,
		RootWindow (display, screen),
		0, 0, width, height,
		5, BlackPixel (display, screen),
		WhitePixel (display, screen) );
	#else
	int depth= 8;
	window= XCreateWindow (display,
		RootWindow (display, screen),
		0, 0, width, height,
		5,
		depth,
		InputOutput,
		CopyFromParent,
		0,
		NULL);
	#endif
	window_created= true;

	#if 1
	int depth= DefaultDepth (display, DefaultScreen (display) );
	#endif
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
		#if 0

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

		#else

		{ &xcBlack,           0,    0,    0 },
		{ &xcBlue,            0,    0, 0xA8 },
		{ &xcGreen,           0, 0xA8,    0 },
		{ &xcCyan,            0, 0xA8, 0xA8 },
		{ &xcRed,          0xA8,    0,    0 },
		{ &xcMagenta,      0xA8,    0, 0xA8 },
		{ &xcBrown,        0xA8, 0x54,    0 },
		{ &xcLightGrey,    0xA8, 0xA8, 0xA8 },

		{ &xcDarkGrey,     0x54, 0x54, 0x54 },
		{ &xcLightBlue,    0x54, 0x54, 0xFF },
		{ &xcLightGreen,   0x54, 0xFF, 0x54 },
		{ &xcLightCyan,    0x54, 0xFF, 0xFF },
		{ &xcLightRed,     0xFF, 0x54, 0x54 },
		{ &xcLightMagenta, 0xFF, 0x54, 0xFF },
		{ &xcYellow,       0xFF, 0xFF, 0x54 },
		{ &xcWhite,        0xFF, 0xFF, 0xFF }

		#endif
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

		#if 0

		//{ &xcBlack,        "black" },
		{ &xcBlack,        "#000000" },
		{ &xcBlue,         "darkblue" },
		{ &xcGreen,        "darkgreen" },
		{ &xcCyan,         "darkcyan" },
		{ &xcRed,          "darkred" },
		{ &xcMagenta,      "darkmagenta" },
		//{ &xcBrown,        "#7F7F00" },
		{ &xcBrown,        "brown" },
		//{ &xcLightGrey,    "#3F3F3F" },
		//{ &xcLightGrey,    "rgb:C0/C0/C0" },
		{ &xcLightGrey,    "grey" },

	        { &xcDarkGrey,     "darkgrey" },
		//{ &xcLightBlue,    "#0000FF" },
		{ &xcLightBlue,    "rgb:80/FF/FF" },
		{ &xcLightGreen,   "#00FF00" },
		{ &xcLightCyan,    "#00FFFF" },
		{ &xcLightRed,     "#FF0000" },
		{ &xcLightMagenta, "#FF00FF" },
		{ &xcYellow,       "yellow" },
		//{ &xcWhite,        "white" }
		{ &xcWhite,        "#FFFFFF" }

		#else

		{ &xcBlack,        "rgb:00/00/00" },
		{ &xcBlue,         "rgb:00/00/A8" },
		{ &xcGreen,        "rgb:00/A8/00" },
		{ &xcCyan,         "rgb:00/A8/A8" },
		{ &xcRed,          "rgb:A8/00/00" },
		{ &xcMagenta,      "rgb:A8/00/A8" },
		{ &xcBrown,        "rgb:A8/54/00" },
		{ &xcLightGrey,    "rgb:A8/A8/A8" },

		{ &xcDarkGrey,     "rgb:54/54/54" },
		{ &xcLightBlue,    "rgb:54/54/FF" },
		{ &xcLightGreen,   "rgb:54/FF/54" },
		{ &xcLightCyan,    "rgb:54/FF/FF" },
		{ &xcLightRed,     "rgb:FF/54/54" },
		{ &xcLightMagenta, "rgb:FF/54/FF" },
		{ &xcYellow,       "rgb:FF/FF/54" },
		{ &xcWhite,        "rgb:FF/FF/FF" }

		#endif

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
	TraceFunc tr ("graphics::initialize");

	// Default symbol after and charset initialization:
	symbolafter (240);

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
	const char * strDisplay;
	if ( (strDisplay= getenv ("DISPLAY") ) != NULL &&
		strDisplay [0] != '\0')
	{
		tr.message (std::string ("Opening ") + strDisplay);
		display= XOpenDisplay (0);
		if (display)
		{
			tr.message ("Display opened");
			inited= true;
			screen= DefaultScreen (display);
			init_xcolors ();
		}
		else
		{
			tr.message ("Error opening display");
			cerr << "Error al abrir display." << endl;
		}
	}
	else
	{
		tr.message ("No DISPLAY value");
		cerr << "Sin soporte de graficos." << endl;
	}
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
	TraceFunc tr ("graphics::uninitialize");

	if (! inited) return;

	if (actualmode != 0)
		setmode (0);

	#ifdef BLASSIC_USE_SVGA
	#if 0
	if (svgalib)
		//vga_setmode (TEXT);
		setmode (0);
	#endif
	#endif

	#ifdef BLASSIC_USE_X
	if (display)
	{
		tr.message ("closing display");
		//if (window_created)
		//	destroy_window ();
		XCloseDisplay (display);
		tr.message ("display is closed");
	}
	#endif

        #ifdef BLASSIC_USE_WINDOWS
        //destroy_thread ();
        //if (window_created)
        //        destroy_window ();

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
		//XSetFunction (display, gc, drawmode_copy);
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
	XSetFunction (display, gcp, drawmode_copy);
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

#if 0
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

        //do_plot_win (hdc, x, y);

	if (! fSynchro)
	{
		MoveToEx (hdc, x, y, 0);
		LineTo (hdc, x + 1, y);
	}
	MoveToEx (hdcPixmap, x, y, 0);
	LineTo (hdcPixmap, x + 1, y);

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
                                //#ifdef BLASSIC_USE_WINDOWS
                                //do_plot_win (hdc, xj, yi);
                                //#else
				do_plot (xj, yi);
                                //#endif
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

void setmaxtext ()
{
        maxtcol= screenwidth / 8;
        maxtrow= screenheight / 8;
}

enum TransformType { TransformIdentity, TransformInvertY };

TransformType activetransform= TransformIdentity;

void transform_y (int & y)
{
	switch (activetransform)
	{
	case TransformIdentity:
		break; // Nothing to do
	case TransformInvertY:
		y= screenheight - y;
		break;
	}
}

void recreate_windows ();

void setmode (int width, int height, int mode)
{
	TraceFunc tr ("setmode");

	sysvar::set16 (sysvar::GraphicsWidth, short (width) );
	sysvar::set16 (sysvar::GraphicsHeight, short (height) );
	screenwidth= width;
	screenheight= height;
	activetransform= TransformIdentity;
	fSynchro= false;

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
			//cout << "Listo" << endl;
		}
		actualmode= mode;
		return;
	}

	#endif

	if (mode != actualmode || mode == user_mode)
	{
		{
			std::ostringstream oss;
			oss << "Changing mode from " << actualmode <<
				" to " << mode;
			tr.message (oss.str () );
		}
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
		lastx= lasty= 0;
		#if defined (BLASSIC_USE_WINDOWS) || defined (BLASSIC_USE_X)
		//pforeground= & xcBlack;
		//pbackground= & xcWhite;
		pforeground= default_foreground;
		pbackground= default_background;
		#endif
		recreate_windows ();
	}
}

} // namespace

void graphics::cls ()
{
	requiregraphics ();

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
        LOGPEN logpen;
        GetObject (* pbackground, sizeof (LOGPEN), & logpen);
        HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
	if (! fSynchro)
		FillRect (hdc, & r, hbrush);
        FillRect (hdcPixmap, & r, hbrush);
        DeleteObject (hbrush);
	#endif

	#ifdef BLASSIC_USE_X
	activecolor (pbackground);
	XSetFunction (display, gcp, drawmode_copy);
	XFillRectangle (display, pixmap, gcp,
		0, 0, screenwidth, screenheight);
	XSetFunction (display, gcp, drawmode);
	if (! fSynchro)
	{
		XSetFunction (display, gc, drawmode_copy);
		XFillRectangle (display, window, gc,
			0, 0, screenwidth, screenheight);
		XSetFunction (display, gc, drawmode);
		// Inserted an idle call because without it
		// the window sometimes is not updated.
		graphics::idle ();
	}
	activecolor (pforeground);
	#endif

}

void graphics::setmode (int width, int height, bool inverty)
{
	default_foreground= & xcBlack;
	default_background= & xcWhite;
	::setmode (width, height, user_mode);
	if (inverty)
		activetransform= TransformInvertY;
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

	default_foreground= & xcBlack;
	default_background= & xcWhite;
	::setmode (width, height, mode);
}

void graphics::setmode (const std::string & mode)
{
	if (mode == "spectrum")
	{
		// Revisar colores.
		default_foreground= & xcBlack;
		default_background= & xcWhite;
		::setmode (256, 176, user_mode);
		activetransform= TransformInvertY;
	}
	#if 0
	// Pendiente de pensarlo mejor.
	else if (mode == "cpc0")
	{
		::setmode (160, 400, user_mode);
		activetransform= TransformInvertY;
	}
	else if (mode == "cpc1")
	{
		::setmode (320, 400, user_mode);
		activetransform= TransformInvertY;
	}
	#endif
	else if (mode == "cpc2")
	{
		default_foreground= & xcYellow;
		default_background= & xcBlue;
		::setmode (640, 400, user_mode);
		activetransform= TransformInvertY;
	}
	else
		throw ErrFunctionCall;
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
	//if (! inited) return;

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
	//if (! inited) return;
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
	if (! inited) return;

	// Draw modes:
	// 0: normal copy mode.
	// 1: XOR
	// 2: AND
	// 3: OR
	// 0 to 3 are Amstrad CPC modes.
	// 4: INVERT, NOT.
	static int modes []= { drawmode_copy, drawmode_xor,
		drawmode_and, drawmode_or, drawmode_invert };

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

namespace {

void do_line_unmasked (int x, int y)
{
	int prevx= lastx, prevy= lasty;
	lastx= x; lasty= y;

	transform_y (y); transform_y (prevy);

	activecolor (pforeground);

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

	//idle ();
}

const unsigned char maskvaluedefault= '\xFF';
unsigned char maskvalue= maskvaluedefault;
bool maskdrawfirst= true;
unsigned maskpos= 0;
unsigned char auxmask []= { 1, 2, 4, 8, 16, 32, 64, 128 };

inline void impl_plot_mask (int x, int y)
{
	if (maskvalue == maskvaluedefault)
		do_plot (x, y);
	else
	{
		if (maskvalue & auxmask [maskpos] )
			do_plot (x, y);
		if (++maskpos == 8) maskpos= 0;
	}
}

void do_line_mask (int x, int y)
{
	int prevx= lastx, prevy= lasty;
	lastx= x; lasty= y;

	transform_y (prevy); transform_y (y);

	activecolor (pforeground);

	int px= x - prevx;
	int py= y - prevy;
	int d1x= px < 0 ? -1 : px > 0 ? 1 : 0;
	int d1y= py < 0 ? -1 : py > 0 ? 1 : 0;
	int d2x= d1x;
	int d2y= 0;
	int m= abs (px);
	int n= abs (py);
	if (m <= n)
	{
		d2x= 0;
		d2y= d1y;
		m= abs (py);
		n= abs (px);
	}
	int s= m / 2;
	for (int i= 0; i <= m; ++i)
	{
		if (i != 0 || maskdrawfirst)
		{
			//if (maskvalue & auxmask [maskpos] )
			//	do_plot (prevx, prevy);
			//if (++maskpos == 8) maskpos= 0;
			impl_plot_mask (prevx, prevy);
		}
		s+= n;
		if (s >= m)
		{
			s-= m;
			prevx+= d1x;
			prevy+= d1y;
		}
		else
		{
			prevx+= d2x;
			prevy+= d2y;
		}
	}
}

inline void do_line (int x, int y)
{
	if (maskvalue == maskvaluedefault && maskdrawfirst)
		do_line_unmasked (x, y);
	else
		do_line_mask (x, y);
}

} // namespace

void graphics::line (int x, int y)
{
	requiregraphics ();
	do_line (x, y);
}

void graphics::liner (int x, int y)
{
	line (lastx + x, lasty + y);
}

void graphics::rectangle (Point org, Point dest)
{
	int x1= org.x;
	int y1= org.y;
	int x2= dest.x;
	int y2= dest.y;
	lastx= x2; lasty= y2;
	transform_y (y1); transform_y (y2);
	if (x1 > x2) std::swap (x1, x2);
	if (y1 > y2) std::swap (y1, y2);

        #ifdef BLASSIC_USE_WINDOWS
        Rectangle (hdcPixmap, x1, y1, x2 + 1, y2 + 1);
        if (! fSynchro)
                Rectangle (hdc, x1, y1, x2 + 1, y2 + 1);
        #endif

        #ifdef BLASSIC_USE_X
	XDrawRectangle (display, pixmap, gcp,
		x1, y1, x2 - x1, y2 - y1);
	if (! fSynchro)
		XDrawRectangle (display, window, gc,
			x1, y1, x2 - x1, y2 - y1);
	#endif
}

void graphics::rectanglefilled (Point org, Point dest)
{
	int x1= org.x;
	int y1= org.y;
	int x2= dest.x;
	int y2= dest.y;
	lastx= x2; lasty= y2;
	transform_y (y1); transform_y (y2);
	if (x1 > x2) std::swap (x1, x2);
	if (y1 > y2) std::swap (y1, y2);

        #ifdef BLASSIC_USE_WINDOWS
        RECT r = { x1, y1, x2 + 1, y2 + 1 };
	LOGPEN logpen;
	GetObject (* pforeground, sizeof (LOGPEN), & logpen);
	HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
        FillRect (hdcPixmap, & r, hbrush);
        if (! fSynchro)
                FillRect (hdc, & r, hbrush);
        DeleteObject (hbrush);
        #endif

        #ifdef BLASSIC_USE_X
	XFillRectangle (display, pixmap, gcp,
		x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	if (! fSynchro)
		XFillRectangle (display, window, gc,
			x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	#endif
}

void graphics::move (int x, int y)
{
	requiregraphics ();
	lastx= x; lasty= y;
	//idle ();
}

void graphics::mover (int x, int y)
{
	requiregraphics ();
	lastx+= x; lasty+= y;
	//idle ();
}

void graphics::plot (int x, int y)
{
	requiregraphics ();
	lastx= x; lasty= y;

	transform_y (y);

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

	activecolor (pforeground);
	do_plot (x, y);

	#endif

	//idle ();
}

void graphics::plotr (int x, int y)
{
	plot (lastx + x, lasty + y);
}

namespace {

void line_to_point (const graphics::Point & p)
{
	graphics::line (p.x, p.y);
}

} // namespace

void graphics::plot (std::vector <Point> & points)
{
	requiregraphics ();
	ASSERT (points.size () != 0);
	lastx= points [0].x;
	lasty= points [0].y;
	std::for_each (points.begin () + 1, points.end (), line_to_point);
}

//#define DEBUG_CIRCLE

#ifdef DEBUG_CIRCLE

#include <unistd.h>

namespace { inline void pausec () { usleep (100000); } }

#else

namespace { inline void pausec () { } }

#endif

void graphics::circle (int x, int y, int radius)
{
	requiregraphics ();
	activecolor (pforeground);

	lastx= x + radius;
	lasty= y;

	transform_y (y);

	if (radius == 0)
	{
		impl_plot_mask (x, y);
		return;
	}

	// sq2_2 is sin (pi/4)
	#ifdef M_SQRT2
	static const double sq2_2= M_SQRT2 / 2.0;
	#else
	static const double sq2_2= sqrt (2.0) / 2.0;
	#endif
	//int r= int (radius * sq2_2 + .5) + 1;
	int r= int (radius * sq2_2 + 1.0);
	int rr= int (sqrt (radius * radius - r * r) + .5);
	int dim= r;
	if (rr >= r) ++dim;

	#ifdef DEBUG_CIRCLE
	cerr << "Circle: " << radius << ", " << r << endl;
	#endif

	util::auto_buffer <int> p (dim);

	// Bresenham algorithm.
	for (int i= 0, j= radius, d= 1 - radius; i < dim; ++i)
	{
		p [i]= j;
		if (d < 0)
			d+= 2 * i + 3;
		else
		{
			d+= 2 * (i - j) + 5;
			--j;
		}
	}

	rr= p [r - 1] - 1;
	ASSERT (rr <= dim - 1);

	// The first point in each quadrant is plotted independently.
	// In the first quadrant is omitted, we plot it at the end.
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (4);
	#endif
	for (int j= 1; j < r; ++j)
	{
		//do_line (x + p [j], y - j);
		impl_plot_mask (x + p [j], y - j);
		pausec ();
	}
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (0);
	#endif
	for (int i= rr; i > 0; --i)
	{
		//do_line (x + i, y - p [i] );
		impl_plot_mask (x + i, y - p [i] );
		pausec ();
	}

	//do_line (x, y - radius);
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (4);
	#endif
	impl_plot_mask (x, y - radius);
	for (int i= 1; i < r; ++i)
	{
		//do_line (x - i, y - p [i] );
		impl_plot_mask (x - i, y - p [i] );
		pausec ();
	}
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (0);
	#endif
	for (int j= rr; j > 0; --j)
	{
		//do_line (x - p [j], y - j);
		impl_plot_mask (x - p [j], y - j);
		pausec ();
	}

	//do_line (x - radius, y);
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (4);
	#endif
	impl_plot_mask (x - radius, y);
	for (int j= 1; j < r; ++j)
	{
		//do_line (x - p [j], y + j);
		impl_plot_mask (x - p [j], y + j);
		pausec ();
	}
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (0);
	#endif
	for (int i= rr; i > 0; --i)
	{
		//do_line (x - i, y + p [i] );
		impl_plot_mask (x - i, y + p [i] );
		pausec ();
	}

	//do_line (x, y + radius);
	impl_plot_mask (x, y + radius);
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (4);
	#endif
	for (int i= 1; i < r; ++i)
	{
		//do_line (x + i, y + p [i] );
		impl_plot_mask (x + i, y + p [i] );
		pausec ();
	}
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (0);
	#endif
	for (int j= rr; j > 0; --j)
	{
		//do_line (x + p [j], y + j);
		impl_plot_mask (x + p [j], y + j);
		pausec ();
	}
	//do_line (x + radius, y);
	#ifdef DEBUG_CIRCLE
	graphics::setcolor (4);
	#endif
	impl_plot_mask (x + radius, y);
}

namespace {

inline void get_point_on_arc (int r, BlNumber a, int & out_x, int & out_y)
{
	BlNumber s= sin (a) * r, c= cos (a) * r;
	out_x= static_cast <int> (c < 0 ? c - .5 : c + .5);
	out_y= static_cast <int> (s < 0 ? s - .5 : s + .5);
}

inline int get_quadrant (int x, int y)
{
	if (x >= 0)
	{
		if (y >= 0)
			return 0;
		else
			return 3;
	}
	else
	{
		if (y > 0)
			return 1;
		else
			return 2;
	}
}

}

void graphics::arccircle (int x, int y, int radius,
	BlNumber arcbeg, BlNumber arcend)
{
	/*
		The code for this function and his auxiliarys
		is taken from the Allegro library.
		Many thanks.
	*/

	requiregraphics ();
	activecolor (pforeground);

	//#define DEBUG_ARCCIRCLE

	#ifdef DEBUG_ARCCIRCLE
	cerr << "arccircle: " << x << ", " << y << ", " << radius << ", " <<
		arcbeg << ", " << arcend << endl;
	#endif

	lastx= x + radius;
	lasty= y;
	transform_y (y);

	int px, py; // Current position and auxiliary.
	get_point_on_arc (radius, arcend, px, py);
	const int ex= px, ey= py; // End position.
	get_point_on_arc (radius, arcbeg, px, py);
	const int sx= px, sy= py; // Start position.
	// Current position start at start position.

	const int sq= get_quadrant (sx, sy); // Start quadrant.
	// Calculate end quadrant, considering that end point
	// must be after start point.
	int q= get_quadrant (ex, ey);
	if (sq > q)
		// Quadrant end must be greater or equal.
		q+= 4;
	else if (sq == q && arcbeg > arcend)
		// If equal, consider the angle.
		q+= 4;
	const int qe= q;
	q= sq; // Current cuadrant.
	#ifdef DEBUG_ARCCIRCLE
	cerr << "Quadrant from " << sq << " to " << qe << endl;
	#endif

	// Direction of movement.
	int dy = ( ( (q + 1) & 2) == 0) ? 1 : -1;
	int dx= ( (q & 2) == 0) ? -1 : 1;

	const int rr= radius * radius;
	int xx= px * px;
	int yy= py * py - rr;

	while (true)
	{
		// Change quadrant when needed, adjusting directions.
		if ( (q & 1) == 0)
		{
			if (px == 0)
			{
				if (qe == q)
					break;
				++q;
				dy= -dy;
			}
		}
		else
		{
			if (py == 0)
			{
				if (qe == q)
					break;
				++q;
				dx= -dx;
			}
		}
		// If we are in the end quadrant, check if at the end position.
		if (qe == q)
		{
			int det= 0;
			if (dy > 0)
			{
				if (py >= ey)
					++det;
			}
			else
			{
				if (py <= ey)
					++det;
			}
			if (dx > 0)
			{
				if (px >= ex)
					++det;
			}
			else
			{
				if (px <= ex)
					++det;
			}
			if (det == 2)
				break;
		}

		impl_plot_mask (x + px, y - py);

		int xx_new= (px + dx) * (px + dx);
		int yy_new= (py + dy) * (py + dy) - rr;
		int rr1= xx_new + yy;
		int rr2= xx_new + yy_new;
		int rr3= xx + yy_new;
		if (rr1 < 0) rr1= -rr1;
		if (rr2 < 0) rr2= -rr2;
		if (rr3 < 0) rr3= -rr3;
		if (rr3 >= std::min (rr1, rr2) )
		{
			px+= dx;
			xx= xx_new;
		}
		if (rr1 > std::min (rr2, rr3) )
		{
			py+= dy;
			yy= yy_new;
		}
	}
	if (px != sx || py != sy || sq == qe)
		impl_plot_mask (x + px, y - py);
}

void graphics::ellipse (int ox, int oy, int rx, int ry)
{
	// Based on "A fast Bresenham type algorithm
	// for drawing ellipses", by John Kennedy.

	//#define DEBUG_ELLIPSE

	#ifdef DEBUG_ELLIPSE
	cerr << "Ellipse: " << rx << ", " << ry << endl;
	#endif

	requiregraphics ();
	activecolor (pforeground);
	lastx= ox+ rx;
	lasty= oy;

	transform_y (oy);

	const int ry2= ry * ry;
	const int rx2= rx * rx;
	//const int dimy= int (ry2 / sqrt (ry2 + rx2) + 0.5);
	const int dimy= int (ry2 / sqrt (ry2 + rx2) ) + 1;
	#ifdef DEBUG_ELLIPSE
	cerr << "Dimy: " << dimy << endl;
	#endif
	util::auto_buffer <int> py (dimy);
	//#define SIMPLER
	#ifndef SIMPLER
	const int ry2_2= ry2 * 2;
	const int rx2_2= rx2 * 2;
	int xchange= ry2 * (1 - 2 * rx);
	int ychange= rx2;
	int ellipseerror= 0;
	for (int y= 0, x= rx; y < dimy; ++y)
	{
		py [y]= x;
		if (x == 0) // Needed for little rx
			continue;
		ellipseerror+= ychange;
		ychange+= rx2_2;
		if (2 * ellipseerror + xchange > 0)
		{
			--x;
			ellipseerror+= xchange;
			xchange+= ry2_2;
		}
	}
	#else
	for (int y= 0; y < dimy; ++y)
		py [y]= int (rx * sqrt (ry * ry - y * y) / ry + 0.5);
	#endif

	int aux= dimy > 0 ? py [dimy - 1] : rx;
	const int dimx= aux < 0 ? 0 : aux;
	#ifdef DEBUG_ELLIPSE
	cerr << "Dimx: " << dimx << endl;
	#endif
	util::auto_buffer <int> px (dimx);
	#ifndef SIMPLER
	xchange= ry2;
	ychange= rx2 * (1 - 2 * ry);
	ellipseerror= 0;
	for (int x= 0, y= ry; x < dimx; ++x)
	{
		px [x]= y;
		if (y == 0) // Needed for little ry
			continue;
		ellipseerror+= xchange;
		xchange+= ry2_2;
		if (2 * ellipseerror + ychange > 0)
		{
			--y;
			ellipseerror+= ychange;
			ychange+= rx2_2;
		}
	}
	#else
	for (int x= 0; x < dimx; ++x)
		px [x]= int (ry * sqrt (rx * rx - x * x) / rx + 0.5);
	#endif

	for (int y= 1; y < dimy; ++y)
		impl_plot_mask (ox + py [y], oy - y);
	for (int x= dimx - 1; x > 0; --x)
		impl_plot_mask (ox + x, oy - px [x] );
	impl_plot_mask (ox, oy - ry);
	for (int x= 1; x < dimx; ++x)
		impl_plot_mask (ox - x, oy - px [x] );
	for (int y= dimy - 1; y > 0; --y)
		impl_plot_mask (ox - py [y], oy - y);
	impl_plot_mask (ox - rx, oy);
	for (int y= 1; y < dimy; ++y)
		impl_plot_mask (ox - py [y], oy + y);
	for (int x= dimx - 1; x > 0; --x)
		impl_plot_mask (ox - x, oy + px [x] );
	impl_plot_mask (ox, oy + ry);
	for (int x= 1; x < dimx; ++x)
		impl_plot_mask (ox + x, oy + px [x] );
	for (int y= dimy - 1; y > 0; --y)
		impl_plot_mask (ox + py [y], oy + y);
	impl_plot_mask (ox + rx, oy);
}

namespace {

inline void get_point_on_arc (int rx, int ry, BlNumber a,
	int & out_x, int & out_y)
{
	BlNumber s= sin (a) * ry, c= cos (a) * rx;
	out_x= static_cast <int> (c < 0 ? c - .5 : c + .5);
	out_y= static_cast <int> (s < 0 ? s - .5 : s + .5);
}

}

void graphics::arcellipse (int ox, int oy, int rx, int ry,
	BlNumber arcbeg, BlNumber arcend)
{
	/*
		Adapted from the arccircle algorithm,
		not optimal but works.
	*/
	//#define DEBUG_ARCELLIPSE

	requiregraphics ();
	activecolor (pforeground);
	lastx= ox+ rx;
	lasty= oy;

	transform_y (oy);

	int px, py; // Current position and auxiliary
	get_point_on_arc (rx, ry, arcend, px, py);
	const int ex= px, ey= py; // End position
	get_point_on_arc (rx, ry, arcbeg, px, py);
	const int sx= px, sy= py; // Start position
	
	const int sq= get_quadrant (sx, sy); // Start quadrant.
	// Calculate end quadrant, considering that end point
	// must be after start point.
	int q= get_quadrant (ex, ey);
	if (sq > q)
		// Quadrant end must be greater or equal.
		q+= 4;
	else if (sq == q && arcbeg > arcend)
		// If equal, consider the angle.
		q+= 4;
	const int qe= q;
	q= sq; // Current cuadrant.
	#ifdef DEBUG_ARCELLIPSE
	cerr << "Arc llipse from: " << arcbeg << " to " << arcend << endl;
	cerr << "Quadrant from " << sq << " to " << qe << endl;
	#endif

	// Direction of movement.
	int dy = ( ( (q + 1) & 2) == 0) ? 1 : -1;
	int dx= ( (q & 2) == 0) ? -1 : 1;

	const int rx2= rx * rx;
	const int ry2= ry * ry;
	const int rxy2= rx2 * ry2;

	while (true)
	{
		// Change quadrant when needed, adjusting directions.
		if ( (q & 1) == 0)
		{
			// Take care that in very eccentric ellipses
			// can be in 0 before the extreme point.
			if (px == 0 && abs (py) == ry)
			{
				if (qe == q)
					break;
				++q;
				dy= -dy;
			}
		}
		else
		{
			if (py == 0 && abs (px) == rx)
			{
				if (qe == q)
					break;
				++q;
				dx= -dx;
			}
		}
		// If we are in the end quadrant, check if at the end position.
		if (qe == q)
		{
			int det= 0;
			if (dy > 0)
			{
				if (py >= ey)
					++det;
			}
			else
			{
				if (py <= ey)
					++det;
			}
			if (dx > 0)
			{
				if (px >= ex)
					++det;
			}
			else
			{
				if (px <= ex)
					++det;
			}
			if (det == 2)
				break;
		}

		impl_plot_mask (ox + px, oy - py);

		int rr1= ry2 * (px + dx) * (px + dx) +
			rx2 * py * py - rxy2;
		int rr2= ry2 * (px + dx) * (px + dx) +
			rx2 * (py + dy) * (py + dy) - rxy2;
		int rr3= ry2 * px * px +
			rx2 * (py + dy) * (py + dy) - rxy2;
		if (rr1 < 0) rr1= -rr1;
		if (rr2 < 0) rr2= -rr2;
		if (rr3 < 0) rr3= -rr3;
		if (rr3 >= std::min (rr1, rr2) )
		{
			px+= dx;
			//xx= xx_new;
		}
		if (rr1 > std::min (rr2, rr3) )
		{
			py+= dy;
			//yy= yy_new;
		}
	}
	if (px != sx || py != sy || sq == qe)
		impl_plot_mask (ox + px, oy - py);
}

void graphics::mask (int m)
{
	requiregraphics ();
	maskvalue= static_cast <unsigned char> (m);
	maskpos= 0;
}

void graphics::maskdrawfirstpoint (bool f)
{
	requiregraphics ();
	maskdrawfirst= f;
}

namespace {

char skipblank (const char * & s)
{
	char c;
	while ( (c= * s) == ' ' || c == '\t')
		++s;
	return c;
}

int getnum (const char * & s)
{
	int r= 0;
	char c;
	if (! isdigit (* s) )
		throw ErrSyntax;
	while ( isdigit (c= * s) )
	{
		r*= 10;
		r+= c - '0';
		++s;
	}
	//cerr << r;
	return r;
}

enum TypeMove { MoveAbs, MovePos, MoveNeg };

TypeMove gettypemove (const char * & s)
{
	TypeMove tm= MoveAbs;
	char c= skipblank (s);
	switch (c)
	{
	case '+':
		//cerr << '+';
		tm= MovePos;
		++s;
		skipblank (s);
		break;
	case '-':
		//cerr << '-';
		tm= MoveNeg;
		++s;
		skipblank (s);
		break;
	}
	return tm;
}

inline void adjust (int & value, TypeMove t, int last)
{
	switch (t)
	{
	case MoveAbs:
		break;
	case MovePos:
		value= last + value;
		break;
	case MoveNeg:
		value= last - value;
		break;
	}
}

} // namespace

void graphics::draw (const std::string & str)
{
	requiregraphics ();
	const char * s= str.c_str ();
	char c;
	int i;
	while ( (c= skipblank (s) ) != '\0')
	{
		//cerr << c;
		++s;
		bool nopaint= false;
		if (c == 'B')
		{
			c= skipblank (s);
			//cerr << c;
			++s;
			nopaint= true;
		}
		switch (c)
		{
		case 'M':
			{
				TypeMove mx= gettypemove (s);
				int x= getnum (s);
				adjust (x, mx, lastx);
				c= skipblank (s);
				if (c != ',')
					throw ErrSyntax;
				//cerr << ',';
				++s;
				TypeMove my= gettypemove (s);
				int y= getnum (s);
				adjust (y, my, lasty);
				if (nopaint)
					graphics::move (x, y);
				else
					graphics::line (x, y);
			}
			break;
		case 'U':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx, lasty - i);
			else
				graphics::line (lastx, lasty - i);
			break;
		case 'D':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx, lasty + i);
			else
				graphics::line (lastx, lasty + i);
			break;
		case 'R':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx + i, lasty);
			else
				graphics::line (lastx + i, lasty);
			break;
		case 'L':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx - i, lasty);
			else
				graphics::line (lastx - i, lasty);
			break;
		case 'E':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx + i, lasty - i);
			else
				graphics::line (lastx + i, lasty - i);
			break;
		case 'F':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx + i, lasty + i);
			else
				graphics::line (lastx + i, lasty + i);
			break;
		case 'G':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx - i, lasty + i);
			else
				graphics::line (lastx - i, lasty + i);
			break;
		case 'H':
			skipblank (s);
			i= getnum (s);
			if (nopaint)
				graphics::move (lastx - i, lasty - i);
			else
				graphics::line (lastx - i, lasty - i);
			break;
		case 'C':
			c= skipblank (s);
			if (! isdigit (c) )
				throw ErrSyntax;
			graphics::setcolor (c - '0');
			++s;
			break;
		case 'X':
			{
				std::string x;
				while ( (c= *s) != ';' && c != '\0')
				{
					x+= c;
					++s;
				}
				if (c != ';')
					throw ErrSyntax;
				++s;
				if (typeofvar (x) != VarString)
					throw ErrMismatch;
				std::string xx= evaluatevarstring (x);
				draw (xx);
			}
			break;
		case ';':
			break;
		default:
			throw ErrSyntax;
		}
	}
	//cerr << endl;
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

namespace {

int zone= 8;

class BlWindow {
public:
	BlWindow ()
	{
		setdefault ();
		defaultcolors ();
	}
	BlWindow (int x1, int x2, int y1, int y2)
	{
		set (x1, x2, y1, y2);
		defaultcolors ();
	}
	void setdefault ()
	{
		set (0, maxtcol - 1, 0, maxtrow - 1);
	}
	void set (int x1, int x2, int y1, int y2)
	{
		if (x1 > x2) std::swap (x1, x2);
		if (y1 > y2) std::swap (y1, y2);
		orgx= x1; orgy= y1;
		width= x2 - x1 + 1;
		height= y2 - y1 + 1;
		x= y= 0;
	}
	void defaultcolors ()
	{
		foreground= default_foreground;
		background= default_background;
	}
	int getwidth () const { return width; }
	int getxpos () const { return x; }
	void gotoxy (int x, int y)
	{
		this->x= x; this->y= y;
	}
	void tab (size_t n)
	{
		int col (n - 1);
		if (x >= col)
		{
			do {
				charout (' ');
			} while (x > 0);
		}
		if (col >= width)
			throw ErrFunctionCall;
		do {
			charout (' ');
		} while (x < col);
	}
	void setcolor (int color)
	{
		foreground= mapcolor (color);
	}
	void setbackground (int color)
	{
		background= mapcolor (color);
	}
	void movecharforward (size_t n)
	{
		for (size_t i= 0; i < n; ++i)
		{
			if (++x >= width)
			{
				x= 0;
				if (++y >= height)
				{
					textscroll ();
					y= height - 1;
				}
			}
		}
	}
	void movecharback (size_t n)
	{
		for (size_t i= 0; i < n; ++i)
		{
			if (x > 0)
				--x;
			else
			{
				if (y > 0)
				{
					x= width - 1;
					--y;
				}
			}
		}
	}
	void movecharup (size_t n)
	{
		y-= n;
	}
	void movechardown (size_t n)
	{
		y+= n;
	}
	void cls ()
	{
		x= y= 0;

		int x1= orgx * 8, x2= width * 8;
		int y1= orgy * 8, y2= height * 8;

		#ifdef BLASSIC_USE_WINDOWS
		RECT r= { x1, y1, x1 + x2, y1 + y2 };
	        LOGPEN logpen;
        	GetObject (* background, sizeof (LOGPEN), & logpen);
		HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
		if (! fSynchro)
			FillRect (hdc, & r, hbrush);
		FillRect (hdcPixmap, & r, hbrush);
		DeleteObject (hbrush);
		#endif

		#ifdef BLASSIC_USE_X
		activecolor (background);
		XSetFunction (display, gcp, drawmode_copy);
		XFillRectangle (display, pixmap, gcp,
			x1, y1, x2, y2);
		XSetFunction (display, gcp, drawmode);
		if (! fSynchro)
		{
			XSetFunction (display, gc, drawmode_copy);
			XFillRectangle (display, window, gc,
				x1, y1, x2, y2);
			XSetFunction (display, gc, drawmode);
			// Inserted an idle call because without it
			// the window sometimes is not updated.
			graphics::idle ();
		}
		activecolor (pforeground);
		#endif
	}
	void textscroll ()
	{
		int x1= orgx * 8, y1= orgy * 8;
		int w= width * 8; int h= height * 8;
		#ifdef BLASSIC_USE_X
		//int h= screenheight - 8;
		XSetFunction (display, gcp, drawmode_copy);
		XCopyArea (display, pixmap, pixmap, gcp,
			x1, y1 + 8, w, h - 8, x1, y1);
		activecolor (background);
		XFillRectangle (display, pixmap, gcp,
			x1, y1 + h - 8, w, 8);
		XSetFunction (display, gcp, drawmode);
		//XSetForeground (display, gcp, black);

		if (! fSynchro)
		{
			//XSetForeground (display, gc, white);
			XSetFunction (display, gc, drawmode_copy);
			XCopyArea (display, window, window, gc,
				x1, y1 + 8, w, h, x1, y1);
			XFillRectangle (display, window, gc,
				x1, y1 + h - 8, w, 8);
			//XSetForeground (display, gc, black);
			XSetFunction (display, gc, drawmode);
		}
		activecolor (foreground);
		#endif

		#ifdef BLASSIC_USE_WINDOWS
		RECT r = { x1, y1 + h - 8, x1 + w, y1 + h};

		BitBlt (hdcPixmap, x1, y1, w, h - 8,
			hdcPixmap, x1, y1 + 8, SRCCOPY);
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
	void charout (char c)
	{
		pcolor foresave= pforeground;
		pforeground= foreground;
		pcolor backsave= pbackground;
		pbackground= background;
		switch (c)
		{
		case '\n':
                	x= 0;
        	        if (++y >= height)
			{
				textscroll ();
				y= height - 1;
			}
			break;
		case '\r':
			x= 0;
			break;
		case '\t':
			if (x >= (width / zone) * zone)
			{
				//cerr << "Fin de linea" << endl;
				for ( ; x < width; ++x)
					print (orgx + x, orgy + y, ' ');
				x= 0;
				if (++y >= height)
				{
					textscroll ();
					y= height - 1;
				}
			}
			else
			{
				do {
					print (orgx + x, orgy + y, ' ');
					++x;
				} while (x % zone);
			}
			break;
		default:
			print (orgx + x, orgy + y, c);
			if (++x >= width)
			{
				x= 0;
				if (++y >= height)
				{
					textscroll ();
					y= height - 1;
				}
			}
		}
		pforeground= foresave;
		pbackground= backsave;
	}
	void invertcursor ()
	{
		int x1= (orgx + x) * 8;
		int y1= (orgy + y) * 8;

		#ifdef BLASSIC_USE_X
		XSetFunction (display, gc, drawmode_invert);
		XSetFunction (display, gcp, drawmode_invert);
		XFillRectangle (display, window, gc, x1, y1 + 6, 8, 2);
		XFillRectangle (display, pixmap, gcp, x1, y1 + 6, 8, 2);
		XSetFunction (display, gc, drawmode);
		XSetFunction (display, gcp, drawmode);
		#endif

		#ifdef BLASSIC_USE_WINDOWS
		HBRUSH hbrush= (HBRUSH) GetStockObject (BLACK_BRUSH);
		HDC ahdc [2]= { hdc, hdcPixmap };
		for (size_t i= 0; i < 2; ++i)
		{
			HDC hdc= ahdc [i];
			SetROP2 (hdc, drawmode_invert);
			HBRUSH hold= (HBRUSH) SelectObject (hdc, hbrush);
			Rectangle (hdc, x1, y1 + 6, x1 + 8, y1 + 8);
			SelectObject (hdc, hold);
			SetROP2 (hdc, drawmode);
		}
		#endif
	}
private:
	int orgx, orgy, width, height;
	pcolor foreground;
	pcolor background;
	int x, y;
};

BlWindow windowzero;

typedef std::map <BlChannel, BlWindow *> MapWindow;
MapWindow mapwindow;

void killwindowifnotzero (const MapWindow::value_type & mw)
{
	if (mw.first != BlChannel (0) )
		delete mw.second;
}

void recreate_windows ()
{
	windowzero.setdefault ();
	windowzero.defaultcolors ();
	windowzero.cls ();
	std::for_each (mapwindow.begin (), mapwindow.end (),
                killwindowifnotzero);
	mapwindow.clear ();
	mapwindow [0]= & windowzero;
}

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

void graphics::setcolor (BlChannel ch, int color)
{
	mapwindow [ch]->setcolor (color);
}

void graphics::setbackground (BlChannel ch, int color)
{
	mapwindow [ch]->setbackground (color);
}

void graphics::cls (BlChannel n)
{
	mapwindow [n]->cls ();
}

void graphics::definewindow (BlChannel n, int x1, int x2, int y1, int y2)
{
	requiregraphics ();
	--x1; --x2; --y1; --y2;
	MapWindow::iterator it= mapwindow.find (n);
	if (it != mapwindow.end () )
		it->second->set (x1, x2, y1, y2);
	else
		mapwindow [n]= new BlWindow (x1, x2, y1, y2);
}

void graphics::undefinewindow (BlChannel n)
{
	if (n == 0)
		return;
	MapWindow::iterator it= mapwindow.find (n);
	if (it != mapwindow.end () )
	{
		delete it->second;
		mapwindow.erase (it);
	}
}

size_t graphics::getlinewidth ()
{
	#if 0
	return maxtcol;
	#else
	return windowzero.getwidth ();
	#endif
}

size_t graphics::getlinewidth (BlChannel ch)
{
	return mapwindow [ch]->getwidth ();
}

void graphics::charout (char c)
{
	//do_charout (c);
	windowzero.charout (c);
	//idle ();
}

void graphics::stringout (const std::string & str)
{
	//bool fSynchroNormal= fSynchro;
	//fSynchro= true;

	//std::for_each (str.begin (), str.end (), do_charout);
	for (std::string::size_type i= 0, l= str.size (); i < l; ++i)
		windowzero.charout (str [i]);

	//fSynchro= fSynchroNormal;
	//if (! fSynchro)
	//	reinit_window ();

	//idle ();
}

void graphics::charout (BlChannel ch, char c)
{
	mapwindow [ch]->charout (c);
}

void graphics::stringout (BlChannel ch, const std::string & str)
{
	BlWindow * pwin= mapwindow [ch];
	for (std::string::size_type i= 0, l= str.size (); i < l; ++i)
		pwin->charout (str [i]);
}

#if 0
void graphics::locate (int row, int col)
{
        trow= row - 1;
        tcol= col - 1;
}
#endif

void graphics::gotoxy (int x, int y)
{
	trow= y;
	tcol= x;
}

void graphics::gotoxy (BlChannel ch, int x, int y)
{
	mapwindow [ch]->gotoxy (x, y);
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

void graphics::tab (BlChannel ch, size_t n)
{
	mapwindow [ch]->tab (n);
}

void graphics::movecharforward (size_t n)
{
	#if 0
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
	#else
	windowzero.movecharforward (n);
	#endif
}

void graphics::movecharforward (BlChannel ch, size_t n)
{
	mapwindow [ch]->movecharforward (n);
}

void graphics::movecharback (size_t n)
{
	#if 0
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
	#else
	windowzero.movecharback (n);
	#endif
}

void graphics::movecharback (BlChannel ch, size_t n)
{
	mapwindow [ch]->movecharback (n);
}

void graphics::movecharup (size_t n)
{
	#if 0
	trow-= n;
	#else
	windowzero.movecharup (n);
	#endif
}

void graphics::movecharup (BlChannel ch, size_t n)
{
	mapwindow [ch]->movecharup (n);
}

void graphics::movechardown (size_t n)
{
	#if 0
	trow+= n;
	#else
	windowzero.movechardown (n);
	#endif
}

void graphics::movechardown (BlChannel ch, size_t n)
{
	mapwindow [ch]->movechardown (n);
}

namespace {

int symbol_after_is;

}

void graphics::symbolafter (int symbol)
{
	if (symbol < 0 || symbol > 256)
		throw ErrFunctionCall;
	memcpy (charset::data, charset::default_data,
		sizeof (charset::data) );
	symbol_after_is= symbol;
}

void graphics::definesymbol (int symbol, const unsigned char (& byte) [8] )
{
	if (symbol < 0 || symbol > 255)
		throw ErrFunctionCall;
	if (symbol < symbol_after_is)
		throw ErrImproperArgument;
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

namespace {

bool fSynchroSaved= false;

}

void graphics::synchronize_suspend ()
{
        fSynchroSaved= fSynchro;
        if (fSynchro)
                reinit_window ();
        fSynchro= false;
}

void graphics::synchronize_restart ()
{
        fSynchro= fSynchroSaved;
}

int graphics::xmouse () { return xmousepos; }
int graphics::ymouse () { return ymousepos; }

int graphics::xpos () { return lastx; }
int graphics::xpos (BlChannel ch)
{
	return mapwindow [ch]->getxpos ();
}
int graphics::ypos () { return lasty; }

namespace {

#if 0
void invertcursor ()
{
	int x1= tcol * 8;
	int y1= trow * 8;

	#ifdef BLASSIC_USE_X
	XSetFunction (display, gc, drawmode_invert);
	XSetFunction (display, gcp, drawmode_invert);
	XFillRectangle (display, window, gc, x1, y1 + 6, 8, 2);
	XFillRectangle (display, pixmap, gcp, x1, y1 + 6, 8, 2);
	XSetFunction (display, gc, drawmode);
	XSetFunction (display, gcp, drawmode);
	#endif

	#ifdef BLASSIC_USE_WINDOWS
	HBRUSH hbrush= (HBRUSH) GetStockObject (BLACK_BRUSH);
        HDC ahdc [2]= { hdc, hdcPixmap };
        for (size_t i= 0; i < 2; ++i)
        {
                HDC hdc= ahdc [i];
	        SetROP2 (hdc, drawmode_invert);
                HBRUSH hold= (HBRUSH) SelectObject (hdc, hbrush);
                Rectangle (hdc, x1, y1 + 6, x1 + 8, y1 + 8);
                SelectObject (hdc, hold);
	        SetROP2 (hdc, drawmode);
        }
	#endif
}
#endif

}

void graphics::showcursor ()
{
	//invertcursor ();
	windowzero.invertcursor ();
}

void graphics::hidecursor ()
{
	//invertcursor ();
	windowzero.invertcursor ();
}

void graphics::showcursor (BlChannel ch)
{
	mapwindow [ch]->invertcursor ();
}

void graphics::hidecursor (BlChannel ch)
{
	mapwindow [ch]->invertcursor ();
}

// Fin de graphics.cpp
