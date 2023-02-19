// graphics.cpp
// Revision 18-aug-2003

#ifdef __BORLANDC__
#pragma warn -8027
#endif

#include "graphics.h"
#include "sysvar.h"
#include "error.h"
#include "var.h"
#include "key.h"
#include "charset.h"
#include "util.h"
#include "trace.h"

#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>
#include <queue>
#include <cmath>

// Para depuracion
#include <iostream>
using std::cerr;
using std::endl;
#if defined __unix__ || defined __linux__
#include <unistd.h>
#endif
#include <cassert>
#define ASSERT assert

#ifdef BLASSIC_USE_SVGALIB

#include <unistd.h>
#include <sys/types.h>
#include <vga.h>
#include <vgagl.h>

#endif

#ifdef BLASSIC_USE_X

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

#ifdef BLASSIC_USE_WINDOWS

#include <process.h>
#include <windows.h>
#undef min
#undef max

#if defined __CYGWIN32__ || defined __CYGWIN__
// This macros are from Anders Norlander, modified to add
// the cast to start_proc.
/* Macro uses args se we can cast proc to LPTHREAD_START_ROUTINE
   in order to avoid warings because of return type */
#define _beginthreadex(security, stack_size, start_proc, arg, flags, pid) \
CreateThread (security, stack_size, (LPTHREAD_START_ROUTINE) start_proc, \
	arg, flags, (LPDWORD) pid)
#define _endthreadex ExitThread
#endif

// Use Polyline to draw a point.
#define USE_POLY

#endif

// Character set
charset::chardata charset::data [256];

namespace {

#ifdef BLASSIC_USE_SVGALIB

char * font= NULL;

#endif

#ifdef BLASSIC_USE_X

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

typedef XColor color_t;

#endif

#ifdef BLASSIC_USE_WINDOWS

ATOM atomClass;
HANDLE hEvent;
HWND window;
HDC hdc= 0;
HBITMAP pixmap;
HDC hdcPixmap= 0;
typedef HPEN color_t;

#endif

#if defined (BLASSIC_USE_WINDOWS) || defined (BLASSIC_USE_X)

color_t xcBlack, xcBlue, xcGreen, xcCyan,
        xcRed, xcMagenta, xcBrown, xcLightGrey,
	xcDarkGrey, xcLightBlue, xcLightGreen, xcLightCyan,
        xcLightRed, xcLightMagenta, xcYellow, xcWhite;

typedef color_t * pcolor;

pcolor pforeground, pbackground,
	//default_foreground= & xcBlack, default_background= & xcWhite,
        activecolor= NULL;

int default_pen, default_paper;

#endif

std::string default_title ("blassic");

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
        void erase ()
        {
        	CriticalLock lock (cs);
        	while (! q.empty () )
        		q.pop ();
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

int screenwidth, screenheight;
int originx= 0, originy= 0;

bool limited= false;
int limit_minx, limit_miny, limit_maxx, limit_maxy;

enum TransformType { TransformIdentity, TransformInvertY };

TransformType activetransform= TransformIdentity;

inline void transform_x (int & x)
{
	x+= originx;
}

inline void adjust_y (int & y)
{
	switch (activetransform)
	{
	case TransformIdentity:
		break; // Nothing to do
	case TransformInvertY:
		y= screenheight - 1 - y;
		break;
	}
}

inline void transform_y (int & y)
{
	y+= originy;
	adjust_y (y);
}

inline void set_origin (int x, int y)
{
	originx= x;
	//adjust_y (y);
	originy= y;
}

inline void requiregraphics ()
{
	if (actualmode == text_mode)
		throw ErrNoGraphics;
}

void clear_limits ()
{
	limited= false;
}

void set_limits (int minx, int maxx, int miny, int maxy)
{
	limited= true;
	if (minx > maxx)
		std::swap (minx, maxx);
	limit_minx= minx;
	limit_maxx= maxx;
	adjust_y (miny);
	adjust_y (maxy);
	if (miny > maxy)
		std::swap (miny, maxy);
	limit_miny= miny;
	limit_maxy= maxy;
	if (limit_minx <= 0 && limit_maxx >= screenwidth - 1 &&
		limit_miny <= 0 && limit_maxy >= screenheight - 1)
	{
		limited= false;
	}
}

inline bool check_limit (int x, int y)
{
	return (! limited) || (x >= limit_minx && x <= limit_maxx &&
		y >= limit_miny && y <= limit_maxy);
}

#ifdef BLASSIC_USE_SVGALIB

bool svgalib= false;

#else

//const bool svgalib= false;

#endif

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

const int BASIC_COLORS= 16;

bool colors_inited= false;

struct ColorRGB {
	int r;
	int g;
	int b;
};

const ColorRGB assignRGB []= {
	{    0,    0,    0 },
	{    0,    0, 0xA8 },
	{    0, 0xA8,    0 },
	{    0, 0xA8, 0xA8 },
	{ 0xA8,    0,    0 },
	{ 0xA8,    0, 0xA8 },
	{ 0xA8, 0x54,    0 },
	{ 0xA8, 0xA8, 0xA8 },

	{ 0x54, 0x54, 0x54 },
	{ 0x54, 0x54, 0xFF },
	{ 0x54, 0xFF, 0x54 },
	{ 0x54, 0xFF, 0xFF },
	{ 0xFF, 0x54, 0x54 },
	{ 0xFF, 0x54, 0xFF },
	{ 0xFF, 0xFF, 0x54 },
	{ 0xFF, 0xFF, 0xFF }
};

struct ColorInUse {
	pcolor pc;
	ColorRGB rgb;
};

typedef std::map <int, ColorInUse> definedcolor_t;

definedcolor_t definedcolor;

ColorInUse tablecolors []=
{
	{ &xcBlack,        { 0, 0, 0} },
	{ &xcBlue,         { 0, 0, 0} },
	{ &xcGreen,        { 0, 0, 0} },
	{ &xcCyan,         { 0, 0, 0} },
	{ &xcRed,          { 0, 0, 0} },
	{ &xcMagenta,      { 0, 0, 0} },
	{ &xcBrown,        { 0, 0, 0} },
	{ &xcLightGrey,    { 0, 0, 0} },

	{ &xcDarkGrey,     { 0, 0, 0} },
	{ &xcLightBlue,    { 0, 0, 0} },
	{ &xcLightGreen,   { 0, 0, 0} },
	{ &xcLightCyan,    { 0, 0, 0} },
	{ &xcLightRed,     { 0, 0, 0} },
	{ &xcLightMagenta, { 0, 0, 0} },
	{ &xcYellow,       { 0, 0, 0} },
	{ &xcWhite,        { 0, 0, 0} }
};

inline ColorInUse & mapcolor (int color)
{
	if (color >= 0 && color < BASIC_COLORS)
		return tablecolors [color];
	definedcolor_t::iterator it= definedcolor.find (color);
	if (it != definedcolor.end () )
		return it->second;
	return tablecolors [0];
}

inline ColorInUse & mapnewcolor (int color)
{
	if (color >= 0 && color < BASIC_COLORS)
		return tablecolors [color];
	definedcolor_t::iterator it= definedcolor.find (color);
	if (it != definedcolor.end () )
		return it->second;
	ColorInUse n= { new color_t, { 0, 0, 0} };
	return definedcolor.insert (std::make_pair (color, n) ).first->second;
}

void setink (int inknum, const ColorRGB & rgb)
{
	ColorInUse & ciu= mapnewcolor (inknum);

	#ifdef BLASSIC_USE_WINDOWS

	ASSERT (hdcPixmap);
        COLORREF newcolor=
        	GetNearestColor (hdcPixmap, RGB (rgb.r, rgb.g, rgb.b) );
        ciu.rgb.r= GetRValue (newcolor);
        ciu.rgb.g= GetGValue (newcolor);
        ciu.rgb.b= GetBValue (newcolor);
	HPEN newpen= CreatePen (PS_SOLID, 1, newcolor);
	if (ciu.pc == pforeground)
	{
		//SelectObject (hdc, * ciu.pc);
		//SelectObject (hdcPixmap, * ciu.pc);
		SelectObject (hdc, newpen);
		SelectObject (hdcPixmap, newpen);
	}
	if (* ciu.pc != NULL)
		DeleteObject (* ciu.pc);
	* ciu.pc= newpen;

	#elif defined BLASSIC_USE_X

	ciu.rgb= rgb;
	ASSERT (display);
	Colormap cm= DefaultColormap (display, screen);
	XColor xc;
	std::ostringstream namecolor;
	namecolor << "rgb:" << std::hex << std::setfill ('0') <<
		std::setw (2) << rgb.r << '/' <<
		std::setw (2) << rgb.g << '/' <<
		std::setw (2) << rgb.b;
	XColor newpen;
	XAllocNamedColor (display, cm,
		namecolor.str ().c_str (), & newpen, & xc);
	if (ciu.pc == pforeground)
	{
		//XSetForeground (display, gcp, ciu.pc->pixel);
		//XSetForeground (display, gc, ciu.pc->pixel);
		XSetForeground (display, gcp, newpen.pixel);
		XSetForeground (display, gc, newpen.pixel);
	}
	// Not sure if previous color needs to be freed and why.
	* ciu.pc= newpen;

	#endif
}

void setcpcink (int inknum, int cpccolor)
{
	// These rgb values are taken from screen captures
	// of the WinAPE2 Amstrad CPC emulator.
	static const ColorRGB cpctable []= {
		{   0,   0,   0 }, // Black
		{   0,   0,  96 }, // Blue
		{   0,   0, 255 }, // Bright blue
		{  96,   0,   0 }, // Red
		{  96,   0,  96 }, // Magenta
		{  96,   0, 255 }, // Mauve
		{ 255,   0,   0 }, // Bright red
		{ 255,   0,  96 }, // Purple
		{ 255,   0, 255 }, // Bright magenta
		{   0, 103,   0 }, // Green
		{   0, 103,  96 }, // Cyan
		{   0, 103, 255 }, // Sky blue
		{  96, 103,   0 }, // Yellow
		{  96, 103,  96 }, // White
		{  96, 103, 255 }, // Pastel blue
		{ 255, 103,   0 }, // Orange
		{ 255, 103,  96 }, // Pink
		{ 255, 103, 255 }, // Pastel magenta
		{   0, 255,   0 }, // Bright green
		{   0, 255,  96 }, // Sea green
		{   0, 255, 255 }, // Bright cyan
		{  96, 255,   0 }, // Lime green
		{  96, 255,  96 }, // Pastel green
		{  96, 255, 255 }, // Pastel cyan
		{ 255, 255,   0 }, // Bright yellow
		{ 255, 255,  96 }, // Pastel yellow
		{ 255, 255, 255 }, // Brigth white
	};
	ASSERT (cpccolor >= 0 &&
		cpccolor < static_cast <int> ( util::dim_array (cpctable) ) );
	const ColorRGB & rgb= cpctable [cpccolor];

	setink (inknum, rgb);
}

void cpc_default_inks ()
{
	static const int default_ink []=
		{ 1, 24, 20, 6, 26, 0, 2, 8, 10, 12, 14, 16, 18, 22, 1, 16 };
	// The last two are blinking on the CPC, we use the first color.
	for (int i= 0; i < int (util::dim_array (default_ink) ); ++i)
	//for (int i= 0; i < 16; ++i)
		setcpcink (i, default_ink [i] );
}

void spectrum_inks ()
{
	// Taken from a screen capture of the Spectaculator Spectrum Emulator.
	static const ColorRGB spectrumtable []=
	{
		{   0,   0,   0 }, // Black
		{   0,   0, 207 }, // Blue
		{ 207,   0,   0 }, // Red
		{ 207,   0, 207 }, // Magenta
		{   0, 200,   0 }, // Green
		{   0, 200, 207 }, // Cyan
		{ 207, 200,   0 }, // Yellow
		{ 207, 200, 207 }, // White
		{   0,   0,   0 }, // Black bright
		{   0,   0, 255 }, // Blue bright
		{ 255,   0,   0 }, // Red bright
		{ 255,   0, 255 }, // Magenta bright
		{   0, 248,   0 }, // Green bright
		{   0, 248, 255 }, // Cyan bright
		{ 255, 248,   0 }, // Yellow bright
		{ 255, 248, 255 }, // White bright
	};
	for (int i= 0; i < 16; ++i)
		setink (i, spectrumtable [i] );
}

enum Inkset { InkStandard, InkCpc, InkSpectrum } inkset= InkStandard;

void init_colors ()
{
	TraceFunc tr ("init_colors");

	COMPILE_ASSERT (sizeof (assignRGB) / sizeof (ColorRGB) ==
		BASIC_COLORS);
	COMPILE_ASSERT (sizeof (tablecolors) / sizeof (ColorInUse) ==
		BASIC_COLORS);
	switch (inkset)
	{
	case InkStandard:
		for (int i= 0; i < BASIC_COLORS; ++i)
		{
			const ColorRGB & rgb= assignRGB [i];
			setink (i, rgb);
		}
		break;
	case InkCpc:
		cpc_default_inks ();
		break;
	case InkSpectrum:
		spectrum_inks ();
		break;
	}
	colors_inited= true;
	pforeground= mapcolor (default_pen).pc;
	pbackground= mapcolor (default_paper).pc;
}

void reinit_pixmap ()
{
	#ifdef BLASSIC_USE_WINDOWS

	RECT r = { 0, 0, screenwidth, screenheight };
	FillRect (hdcPixmap, & r, (HBRUSH) GetStockObject (WHITE_BRUSH) );

	#elif defined BLASSIC_USE_X

	XSetForeground (display, gcp, WhitePixel (display, screen) );
	XFillRectangle (display, pixmap, gcp, 0, 0, screenwidth, screenheight);
	XSetForeground (display, gcp, BlackPixel (display, screen) );

        #endif
}

void reinit_window ()
{
	#ifdef BLASSIC_USE_WINDOWS
        
	BitBlt (hdc, 0, 0, screenwidth, screenheight, hdcPixmap,
		0, 0, SRCCOPY);

	#elif defined BLASSIC_USE_X

	XSetFunction (display, gc, drawmode_copy);
	XCopyArea (display, pixmap, window, gc,
		0, 0, screenwidth, screenheight, 0, 0);
	//XSetForeground (display, gc, BlackPixel (display, screen) );
	//XSetForeground (display, gc, pforeground->pixel);
	XFlush (display);
	XSetFunction (display, gc, drawmode);

        #endif
}

void reinit_window (int x, int y, int width, int height)
{
	#ifdef BLASSIC_USE_WINDOWS
        
	BitBlt (hdc, x, y, width, height, hdcPixmap,
		x, y, SRCCOPY);

	#elif defined BLASSIC_USE_X

	XSetFunction (display, gc, drawmode_copy);
	XCopyArea (display, pixmap, window, gc,
		x, y, width, height, x, y);
	//XSetForeground (display, gc, BlackPixel (display, screen) );
	//XSetForeground (display, gc, pforeground->pixel);
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

class ProtectWindow {
	HWND hwnd;
public:
	ProtectWindow (HWND hwnd) : hwnd (hwnd) { }
	~ProtectWindow ()
	{
		if (hwnd)
			DestroyWindow (hwnd);
	}
	void release ()
	{
		hwnd= 0;
	}
};

class ProtectPixmap {
	HBITMAP & pixmap;
	bool active;
public:
	ProtectPixmap (HBITMAP & pixmap) : pixmap (pixmap), active (true) { }
	~ProtectPixmap ()
	{
		if (active)
		{
			DeleteObject (pixmap);
			pixmap= NULL;
		}
	}
	void release ()
	{
		active= false;
	}
};

void thread_create_window (int width, int height)
{
        window= CreateWindow (
                LPCTSTR (atomClass),
                default_title.c_str (),
                /*WS_VISIBLE | */ WS_SYSMENU | WS_MINIMIZEBOX,
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
		ProtectWindow pw (window);
                hdc= GetDC (window);
		if (hdc == NULL)
			return;
		pixmap= CreateCompatibleBitmap (hdc, width, height);
		if (pixmap == NULL)
			return;
		ProtectPixmap pp (pixmap);
		hdcPixmap= CreateCompatibleDC (hdc);
		if (hdcPixmap == NULL)
			return;
		SelectObject (hdcPixmap, pixmap);
                init_colors ();
		reinit_pixmap ();
                window_created= true;
		ShowWindow (window, SW_SHOWNORMAL);
                //SetActiveWindow (window);
		SetForegroundWindow (window);
		pw.release ();
		pp.release ();
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

	#elif defined BLASSIC_USE_X

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

	#if 0
	int depth= DefaultDepth (display, DefaultScreen (display) );
	#else
	unsigned int depth;
	{
		XWindowAttributes attr;
		XGetWindowAttributes (display, window, & attr);
		depth= attr.depth;
	}
	#endif
	pixmap= XCreatePixmap (display, window,
		width, height, depth);
	pixmap_created= true;

	gc= XCreateGC (display, window, 0, & gcvalues);
	gcp= XCreateGC (display, pixmap, 0, & gcvaluesp);
	init_colors ();
	reinit_pixmap ();
	XSetStandardProperties (display, window,
			default_title.c_str (),
			default_title.c_str (),
			None,
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

	#elif defined BLASSIC_USE_X

	XDestroyWindow (display, window);
	window_created= false;
	XFreePixmap (display, pixmap);
	pixmap_created= false;
	XFlush (display);
	graphics::idle ();

        #endif
}

} // namespace

void graphics::initialize (const char * progname)
{
	TraceFunc tr ("graphics::initialize");

	// Default symbol after and charset initialization:
	symbolafter (0);

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
			//init_xcolors ();
			//init_colors ();
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

	#elif defined  BLASSIC_USE_WINDOWS

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
                //init_wincolors ();
		//init_colors ();
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
		display= 0;
	}

	#elif defined BLASSIC_USE_WINDOWS

        //destroy_thread ();
        //if (window_created)
        //        destroy_window ();

        if (atomClass)
                UnregisterClass (LPCTSTR (atomClass),
                        GetModuleHandle (0) );
        #endif
}

void graphics::origin (int x, int y)
{
	set_origin (x, y);
}

void graphics::limits (int minx, int maxx, int miny, int maxy)
{
	TraceFunc tr ("graphics::limits");

	set_limits (minx, maxx, miny, maxy);
}

void graphics::ink (int inknum, int cpccolor)
{
	requiregraphics ();
	if (cpccolor < 0 || cpccolor > 26)
		throw ErrFunctionCall;
	setcpcink (inknum, cpccolor);
}

void graphics::ink (int inknum, int r, int g, int b)
{
	requiregraphics ();
	ColorRGB rgb= { r, g, b };
	setink (inknum, rgb);
}

void graphics::clearink ()
{
	requiregraphics ();
	init_colors ();
}

namespace {

#ifdef BLASSIC_USE_X

void keypressed (XKeyEvent & xk)
{
	char buffer [10];
	KeySym ks;
	//XComposeStatus xcs;
	int r= XLookupString (& xk, buffer, sizeof (buffer - 1),
		& ks, /*& xcs*/ NULL);
	#if 0
	if (r > 0)
	{
		buffer [r]= '\0';
		queuekey.push (std::string (buffer) );
	}
	else
		queuekey.push (string_from_key (ks) );
	#else
	// We lookup the code first, thus Delete is not translated.
	std::string str= string_from_key (ks);
	if (! str.empty () )
		queuekey.push (str);
	else
		if (r > 0)
			queuekey.push (std::string (buffer, r) );
	#endif
}

#endif

} // namespace

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
			keypressed (x_event.xkey);
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
        //UpdateWindow (window);
	//Sleep (0);
        #endif

}

#include "charset.h"

namespace {

void setactivecolor (pcolor pxc)
{
        activecolor= pxc;

	#ifdef BLASSIC_USE_X

	XSetForeground (display, gcp, pxc->pixel);
	XSetForeground (display, gc, pxc->pixel);

	#elif defined BLASSIC_USE_WINDOWS

	SelectObject (hdc, * pxc);
	SelectObject (hdcPixmap, *pxc);

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

	XSetFunction (display, gcp, drawmode_copy);
	XCopyArea (display, pixmap, pixmap, gcp,
		0, 8, screenwidth, h, 0, 0);
	setactivecolor (pbackground);
	XFillRectangle (display, pixmap, gcp,
		0, h, screenwidth, 8);
	setactivecolor (pforeground);
	XSetFunction (display, gcp, drawmode);

	if (! fSynchro)
		reinit_window ();

	#elif defined BLASSIC_USE_WINDOWS

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
	if (! fSynchro)
		reinit_window ();

        #endif

}

void do_fill_rectangle (int x1, int y1, int x2, int y2, bool limitable)
{
	using std::min;
	using std::max;
	if (limitable && limited)
	{
		x1= max (x1, limit_minx);
		y1= max (y1, limit_miny);
		x2= min (x2, limit_maxx);
		y2= min (y2, limit_maxy);
		if (x1 > limit_maxx || x2 < limit_minx ||
				y1 > limit_maxy || y2 < limit_miny)
			return;
	}
        #ifdef BLASSIC_USE_WINDOWS
        RECT r = { x1, y1, x2 + 1, y2 + 1 };
	LOGPEN logpen;
	//GetObject (* pforeground, sizeof (LOGPEN), & logpen);
	GetObject (* activecolor, sizeof (LOGPEN), & logpen);
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

#ifdef BLASSIC_USE_WINDOWS

// Define in windows the struct used in X for XDrawPoints.
struct XPoint {
	short x, y;
};

#endif

inline void do_plot (int x, int y)
{
	if (! check_limit (x, y) )
		return;

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

	#elif defined BLASSIC_USE_WINDOWS

	#ifdef USE_POLY
	POINT p [2]= { {x, y}, {x + 1, y} };
	#endif

	if (! fSynchro)
	{
		#ifdef USE_POLY
		Polyline (hdc, p, 2);
		#else
		MoveToEx (hdc, x, y, 0);
		LineTo (hdc, x + 1, y);
		#endif
	}
	#ifdef USE_POLY
	Polyline (hdcPixmap, p, 2);
	#else
	MoveToEx (hdcPixmap, x, y, 0);
	LineTo (hdcPixmap, x + 1, y);
	#endif

        #endif
}

int tcol, trow;
int maxtcol= 40, maxtrow= 25;
const int MAXZOOMTEXTY= 4;
const int MAXZOOMTEXTX= 4;
// Default values are needed by the initialization of windowzero.
int zoomtextx= 1, zoomtexty= 1, charwidth= 8, charheight= 8;

#ifdef BLASSIC_USE_WINDOWS

// Length of each segment: all are 2 points.
DWORD poly_points [64 * MAXZOOMTEXTY];
bool init_poly_points ()
{
	std::fill_n (poly_points, util::dim_array (poly_points), 2);
	return true;
}
bool poly_points_inited= init_poly_points ();

#endif

inline void do_plot_points (XPoint point [], int npoints, bool limitable)
{
	ASSERT (npoints < 64 * zoomtexty);

	#ifdef BLASSIC_USE_X
	if (zoomtextx == 1)
	{
		XDrawPoints (display, pixmap, gcp,
			point, npoints, CoordModeOrigin);
		if (! fSynchro)
			XDrawPoints (display, window, gc,
				point, npoints, CoordModeOrigin);
	}
	else
	{
		// 64 points each char * max zoomtexty
		XSegment seg [64 * MAXZOOMTEXTY];
		for (int i= 0; i < npoints; ++i)
		{
			int xpos= point [i].x;
			int ypos= point [i].y;
			seg [i].x1= xpos;
			seg [i].y1= ypos;
			xpos+= zoomtextx - 1;
			if (limitable && limited)
				xpos= std::min (xpos, limit_maxx);
			seg [i].x2= xpos;
			seg [i].y2= ypos;
		}
		XDrawSegments (display, pixmap, gcp, seg, npoints);
		if (! fSynchro)
			XDrawSegments (display, window, gc, seg, npoints);
	}
	#endif

	#ifdef BLASSIC_USE_WINDOWS
	// The PolyPolyline is a bit faster than doing each
	// point separately.
	// 64 * 2 points by segment * max zoomtexty
	static POINT p [64 * 2 * MAXZOOMTEXTY];
	// 64 points each char * max zoomtexty
	for (int i= 0; i < npoints; ++i)
	{
		int xpos= point [i].x;
		int ypos= point [i].y;
		p [2 * i].x= xpos;
		p [2 * i].y= ypos;
		xpos+= zoomtextx;
		if (zoomtextx > 1) ++xpos;
		if (limitable && limited)
			xpos= std::min (xpos, limit_maxx);
		p [2 * i + 1].x= xpos;
		p [2 * i + 1].y= ypos;
	}
	if (! fSynchro)
		PolyPolyline (hdc, p, poly_points, npoints);
	PolyPolyline (hdcPixmap, p, poly_points, npoints);
	#endif
}

void printxy (int x, int y, unsigned char ch,
	bool limitable, bool inverse= false, bool underline= false)
{
	static unsigned char mask [8]= { 128, 64, 32, 16, 8, 4, 2, 1 };

	//charset::chardata & data= charset::data [ch];

	charset::chardata data;
	memcpy (data, charset::data [ch], sizeof (charset::chardata) );
	if (underline)
		data [7]= 255;

	XPoint point [64 * MAXZOOMTEXTY]; // 64 pixels * max zoom height
	int n= 0, npoints= 0;
	for (int i= 0, yi= y; i < 8; ++i, yi+= zoomtexty)
	{
                unsigned char c= data [i];
                if (inverse)
                	c= static_cast <unsigned char> (~ c);

		// Little optimization:
		if (c == 0)
			continue;

		for (int j= 0, xj= x; j < 8; ++j, xj+= zoomtextx)
		{
			if (c & mask [j] )
			{
				for (int z= 0; z < zoomtexty; ++z)
				{
					int y= yi + z;
					if (! limitable ||
						check_limit (xj, y) )
					{
						point [n].x= static_cast
							<short> (xj);
						point [n].y= static_cast
							<short> (y);
						++n;
					}
				}
				++npoints;
			}
		}
	}
	if (npoints < 64)
	{
		if (opaquemode)
		{
			setactivecolor (pbackground);
			do_fill_rectangle (x, y,
				x + charwidth - 1, y + charheight - 1,
				limitable);
		}
		if (n > 0)
		{
			setactivecolor (pforeground);
			do_plot_points (point, n, limitable);
		}
		else
			setactivecolor (pforeground);
	}
	else
	{
		setactivecolor (pforeground);
		do_fill_rectangle (x, y,
			x + charwidth - 1, y + charheight - 1,
			limitable);
	}
}

inline void print (int col, int row, unsigned char ch,
	bool inverse, bool underline= false)
{
	printxy (col * charwidth, row * charheight, ch, false,
		inverse, underline);
}

void setmaxtext ()
{
	charwidth= 8 * zoomtextx;
	charheight= 8 * zoomtexty;
        maxtcol= screenwidth / charwidth;
        maxtrow= screenheight / charheight;
}

void recreate_windows ();

void set_mode (int width, int height, int mode, int charx= 1, int chary= 1)
{
	TraceFunc tr ("set_mode");
	ASSERT (charx >= 1 && charx <= MAXZOOMTEXTX);
	ASSERT (chary >= 1 && chary <= MAXZOOMTEXTY);

	std::ostringstream oss;
	oss << "Width " << short (width) << ", height " << short (height);
	tr.message (oss.str () );

	if (mode != 0 && (width <= 0 || height <= 0) )
		throw ErrImproperArgument;

	sysvar::set16 (sysvar::GraphicsWidth, short (width) );
	sysvar::set16 (sysvar::GraphicsHeight, short (height) );
	screenwidth= width;
	screenheight= height;
	activetransform= TransformIdentity;
	set_origin (0, 0);
	fSynchro= false;
	zoomtextx= charx;
	zoomtexty= chary;
	clear_limits ();

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
                        //graphics::setcolor (0);
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
		//if (! colors_inited)
		//	init_colors ();
                setmaxtext ();
		tcol= trow= 0;
		lastx= lasty= 0;
		#if defined (BLASSIC_USE_WINDOWS) || defined (BLASSIC_USE_X)
		//pforeground= default_foreground;
		//pbackground= default_background;
		#endif
		recreate_windows ();
	}
}

} // namespace

void graphics::cls ()
{
	TraceFunc tr ("graphics::cls");

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

	int x1, y1, width, height;
	if (limited)
	{
		tr.message ("lmited");
		x1= limit_minx; width= limit_maxx - limit_minx + 1;
		y1= limit_miny; height= limit_maxy - limit_miny + 1;
	}
	else
	{
		tr.message ("unlimited");
		x1= 0; width= screenwidth;
		y1= 0; height= screenheight;
	}

	#ifdef BLASSIC_USE_WINDOWS

	//RECT r= { 0, 0, screenwidth, screenheight };
	RECT r= { x1, y1, x1 + width + 1, y1 + height + 1 };
        LOGPEN logpen;
        GetObject (* pbackground, sizeof (LOGPEN), & logpen);
        HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
	if (! fSynchro)
		FillRect (hdc, & r, hbrush);
        FillRect (hdcPixmap, & r, hbrush);
        DeleteObject (hbrush);

	#elif defined BLASSIC_USE_X

	setactivecolor (pbackground);
	XSetFunction (display, gcp, drawmode_copy);
	XFillRectangle (display, pixmap, gcp,
		//0, 0, screenwidth, screenheight);
		x1, y1, width, height);
	XSetFunction (display, gcp, drawmode);
	if (! fSynchro)
	{
		XSetFunction (display, gc, drawmode_copy);
		XFillRectangle (display, window, gc,
			//0, 0, screenwidth, screenheight);
			x1, y1, width, height);
		XSetFunction (display, gc, drawmode);
		// Inserted an idle call because without it
		// the window sometimes is not updated.
		graphics::idle ();
	}
	setactivecolor (pforeground);

	#endif
}

void graphics::setmode (int width, int height, bool inverty,
	int zoomx, int zoomy)
{
	if (! inited)
		throw ErrFunctionCall;
	if (zoomx < 1 || zoomx > MAXZOOMTEXTX)
		throw ErrImproperArgument;
	if (zoomy < 1 || zoomy > MAXZOOMTEXTY)
		throw ErrImproperArgument;

	inkset= InkStandard;
	//default_foreground= & xcBlack;
	//default_background= & xcWhite;
	default_pen= 0;
	default_paper= 15;
	::set_mode (width, height, user_mode, zoomx, zoomy);
	if (inverty)
		activetransform= TransformInvertY;
}

void graphics::setmode (int mode)
{
	TraceFunc tr ("graphics::setmode");

	if (! inited && mode != text_mode)
		throw ErrFunctionCall;

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

	//default_foreground= & xcBlack;
	//default_background= & xcWhite;
	if (mode != 0)
	{
		inkset= InkStandard;
		default_pen= 0;
		default_paper= 15;
	}
	::set_mode (width, height, mode);
}

namespace {

struct SpecialMode {
	std::string name;
	Inkset inkset;
	int pen;
	int paper;
	int width;
	int height;
	TransformType transform;
	int zoomx;
	int zoomy;
        SpecialMode (const std::string & name, Inkset inkset,
                        int pen, int paper, int width, int height,
                        TransformType transform, int zoomx, int zoomy) :
                name (name), inkset (inkset),
                pen (pen), paper (paper), width (width), height (height),
                transform (transform), zoomx (zoomx), zoomy (zoomy)
        { }
};

const SpecialMode specialmodes []= {
	SpecialMode ("spectrum", InkSpectrum, 0,  7, 256, 192,
		TransformInvertY,  1, 1),
	SpecialMode ("cpc0",     InkCpc,      1,  0, 640, 400,
		TransformInvertY,  4, 2),
	SpecialMode ("cpc1",     InkCpc,      1,  0, 640, 400,
		TransformInvertY,  2, 2),
	SpecialMode ("cpc2",     InkCpc,      1,  0, 640, 400,
		TransformInvertY,  1, 2),
	SpecialMode ("pcw",      InkStandard, 0, 15, 720, 248,
		TransformIdentity, 1, 1),
	SpecialMode ("pcw2",     InkStandard, 0, 15, 720, 496,
		TransformIdentity, 1, 2),
};

} // namespace

void graphics::setmode (const std::string & mode)
{
	TraceFunc tr ("graphics::setmode (string)");

	if (! inited)
		throw ErrFunctionCall;
	for (size_t i= 0; i < util::dim_array (specialmodes); ++i)
	{
		const SpecialMode & m= specialmodes [i];
		if (m.name == mode)
		{
			inkset= m.inkset;
			default_pen= m.pen;
			default_paper= m.paper;
			::set_mode (m.width, m.height, user_mode,
				m.zoomx, m.zoomy);
			activetransform= m.transform;
			return;
		}
	}
	throw ErrFunctionCall;
}

bool graphics::ingraphicsmode ()
{
        return actualmode != text_mode;
}

void graphics::setcolor (int color)
{
	requiregraphics ();

        #ifdef BLASSIC_USE_SVGALIB

	if (svgalib) {
		vga_setcolor (color);
		return;
	}

	#endif

	pcolor pxc= mapcolor (color).pc;

	setactivecolor (pxc);

	#if defined (BLASSIC_USE_WINDOWS) || defined (BLASSIC_USE_X)
	pforeground= pxc;
	#endif
}

void graphics::setbackground (int color)
{
	//if (! inited) return;
	pcolor pxc= mapcolor (color).pc;
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

	#elif defined BLASSIC_USE_WINDOWS

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

	transform_x (x); transform_x (prevx);
	transform_y (y); transform_y (prevy);

	setactivecolor (pforeground);

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

	#elif defined BLASSIC_USE_WINDOWS

	if (! fSynchro)
	{
		MoveToEx (hdc, prevx, prevy, 0);
		LineTo (hdc, x, y);
		LineTo (hdc, x + 1, y); // Last point
	}

        MoveToEx (hdcPixmap, prevx, prevy, 0);
        LineTo (hdcPixmap, x, y);
	LineTo (hdcPixmap, x + 1, y); // Last point

        #endif
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

	transform_x (prevx); transform_x (x);
	transform_y (prevy); transform_y (y);

	setactivecolor (pforeground);

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
	// When limited we use line masked to simplify line unmasked.
	if (! limited && (maskvalue == maskvaluedefault && maskdrawfirst) )
		do_line_unmasked (x, y);
	else
		do_line_mask (x, y);
}

#ifdef BLASSIC_USE_X
inline bool load_char_image (int x, int y, unsigned char (& ch) [8] )
{
	class ImageGuard {
	public:
		ImageGuard (XImage * img) :
			img (img)
		{ }
		~ImageGuard ()
		{
			XDestroyImage (img);
		}
	private:
		XImage * img;
	};
	
	//XImage * img= XGetImage (display, pixmap, x, y, 8, 8,
	//	AllPlanes, XYPixmap);
	XImage * img= XGetImage (display, pixmap, x, y,
		charwidth, charheight, AllPlanes, XYPixmap);
	if (img == NULL)
		throw ErrNoGraphics; // Not a good idea, provisional.
	ImageGuard guard (img);
	unsigned long back= XGetPixel (img, 0, 0);
	bool fFore= false;
	unsigned long fore= 0;
	for (int i= 0, ipos= 0; i < 8; ++i, ipos+= zoomtexty)
		for (int j= 0, jpos= 0; j < 8; ++j, jpos+= zoomtextx)
		{
			unsigned long c= XGetPixel (img, jpos, ipos);
			bool bit;
			if (c == back)
			{
				bit= false;
			}
			else
			{
				if (! fFore)
				{
					fFore= true;
					fore= c;
				}
				if (c != fore)
					return false;
				bit= true;
			}
			if (zoomtextx > 1 || zoomtexty > 1)
			{
				const int iend= ipos + zoomtexty;
				const int jend= jpos + zoomtextx;
				for (int ii= ipos; ii < iend; ++ii)
					for (int jj= jpos; jj < jend; ++jj)
						if (XGetPixel (img, jj, ii)
								!= c)
							return false;
			}
			ch [i]<<= 1;
			ch [i]|= bit;
		}
	//XDestroyImage (img);
	return true;
}
#endif

std::string copychrat (int x, int y)
{
	TraceFunc tr ("copychrat");
	{
		std::ostringstream oss;
		oss << "at " << x << ", " << y;
		tr.message (oss.str () );
	}

	unsigned char ch [8]= {0};

	//if (x < 0 || x > screenwidth - 8 || y < 0 || y > screenheight - 8)
	if (x < 0 || x > screenwidth - charwidth ||
		y < 0 || y > screenheight - charheight)
		return std::string ();

	#ifdef BLASSIC_USE_X

	if (! load_char_image (x, y, ch) )
		return std::string ();

	#elif defined BLASSIC_USE_WINDOWS

        COLORREF back= GetPixel (hdcPixmap, x, y);
        bool fFore= false;
        COLORREF fore= 0;
        for (int i= 0, ipos= y; i < 8; ++i, ipos+= zoomtexty)
                for (int j= 0, jpos= x; j < 8; ++j, jpos+= zoomtextx)
                {
                        COLORREF c= GetPixel (hdcPixmap, jpos, ipos);
                        bool bit;
                        if (c == back)
                                bit= false;
                        else
                        {
                                if (! fFore)
                                {
                                        fFore= true;
                                        fore= c;
                                }
                                if (c != fore)
                                        return  std::string ();
                                bit= true;
                        }
			if (zoomtextx > 1 || zoomtexty > 1)
			{
				const int iend= ipos + zoomtexty;
				const int jend= jpos + zoomtextx;
				for (int ii= ipos; ii < iend; ++ii)
					for (int jj= jpos; jj < jend; ++jj)
						if (GetPixel (hdcPixmap,
								jj, ii) != c)
							return std::string ();
			}
                        ch [i]<<= 1;
                        ch [i]|= bit;
                }

        #endif

	#ifndef NDEBUG
	{
		std::ostringstream oss;
		oss << "Char: ";
		for (int i= 0; i < 8; ++i)
		{
			oss << static_cast <unsigned int> (ch [i]) << ", ";
		}
		tr.message (oss.str () );
	}
	#endif

	char chinv [8];
	for (int i= 0; i < 8; ++i)
		chinv [i]= static_cast <unsigned char> (~ ch [i]);
	for (int i= 0; i < 256; ++i)
	{
		if (memcmp (charset::data [i], ch, 8) == 0 ||
			memcmp (charset::data [i], chinv, 8) == 0)
		{
			return std::string (1, static_cast <char> (i) );
		}
	}

	return std::string ();
}

int test ()
{
	int x= lastx;
	int y= lasty;
	transform_x (x);
	transform_y (y);

	if (x < 0 || x >= screenwidth || y < 0 || y >= screenheight)
		return 0;

	#ifdef BLASSIC_USE_X

	XImage * img= XGetImage (display, pixmap, x, y, 1, 1,
		AllPlanes, XYPixmap);
	if (img == NULL)
		throw ErrNoGraphics; // Not a good idea, provisional.
	unsigned long color= XGetPixel (img, 0, 0);
	XDestroyImage (img);
	for (int i= 0; i < 16; ++i)
		if (mapcolor (i).pc->pixel == color)
			return i;
	for (definedcolor_t::iterator it= definedcolor.begin ();
		it != definedcolor.end (); ++it)
	{
		if (it->second.pc->pixel == color)
			return it->first;
	}
	return -1;

	#elif defined BLASSIC_USE_WINDOWS

	COLORREF color= GetPixel (hdcPixmap, x, y);
	for (int i= 0; i < 16; ++i)
	{
		//const ColorRGB & c= assignRGB [i];
		const ColorRGB & c= mapcolor (i).rgb;
		if (RGB (c.r, c.g, c.b) == color)
		//if (GetNearestColor (hdcPixmap, RGB (c.r, c.g, c.b) ) == color)
			return i;
	}
	for (definedcolor_t::iterator it= definedcolor.begin ();
		it != definedcolor.end (); ++it)
	{
		const ColorRGB & c= it->second.rgb;
		if (RGB (c.r, c.g, c.b) == color)
			return it->first;
	}
	return -1;

	#else

	return -1;

	#endif
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
	transform_x (x1); transform_x (x2);
	transform_y (y1); transform_y (y2);
	if (x1 > x2) std::swap (x1, x2);
	if (y1 > y2) std::swap (y1, y2);

        #ifdef BLASSIC_USE_WINDOWS

        Rectangle (hdcPixmap, x1, y1, x2 + 1, y2 + 1);
        if (! fSynchro)
                Rectangle (hdc, x1, y1, x2 + 1, y2 + 1);

	#elif defined BLASSIC_USE_X

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
	transform_x (x1); transform_x (x2);
	transform_y (y1); transform_y (y2);
	if (x1 > x2) std::swap (x1, x2);
	if (y1 > y2) std::swap (y1, y2);
	do_fill_rectangle (x1, y1, x2, y2, true);
}

void graphics::move (int x, int y)
{
	requiregraphics ();
	lastx= x; lasty= y;
}

void graphics::mover (int x, int y)
{
	requiregraphics ();
	lastx+= x; lasty+= y;
}

void graphics::plot (int x, int y)
{
	requiregraphics ();
	lastx= x; lasty= y;
	transform_x (x);
	transform_y (y);
	setactivecolor (pforeground);
	do_plot (x, y);
}

void graphics::plotr (int x, int y)
{
	plot (lastx + x, lasty + y);
}

int graphics::test (int x, int y, bool relative)
{
	// Check for graphics mode is done when calling move.
	if (relative)
	{
		x+= lastx;
		y+= lasty;
	}
	move (x, y);
	return ::test ();
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
	setactivecolor (pforeground);

	lastx= x + radius;
	lasty= y;

	transform_x (x);
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
	setactivecolor (pforeground);

	//#define DEBUG_ARCCIRCLE

	#ifdef DEBUG_ARCCIRCLE
	cerr << "arccircle: " << x << ", " << y << ", " << radius << ", " <<
		arcbeg << ", " << arcend << endl;
	#endif

	lastx= x + radius;
	lasty= y;
	transform_x (x);
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
	setactivecolor (pforeground);
	lastx= ox+ rx;
	lasty= oy;

	transform_x (ox);
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
	setactivecolor (pforeground);
	lastx= ox+ rx;
	lasty= oy;

	transform_x (ox);
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

int symbol_after_is;

class BlWindow {
	size_t collecting_params;
	std::string params;
	unsigned char actual_control;
	struct DefControlChar {
		size_t nparams;
		bool force;
		void (BlWindow::* action) ();
		DefControlChar (size_t nparams, bool force,
				void (BlWindow::* action) () ) :
			nparams (nparams),
			force (force),
			action (action)
		{ }
		// Default constructor required by the initialization
		// of the escape map (I don't want to use insert
		// instead of [ ] ).
		DefControlChar ()
		{ }
	};
	static const DefControlChar control [];
	typedef std::map <char, DefControlChar> escape_t;
	static const escape_t escape;
	static escape_t init_escape ();
	void ignore ()
	{ /* Nothing to do */ }
	void do_SOH () // 1
	{
		do_charout (params [0] );
	}
	void do_STX () // 2
	{
		cursor_visible= false;
	}
	void do_ETX () // 3
	{
		cursor_visible= true;
	}
	void do_ENQ () // 5
	{
		tag_charout (params [0] );
	}
	void do_BEL () // 7
	{
		graphics::ring ();
	}
	void do_BS () // 8
	{
		--x;
	}
	void do_TAB () // 9
	{
		++x;
	}
	void do_LF () // 10
	{
		//x= 0;
		++y;
	}
	void do_VT () // 11
	{
		--y;
	}
	void do_FF () // 12
	{
		cls ();
	}
	void do_CR () // 13
	{
		x= 0;
	}
	void do_SO () // 14
	{
		setbackground (static_cast <unsigned char> (params [0] ) % 16);
	}
	void do_SI () // 15
	{
		setcolor (static_cast <unsigned char> (params [0] ) % 16 );
	}
	void do_DLE () // 16
	{
		clear_rectangle (x, x, y, y);
	}
	void do_DC1 () // 17
	{
		clear_from_left ();
	}
	void do_DC2 () // 18
	{
		clear_to_right ();
	}
	void do_DC3 () // 19
	{
		clear_from_begin ();
	}
	void do_DC4 () // 20
	{
		clear_to_end ();
	}
	void do_SYN () // 22
	{
		graphics::settransparent
			(static_cast <unsigned char> (params [0] ) );
	}
	void do_ETB () // 23
	{
		// Ink mode, only CPC modes are allowed.
		graphics::setdrawmode
			(static_cast <unsigned char> (params [0] ) % 4);
	}
	void do_CAN () // 24
	{
		std::swap (foreground, background);
	}
	void do_EM () // 25
	{
		int symbol= static_cast <unsigned char> (params [0] );
		// Avoid generate an error if out of range.
		if (symbol < symbol_after_is)
			return;
		unsigned char byte [8];
		params.copy (reinterpret_cast <char *> (& byte [0] ), 8, 1);
		graphics::definesymbol (symbol, byte);
	}
	void do_SUB () // 26
	{
		set (static_cast <unsigned char> (params [0] ),
			static_cast <unsigned char> (params [1] ),
			static_cast <unsigned char> (params [2] ),
			static_cast <unsigned char> (params [3] ) );
	}
	void do_ESC () // 27
	{
		char c= params [0];
		escape_t::const_iterator it= escape.find (c);
		if (it == escape.end () )
		{
			do_charout (c);
			return;
		}
		const DefControlChar & defcontrol= it->second;
		if (defcontrol.nparams == 0)
		{
			if (defcontrol.force)
				forcelegalposition ();
			(this->* defcontrol.action) ();
			return;
		}
		collecting_params= defcontrol.nparams;
		// All escapes used have non-control codes,
		// then we can use the character code
		// without confusion.
		ASSERT (c < 0 || c >= 32); 
		actual_control= c;
		params.erase ();
	}
	void do_FS () // 28
	{
		int ink= static_cast <unsigned char> (params [0] ) % 16;
		int color= static_cast <unsigned char> (params [1] ) % 32;
		if (color > 26)
			return;
		// Third parameter (flashing ink) ignored.
		graphics::ink (ink, color);
	}
	void do_RS () // 30
	{
		x= 0; y= 0;
	}
	void do_US () // 31
	{
		int xx= static_cast <unsigned char> (params [0] );
		int yy= static_cast <unsigned char> (params [1] );
		if (xx == 0 || yy == 0)
			return;
		x= xx - 1; y= yy - 1;
	}
	void do_ESC_A ()
	{
		if (y > 0)
			--y;
	}
	void do_ESC_B ()
	{
		if (y < height - 1)
			++y;
	}
	void do_ESC_C ()
	{
		if (x < width - 1)
			++x;
	}
	void do_ESC_D ()
	{
		if (x > 0)
			--x;
	}
	void do_ESC_E ()
	{
		clear_rectangle (0, width - 1, 0, height - 1);
	}
	void do_ESC_H ()
	{
		x= 0; y= 0;
	}
	void do_ESC_I ()
	{
		--y;
	}
	void do_ESC_J ()
	{
		clear_to_end ();
	}
	void do_ESC_K ()
	{
		clear_to_right ();
	}
	void do_ESC_L ()
	{
		textscrollinverse (y);
	}
	void do_ESC_M ()
	{
		textscroll (y);
	}
	void do_ESC_N ()
	{
		deletechar ();
	}
	void do_ESC_Y ()
	{
		y= static_cast <unsigned char> (params [0] ) - 32;
		x= static_cast <unsigned char> (params [1] ) - 32;
		if (y > height - 1)
			y= height - 1;
		if (x > width - 1)
			x= width - 1;
	}
	void do_ESC_d ()
	{
		clear_from_begin ();
	}
	void do_ESC_e ()
	{
		cursor_visible= true;
	}
	void do_ESC_f ()
	{
		cursor_visible= false;
	}
	void do_ESC_j ()
	{
		savex= x; savey= y;
	}
	void do_ESC_k ()
	{
		x= savex; y= savey;
	}
	void do_ESC_l ()
	{
		clear_rectangle (0, width - 1, y, y);
	}
	void do_ESC_o ()
	{
		clear_from_left ();
	}
	void do_ESC_p ()
	{
		inverse= true;
		//std::swap (foreground, background);
	}
	void do_ESC_q ()
	{
		inverse= false;
	}
	void do_ESC_r ()
	{
		underline= true;
	}
	void do_ESC_u ()
	{
		underline= false;
	}
	void do_ESC_x ()
	{
		// set mode 24 x 80
		setdefault ();
		cls ();
		set (0, 79, 0, 23);
	}
	void do_ESC_y ()
	{
		// unset mode 24 x 80
		setdefault ();
		cls ();
	}
public:
	BlWindow () :
		collecting_params (0),
		fTag (false),
		cursor_visible (true),
		inverse (false),
		underline (false)
	{
		setdefault ();
		defaultcolors ();
	}
	BlWindow (int x1, int x2, int y1, int y2) :
		collecting_params (0),
		fTag (false),
		cursor_visible (true),
		inverse (false),
		underline (false)
	{
		set (x1, x2, y1, y2);
		defaultcolors ();
	}
	void setdefault ()
	{
		TraceFunc tr ("BlWindow::setdefault");

		set (0, maxtcol - 1, 0, maxtrow - 1);
	}
	void set (int x1, int x2, int y1, int y2)
	{
		TraceFunc tr ("BlWindow::set");

		if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0)
		{
			tr.message ("Invalid window values");
			throw ErrImproperArgument;
		}
		if (x1 > x2) std::swap (x1, x2);
		if (y1 > y2) std::swap (y1, y2);
		if (x1 >= maxtcol) x1= maxtcol - 1;
		if (x2 >= maxtcol) x2= maxtcol - 1;
		if (y1 >= maxtrow) y1= maxtrow - 1;
		if (y2 >= maxtrow) y2= maxtrow - 1;
		orgx= x1; orgy= y1;
		width= x2 - x1 + 1;
		height= y2 - y1 + 1;
		x= y= 0;
	}
	void defaultcolors ()
	{
		pen= default_pen;
		paper= default_paper;
		foreground= mapcolor (default_pen).pc;
		background= mapcolor (default_paper).pc;
	}
	void setinverse (bool active) { inverse= active; }
	bool getinverse () { return inverse; }
	int getwidth () const { return width; }
	int getxpos () const { return x; }
	int getypos () const { return y; }
	void gotoxy (int x, int y)
	{
		this->x= x; this->y= y;
	}
	void forcelegalposition ()
	{
		if (x >= width)
			{ x= 0; ++y; }
		if (x < 0)
			{ x= width - 1; --y; }
		if (y < 0)
		{
			y= 0;
			textscrollinverse (0);
		}
		if (y >= height)
		{
			textscroll (0);
			y= height - 1;
		}
	}
	void tab ()
	{
		forcelegalposition ();
		if (x >= (width / zone) * zone)
		{
			//cerr << "Fin de linea" << endl;
			int yy= orgy + y;
			for ( ; x < width; ++x)
				print (orgx + x, yy, ' ', inverse, underline);
			x= 0;
			++y;
		}
		else
		{
			int yy= orgy + y;
			do {
				print (orgx + x, yy, ' ', inverse, underline);
				++x;
			} while (x % zone);
		}
	}
	void tab (size_t n)
	{
		forcelegalposition ();
		int col= n;
		if (x > col)
		{
			do {
				charout (' ');
			} while (x < width);
		}
		int maxpos= std::min (col, width);
		while (x < maxpos)
			charout (' ');
		x= col;
	}
	void setcolor (int color)
	{
		pen= color;
		foreground= mapcolor (color).pc;
	}
	int getcolor ()
	{
		return pen;
	}
	void setbackground (int color)
	{
		paper= color;
		background= mapcolor (color).pc;
	}
	int getbackground ()
	{
		return paper;
	}
	void movecharforward ()
	{
		forcelegalposition ();
		++x;
	}
	void movecharforward (size_t n)
	{
		for ( ; n > 0; --n)
			movecharforward ();
	}
	void movecharback ()
	{
		forcelegalposition ();
		--x;
	}
	void movecharback (size_t n)
	{
		for ( ; n > 0; --n)
			movecharback ();
	}
	void movecharup ()
	{
		forcelegalposition ();
		--y;
	}
	void movecharup (size_t n)
	{
		for ( ; n > 0; --n)
			movecharup ();
	}
	void movechardown ()
	{
		forcelegalposition ();
		++y;
	}
	void movechardown (size_t n)
	{
		for ( ; n > 0; --n)
			movechardown ();
	}
	void clear_from_left ()
	{
		clear_rectangle (0, x, y, y);
	}
	void clear_to_right ()
	{
		clear_rectangle (x, width - 1, y, y);
	}
	void clear_from_begin ()
	{
		if (y > 0)
			clear_rectangle (0, width - 1, 0, y - 1);
		clear_from_left ();
	}
	void clear_to_end ()
	{
		clear_to_right ();
		if (y < height - 1)
			clear_rectangle (0, width - 1, y + 1, height - 1);
	}
	void cls ()
	{
		x= y= 0;
		clear_rectangle (0, width - 1, 0, height - 1);
	}
	void clear_rectangle (int left, int right, int top, int bottom)
	{
		int x1= (orgx + left) * charwidth;
		int w= (right - left + 1) * charwidth;
		int y1= (orgy + top) * charheight;
		int h= (bottom - top + 1) * charheight;
		#ifdef BLASSIC_USE_WINDOWS
		RECT r= { x1, y1, x1 + w, y1 + h };
	        LOGPEN logpen;
        	GetObject (* background, sizeof (LOGPEN), & logpen);
		HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
		if (! fSynchro)
			FillRect (hdc, & r, hbrush);
		FillRect (hdcPixmap, & r, hbrush);
		DeleteObject (hbrush);
		#endif

		#ifdef BLASSIC_USE_X
		setactivecolor (background);
		XSetFunction (display, gcp, drawmode_copy);
		XFillRectangle (display, pixmap, gcp,
			x1, y1, w, h);
		XSetFunction (display, gcp, drawmode);
		if (! fSynchro)
		{
			XSetFunction (display, gc, drawmode_copy);
			XFillRectangle (display, window, gc,
				x1, y1, w, h);
			XSetFunction (display, gc, drawmode);
			// Inserted an idle call because without it
			// the window sometimes is not updated.
			graphics::idle ();
		}
		setactivecolor (pforeground);
		#endif
	}
	void deletechar ()
	{
		int x1= (orgx + x) * charwidth;
		int y1= (orgy + y) * charheight;
		int w= (width - x - 1) * charwidth;
		int h= charheight;

		#ifdef BLASSIC_USE_X

		XSetFunction (display, gcp, drawmode_copy);
		XCopyArea (display, pixmap, pixmap, gcp,
			x1 + charwidth, y1, w, h, x1, y1);
		setactivecolor (background);
		XFillRectangle (display, pixmap, gcp,
			x1 + w, y1, charwidth, h);
		if (! fSynchro)
			reinit_window (x1, y1, w + charwidth, h);
		setactivecolor (foreground);

		#elif defined BLASSIC_USE_WINDOWS

		RECT r= { x1 + w, y1, x1 + w + charwidth, y1 + h };
		BitBlt (hdcPixmap, x1, y1, w, h,
                        hdcPixmap, x1 + charwidth, y1, SRCCOPY);
		LOGPEN logpen;
		GetObject (* background, sizeof (LOGPEN), & logpen);
		HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
		FillRect (hdcPixmap, & r, hbrush);
		DeleteObject (hbrush);
		if (! fSynchro)
			reinit_window (x1, y1, w + charwidth, h);

		#endif
	}
	void textscroll (int fromline)
	{
		int x1= orgx * charwidth;
		int y1= (orgy + fromline) * charheight;
		int w= width * charwidth;
		int h= (height - 1 - fromline) * charheight;

		#ifdef BLASSIC_USE_X

		XSetFunction (display, gcp, drawmode_copy);
		XCopyArea (display, pixmap, pixmap, gcp,
			x1, y1 + charheight, w, h, x1, y1);
		setactivecolor (background);
		XFillRectangle (display, pixmap, gcp,
			x1, y1 + h, w, charheight);
		XSetFunction (display, gcp, drawmode);

		if (! fSynchro)
			reinit_window (x1, y1, w, h + charheight);
		setactivecolor (foreground);

		#elif defined BLASSIC_USE_WINDOWS

		RECT r = { x1, y1 + h, x1 + w, y1 + h + charheight};
		BitBlt (hdcPixmap, x1, y1, w, h,
			hdcPixmap, x1, y1 + charheight, SRCCOPY);
		LOGPEN logpen;
		GetObject (* background, sizeof (LOGPEN), & logpen);
		HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
		FillRect (hdcPixmap, & r, hbrush);
		DeleteObject (hbrush);
		if (! fSynchro)
			reinit_window (x1, y1, w, h + charheight);

		#endif
	}
	void textscrollinverse (int fromline)
	{
		int x1= orgx * charwidth;
		int y1= (orgy + fromline) * charheight;
		int w= width * charwidth;
		int h= (height - 1 - fromline) * charheight;

		#ifdef BLASSIC_USE_X

		XSetFunction (display, gcp, drawmode_copy);
		XCopyArea (display, pixmap, pixmap, gcp,
			x1, y1, w, h, x1, y1 + charheight);
		setactivecolor (background);
		XFillRectangle (display, pixmap, gcp,
			x1, y1, w, charheight);
		XSetFunction (display, gcp, drawmode);

		if (! fSynchro)
			reinit_window (x1, y1, w, h + charheight);
		setactivecolor (foreground);

		#elif defined BLASSIC_USE_WINDOWS

		RECT r = { x1, y1, x1 + w, y1 + charheight};
		BitBlt (hdcPixmap, x1, y1 + charheight, w, h,
			hdcPixmap, x1, y1, SRCCOPY);
		LOGPEN logpen;
		GetObject (* background, sizeof (LOGPEN), & logpen);
		HBRUSH hbrush= CreateSolidBrush (logpen.lopnColor);
		FillRect (hdcPixmap, & r, hbrush);
		DeleteObject (hbrush);
		if (! fSynchro)
			reinit_window (x1, y1, w, h + charheight);

		#endif
	}
	void tag_charout (char c)
	{
		int x= lastx, y= lasty;
		lastx+= charwidth;
		transform_x (x);
		transform_y (y);
		printxy (x, y, c, true);
	}
	void charout (char c)
	{
		if (fTag)
		{
			tag_charout (c);
			return;
		}
		if (collecting_params)
		{
			params+= c;
			if (--collecting_params == 0)
			{
				const DefControlChar & defcontrol=
					actual_control < 32 ?
					control [actual_control] :
					escape.find (actual_control)->second;
				if (defcontrol.force)
					forcelegalposition ();
				(this->* defcontrol.action) ();
			}
			return;
		}
		if (c >= 0 && c < 32)
		{
			actual_control= c;
			params.erase ();
			const DefControlChar & defcontrol=
				control [actual_control];
			if (defcontrol.nparams > 0)
				collecting_params= defcontrol.nparams;
			else
			{
				if (defcontrol.force)
					forcelegalposition ();
				(this->* defcontrol.action) ();
			}
			return;
		}
		forcelegalposition ();
		pcolor foresave= pforeground;
		pforeground= foreground;
		pcolor backsave= pbackground;
		pbackground= background;
		switch (c)
		{
		case '\n':
                	x= 0;
                	++y;
			break;
		case '\b':
			--x;
			break;
		case '\r':
			x= 0;
			break;
		case '\t':
			if (x >= (width / zone) * zone)
			{
				//cerr << "Fin de linea" << endl;
				int yy= orgy + y;
				for ( ; x < width; ++x)
					print (orgx + x, yy, ' ',
						inverse, underline);
				x= 0;
				++y;
			}
			else
			{
				int yy= orgy + y;
				do {
					print (orgx + x, yy, ' ',
						inverse, underline);
					++x;
				} while (x % zone);
			}
			break;
		default:
			print (orgx + x, orgy + y, c, inverse, underline);
			++x;
		}
		pforeground= foresave;
		pbackground= backsave;
	}
	void do_charout (char c)
	{
		pcolor foresave= pforeground;
		pforeground= foreground;
		pcolor backsave= pbackground;
		pbackground= background;
		print (orgx + x, orgy + y, c, inverse, underline);
		pforeground= foresave;
		pbackground= backsave;
		++x;
	}
	void invertcursor ()
	{
		forcelegalposition ();
		if (! cursor_visible)
			return;
		int x1= (orgx + x) * charwidth;
		int y1= (orgy + y) * charheight;
		int y1ini= y1 + charheight - 2;

		#ifdef BLASSIC_USE_X

		XSetFunction (display, gc, drawmode_invert);
		XSetFunction (display, gcp, drawmode_invert);
		XFillRectangle (display, window, gc,
			x1, y1ini, charwidth, 2);
		XFillRectangle (display, pixmap, gcp,
			x1, y1ini, charwidth, 2);
		XSetFunction (display, gc, drawmode);
		XSetFunction (display, gcp, drawmode);

		#elif defined BLASSIC_USE_WINDOWS

		HBRUSH hbrush= (HBRUSH) GetStockObject (BLACK_BRUSH);
		HDC ahdc [2]= { hdc, hdcPixmap };
		for (size_t i= 0; i < 2; ++i)
		{
			HDC hdc= ahdc [i];
			SetROP2 (hdc, drawmode_invert);
			HBRUSH hold= (HBRUSH) SelectObject (hdc, hbrush);
			//Rectangle (hdc, x1, y1 + 6, x1 + 8, y1 + 8);
			Rectangle (hdc, x1, y1ini,
				x1 + charwidth, y1 + charheight);
			SelectObject (hdc, hold);
			SetROP2 (hdc, drawmode);
		}

		#endif
	}
	std::string copychr ()
	{
		// I don't tested yet if that is done in the cpc
		forcelegalposition ();

		int x1= (orgx + x) * charwidth;
		int y1= (orgy + y) * charheight;
		return copychrat (x1, y1);
	}
	void tag ()
	{
		fTag= true;
	}
	void tagoff ()
	{
		fTag= false;
	}
	bool istagactive ()
	{
		return fTag;
	}
private:
	int orgx, orgy, width, height;
	pcolor foreground;
	pcolor background;
	int x, y, savex, savey;
	bool fTag;
	bool cursor_visible;
	bool inverse;
	bool underline;
	int pen;
	int paper;
};

const BlWindow::DefControlChar BlWindow::control [32]= {
	BlWindow::DefControlChar (0, false, & BlWindow::ignore), // NUL
	BlWindow::DefControlChar (1, true,  & BlWindow::do_SOH), // SOH
	BlWindow::DefControlChar (0, false, & BlWindow::do_STX), // STX
	BlWindow::DefControlChar (0, false, & BlWindow::do_ETX), // ETX
	BlWindow::DefControlChar (1, false, & BlWindow::ignore), // EOT
	BlWindow::DefControlChar (1, false, & BlWindow::do_ENQ), // ENQ
	BlWindow::DefControlChar (0, false, & BlWindow::ignore), // ACK
	BlWindow::DefControlChar (0, false, & BlWindow::do_BEL), // BEL
	BlWindow::DefControlChar (0, true,  & BlWindow::do_BS ), // BS
	BlWindow::DefControlChar (0, true,  & BlWindow::do_TAB), // TAB
	BlWindow::DefControlChar (0, true,  & BlWindow::do_LF ), // LF
	BlWindow::DefControlChar (0, true,  & BlWindow::do_VT ), // VT
	BlWindow::DefControlChar (0, false, & BlWindow::do_FF ), // FF
	BlWindow::DefControlChar (0, true,  & BlWindow::do_CR ), // CR
	BlWindow::DefControlChar (1, false, & BlWindow::do_SO ), // SO
	BlWindow::DefControlChar (1, false, & BlWindow::do_SI ), // SI
	BlWindow::DefControlChar (0, true,  & BlWindow::do_DLE), // DLE
	BlWindow::DefControlChar (0, true,  & BlWindow::do_DC1), // DC1
	BlWindow::DefControlChar (0, true,  & BlWindow::do_DC2), // DC2
	BlWindow::DefControlChar (0, true,  & BlWindow::do_DC3), // DC3
	BlWindow::DefControlChar (0, true,  & BlWindow::do_DC4), // DC4
	BlWindow::DefControlChar (0, false, & BlWindow::ignore), // NAK
	BlWindow::DefControlChar (1, false, & BlWindow::do_SYN), // SYN
	BlWindow::DefControlChar (1, false, & BlWindow::do_ETB), // ETB
	BlWindow::DefControlChar (0, false, & BlWindow::do_CAN), // CAN
	BlWindow::DefControlChar (9, false, & BlWindow::do_EM ), // EM
	BlWindow::DefControlChar (4, false, & BlWindow::ignore), // SUB
	BlWindow::DefControlChar (1, false, & BlWindow::do_ESC), // ESC
	BlWindow::DefControlChar (3, false, & BlWindow::do_FS ), // FS
	BlWindow::DefControlChar (2, false, & BlWindow::ignore), // GS
	BlWindow::DefControlChar (0, false, & BlWindow::do_RS ), // RS
	BlWindow::DefControlChar (2, false, & BlWindow::do_US ), // US
};

BlWindow::escape_t BlWindow::init_escape ()
{
	escape_t aux;
	DefControlChar ignore0 (0, false, & BlWindow::ignore);
	DefControlChar ignore1 (1, false, & BlWindow::ignore);
	aux ['0']= ignore0; // Status line inactive.
	aux ['1']= ignore0; // Status line active.
	aux ['2']= ignore1; // Select national character set
	aux ['3']= ignore1; // Set mode
	aux ['A']= DefControlChar (0, true,  & BlWindow::do_ESC_A);
	aux ['B']= DefControlChar (0, true,  & BlWindow::do_ESC_B);
	aux ['C']= DefControlChar (0, true,  & BlWindow::do_ESC_C);
	aux ['D']= DefControlChar (0, true,  & BlWindow::do_ESC_D);
	aux ['E']= DefControlChar (0, false, & BlWindow::do_ESC_E);
	aux ['H']= DefControlChar (0, false, & BlWindow::do_ESC_H);
	aux ['I']= DefControlChar (0, true,  & BlWindow::do_ESC_I);
	aux ['J']= DefControlChar (0, true,  & BlWindow::do_ESC_J);
	aux ['K']= DefControlChar (0, true,  & BlWindow::do_ESC_K);
	aux ['L']= DefControlChar (0, true,  & BlWindow::do_ESC_L);
	aux ['M']= DefControlChar (0, true,  & BlWindow::do_ESC_M);
	aux ['N']= DefControlChar (0, true,  & BlWindow::do_ESC_N);
	aux ['Y']= DefControlChar (2, false, & BlWindow::do_ESC_Y);
	aux ['d']= DefControlChar (0, true,  & BlWindow::do_ESC_d);
	aux ['e']= DefControlChar (0, false, & BlWindow::do_ESC_e);
	aux ['f']= DefControlChar (0, false, & BlWindow::do_ESC_f);
	aux ['j']= DefControlChar (0, false, & BlWindow::do_ESC_j);
	aux ['k']= DefControlChar (0, false, & BlWindow::do_ESC_k);
	aux ['l']= DefControlChar (0, true,  & BlWindow::do_ESC_l);
	aux ['o']= DefControlChar (0, true,  & BlWindow::do_ESC_o);
	aux ['p']= DefControlChar (0, false, & BlWindow::do_ESC_p);
	aux ['q']= DefControlChar (0, false, & BlWindow::do_ESC_q);
	aux ['r']= DefControlChar (0, false, & BlWindow::do_ESC_r);
	aux ['u']= DefControlChar (0, false, & BlWindow::do_ESC_u);
	aux ['x']= DefControlChar (0, false, & BlWindow::do_ESC_x);
	aux ['y']= DefControlChar (0, false, & BlWindow::do_ESC_y);
	return aux;
}

const BlWindow::escape_t BlWindow::escape= BlWindow::init_escape ();

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
	TraceFunc tr ("recreate_windows");

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
				print (tcol, trow, ' ', false);
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
				print (tcol, trow, ' ', false);
				++tcol;
			} while (tcol % zone);
		}
		return;
	}
        print (tcol, trow, c, false);
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

int graphics::getcolor (BlChannel ch)
{
	return mapwindow [ch]->getcolor ();
}

void graphics::setbackground (BlChannel ch, int color)
{
	mapwindow [ch]->setbackground (color);
}

int graphics::getbackground (BlChannel ch)
{
	return mapwindow [ch]->getbackground ();
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
	for (std::string::size_type i= 0, l= str.size (); i < l; ++i)
		windowzero.charout (str [i]);
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

std::string graphics::copychr (BlChannel ch)
{
	BlWindow * pwin= mapwindow [ch];
	return pwin->copychr ();
}

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

void graphics::tab (BlChannel ch)
{
	mapwindow [ch]->tab ();
}

void graphics::tab (BlChannel ch, size_t n)
{
	mapwindow [ch]->tab (n);
}

void graphics::movecharforward (size_t n)
{
	windowzero.movecharforward (n);
}

void graphics::movecharforward (BlChannel ch, size_t n)
{
	mapwindow [ch]->movecharforward (n);
}

void graphics::movecharback (size_t n)
{
	windowzero.movecharback (n);
}

void graphics::movecharback (BlChannel ch, size_t n)
{
	mapwindow [ch]->movecharback (n);
}

void graphics::movecharup (size_t n)
{
	windowzero.movecharup (n);
}

void graphics::movecharup (BlChannel ch, size_t n)
{
	mapwindow [ch]->movecharup (n);
}

void graphics::movechardown (size_t n)
{
	windowzero.movechardown (n);
}

void graphics::movechardown (BlChannel ch, size_t n)
{
	mapwindow [ch]->movechardown (n);
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
	idle ();
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
int graphics::ypos (BlChannel ch)
{
	return mapwindow [ch]->getypos ();
}

void graphics::tag (BlChannel ch)
{
	mapwindow [ch]->tag ();
}

void graphics::tagoff (BlChannel ch)
{
	mapwindow [ch]->tagoff ();
}

bool graphics::istagactive (BlChannel ch)
{
	return mapwindow [ch]->istagactive ();
}

void graphics::showcursor ()
{
	windowzero.invertcursor ();
}

void graphics::hidecursor ()
{
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

void graphics::inverse (BlChannel ch, bool active)
{
	mapwindow [ch]->setinverse (active);
}

bool graphics::getinverse (BlChannel ch)
{
	return mapwindow [ch]->getinverse ();
}

void graphics::clean_input ()
{
	TraceFunc tr ("graphics::clean_input");

	graphics::idle ();
	queuekey.erase ();
}

void graphics::ring ()
{
	#ifdef BLASSIC_USE_X

	XBell (display, 100);

	#elif defined BLASSIC_USE_WINDOWS

	MessageBeep (MB_ICONEXCLAMATION);

	#endif
}

void graphics::set_title (const std::string & title)
{
	#ifdef BLASSIC_USE_WINDOWS

	SetWindowText (window, title.c_str () );

	#elif defined BLASSIC_USE_X

	XmbSetWMProperties (display, window, title.c_str (), title.c_str (),
		NULL, 0, NULL, NULL, NULL);

	#endif
}

void graphics::set_default_title (const std::string & title)
{
	default_title= title;
}

// Fin de graphics.cpp
