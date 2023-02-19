// cursor.cpp
// Revision 9-jun-2003

#include "cursor.h"
#include "graphics.h"
#include "key.h"
#include "util.h"
#include "trace.h"

#include <map>
#include <sstream>
#include <cstdlib>

#ifdef __WIN32__

#include <windows.h>

#else

#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <errno.h>
#include <cassert>
#define ASSERT assert

#include <ncurses.h>
#include <term.h>

#endif

#if defined __linux__ || defined __unix__

// Stuff needed by getwidth
 
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
  
// This is from ncurses.
#ifdef TIOCGSIZE
# define IOCTL_WINSIZE TIOCGSIZE
# define STRUCT_WINSIZE struct ttysize
# define WINSIZE_ROWS(n) (int)n.ts_lines
# define WINSIZE_COLS(n) (int)n.ts_cols
#else
# ifdef TIOCGWINSZ
#  define IOCTL_WINSIZE TIOCGWINSZ
#  define STRUCT_WINSIZE struct winsize
#  define WINSIZE_ROWS(n) (int)n.ws_row
#  define WINSIZE_COLS(n) (int)n.ws_col
# endif
#endif

#endif

#include <iostream>
#include <map>

namespace {

#ifdef BLASSIC_USE_TERMINFO

bool fInit= true;

int background= 8;

const char
	* strCls= NULL,           * strCup= NULL,
	* strCursorNormal= NULL,  * strCursorInvisible= NULL,
	* strForeground= NULL,    * strBackground= NULL,
	* strEnterBold= NULL,     * strExitBold= NULL,
	* strMoveForward= NULL,   * strMoveBack= NULL,
	* strMoveForwardN= NULL,  * strMoveBackN= NULL,
	* strMoveUp= NULL,        * strMoveDown= NULL,
	* strMoveUpN= NULL,       * strMoveDownN= NULL,
	* strSaveCursorPos= NULL, * strRestoreCursorPos= NULL,
	* strBell= NULL;

const char * newstr (const char * str)
{
	if (str == NULL)
		return NULL;
	size_t l= strlen (str);
	char *n= new char [l + 1];
	strcpy (n, str);
	return n;
}

inline const char * calltigetstr (const char * id)
{
	const char * str= tigetstr ( (char *) id);
	if (str == (char *) -1)
		return NULL;
	return str;
}

inline const char * mytigetstr (const char * id)
{
        return newstr (calltigetstr (id) );
}

int putfunc (int ic)
{
	char c= ic;
	write (STDOUT_FILENO, & c, 1);
	return c;
}

inline void calltputs (const char * str)
{
	if (str != NULL)
		tputs (str, 1, putfunc);
}

inline void calltparm (const char * str, int n)
{
	if (str != NULL)
		calltputs (tparm ( (char *) str, n) );		
}

void initkeytable ();

struct str_terminfo {
	const char * & str;
	const char * tinfoname;
        str_terminfo (const char * & str, const char * tinfoname) :
                str (str), tinfoname (tinfoname)
        { }
};

const str_terminfo strinfo []= {
	str_terminfo (strCls, "clear" ),
	str_terminfo (strCup, "cup" ),

	str_terminfo (strCursorNormal, "cnorm" ),
	str_terminfo (strCursorInvisible, "civis" ),

	str_terminfo (strForeground, "setaf" ),
	str_terminfo (strBackground, "setab" ),

	str_terminfo (strEnterBold, "bold" ),
	str_terminfo (strExitBold, "sgr0" ),

	str_terminfo (strMoveForward, "cuf1" ),
	str_terminfo (strMoveBack, "cub1" ),
	str_terminfo (strMoveForwardN, "cuf" ),
	str_terminfo (strMoveBackN, "cub" ),
	str_terminfo (strMoveUp, "cuu1" ),
	str_terminfo (strMoveDown, "cud1" ),
	str_terminfo (strMoveUpN, "cuu" ),
	str_terminfo (strMoveDownN, "cud" ),

	str_terminfo (strSaveCursorPos, "sc" ),
	str_terminfo (strRestoreCursorPos, "rc" ),

	str_terminfo (strBell, "bel"),
};

void init ()
{
	TraceFunc tr ("init");

	fInit= false;
	int errret;
	setupterm (0, 1, & errret);

	if (isatty (STDOUT_FILENO) )
	{
		#if 0
		//calltputs (tgetstr ( (char *) "smcup", 0) );
		const char * str= calltigetstr ("smcup");
		calltputs (str);
		#endif

		const char * str_keypad_xmit= calltigetstr ("smkx");
		calltputs (str_keypad_xmit);
	}

	for (size_t i= 0; i < util::dim_array (strinfo); ++i)
		strinfo [i].str= mytigetstr (strinfo [i].tinfoname);

	initkeytable ();
}

inline void checkinit ()
{
	if (fInit)
		init ();
}

#endif

} // namespace

void initconsole ()
{
	TraceFunc tr ("initconsole");

	cursorinvisible ();
}

void quitconsole ()
{
	TraceFunc tr ("quitconsole");

	cursorvisible ();

	#ifdef BLASSIC_USE_TERMINFO

	if (! fInit)
	{
		if (isatty (STDOUT_FILENO) )
		{
			#if 0
			//calltputs (tgetstr ( (char *) "rmcup", 0) );
			const char * str= calltigetstr ("rmcup");
			if (str != 0)
				calltputs (str);
			#endif

			const char * str_keypad_local= calltigetstr ("rmkx");
			calltputs (str_keypad_local);
		}

	}

	#endif
}

size_t getwidth ()
{
	size_t width;
	if (graphics::ingraphicsmode () )
		width= graphics::getlinewidth ();
	else
	{
		#ifdef __WIN32__
                HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
                CONSOLE_SCREEN_BUFFER_INFO info;
                if (GetConsoleScreenBufferInfo (h, & info) )
                        width= info.dwSize.X;
                else
                        width= 80;
		#else
		STRUCT_WINSIZE win;
		if (ioctl (0, IOCTL_WINSIZE, & win) == 0)
			width= WINSIZE_COLS (win);
		else
		{
			const char * aux= getenv ("COLUMNS");
			if (aux)
				width= atoi (aux);
			else
				width= 80;
		}
		#endif
	}
	return width;
}

void cursorvisible ()
{
	TraceFunc tr ("cursorvisible");

	// checkinit not needed, is done by showcursor
	showcursor ();

	#ifndef __WIN32__

	struct termios ter;
	tcgetattr (STDIN_FILENO, & ter);
	//ter.c_lflag|= (ECHO | ICANON | PENDIN);
	ter.c_lflag|= (ECHO | ICANON);
	tcsetattr (STDIN_FILENO, TCSANOW, & ter);

        #endif
}

void cursorinvisible ()
{
	TraceFunc tr ("cursorinvisible");

	// checkinit not needed, is done by hidecursor
	hidecursor ();

	#ifndef __WIN32__

	struct termios ter;
	tcgetattr (STDIN_FILENO, & ter);
	ter.c_lflag&= ~ (ECHO | ICANON);
	ter.c_cc [VMIN]= 1;
	tcsetattr (STDIN_FILENO, TCSANOW, & ter);

        #endif
}

void showcursor ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::showcursor ();
		return;
	}

	#ifdef __WIN32__

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo (h, & info);
        info.bVisible= TRUE;
        SetConsoleCursorInfo (h, & info);

	#elif defined BLASSIC_USE_TERMINFO

	checkinit ();

	if (isatty (STDOUT_FILENO) )
		calltputs (strCursorNormal );

	#endif
}

void hidecursor ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::hidecursor ();
		return;
	}

	#ifdef __WIN32__

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo (h, & info);
        info.bVisible= FALSE;
        SetConsoleCursorInfo (h, & info);

	#elif defined BLASSIC_USE_TERMINFO

	checkinit ();

	if (isatty (STDOUT_FILENO) )
		calltputs (strCursorInvisible);

	#endif
}

#ifdef __WIN32__

const WORD init_attributes=
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
WORD attributes= init_attributes;

#endif

void cls ()
{
	#ifdef __WIN32__

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        if (h != INVALID_HANDLE_VALUE)
        {
                CONSOLE_SCREEN_BUFFER_INFO info;
                if (GetConsoleScreenBufferInfo (h, & info) )
                {
                        DWORD l= info.dwSize.X * info.dwSize.Y;
                        COORD coord= {0, 0};
                        DWORD notused;
                        FillConsoleOutputAttribute (h,
                                attributes,
                                l, coord, & notused);
                        FillConsoleOutputCharacter (h,
                        	' ', l, coord, & notused);
                        SetConsoleCursorPosition (h, coord);
                }
        }

	#elif defined BLASSIC_USE_TERMINFO

	checkinit ();

	calltputs (strCls);

	#endif
}

#if 0
void locate (int row, int col)
{
	#ifdef BLASSIC_USE_WINDOWS

        COORD coord= { SHORT (col - 1), SHORT (row - 1) };
        SetConsoleCursorPosition (GetStdHandle (STD_OUTPUT_HANDLE), coord);

	#elif defined BLASSIC_USE_TERMINFO

	checkinit ();

	if (strCup)
		calltputs (tgoto (strCup, col - 1, row - 1) );

	#endif
}
#endif

void gotoxy (int x, int y)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::gotoxy (x, y);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        COORD coord= { SHORT (x), SHORT (y) };
        SetConsoleCursorPosition (GetStdHandle (STD_OUTPUT_HANDLE), coord);

	#elif defined BLASSIC_USE_TERMINFO

	checkinit ();

	if (strCup)
		calltputs (tgoto (strCup, x, y) );

	#endif
}

int getcursorx ()
{
	#ifdef BLASSIC_USE_WINDOWS

	HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (GetConsoleScreenBufferInfo (h, & info) == 0)
		return 0;
	return info.dwCursorPosition.X;

	#else

	return 0;

	#endif
}

void movecharforward ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharforward (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movecharforward (1);

	#elif defined BLASSIC_USE_TERMINFO

	calltputs (strMoveForward);

	#endif
}

void movecharback ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharback (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movecharback (1);

	#elif defined BLASSIC_USE_TERMINFO

	calltputs (strMoveBack);

	#endif
}

void movecharup ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharup (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movecharup (1);

	#elif defined BLASSIC_USE_TERMINFO

	calltputs (strMoveUp);

	#endif
}

void movechardown ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movechardown (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movechardown (1);

	#elif defined BLASSIC_USE_TERMINFO

	calltputs (strMoveDown);

	#endif
}

namespace {

#ifdef BLASSIC_USE_TERMINFO

inline void auxmovechar (const char * strN, const char * str, size_t n)
{
	if (n != 0)
	{
		if (strN)
			//calltputs (tparm ( (char *) strN, n) );
			calltparm (strN, n);
		else
			if (str)
				for (size_t i= 0; i < n; ++i)
					calltputs (str);
	}
}

#endif

#ifdef __WIN32__



#endif

} // namespace

void movecharforward (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharforward (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.X+= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#elif defined BLASSIC_USE_TERMINFO

	auxmovechar (strMoveForwardN, strMoveForward, n);

	#endif
}

void movecharback (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharback (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.X-= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#elif defined BLASSIC_USE_TERMINFO

	auxmovechar (strMoveBackN, strMoveBack, n);

	#endif
}

void movecharup (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharup (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.Y-= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#elif defined BLASSIC_USE_TERMINFO

	auxmovechar (strMoveUpN, strMoveUp, n);

	#endif
}

void movechardown (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movechardown (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.Y+= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#elif defined BLASSIC_USE_TERMINFO

	auxmovechar (strMoveDownN, strMoveDown, n);

	#endif
}

void savecursorpos ()
{
	#ifdef BLASSIC_USE_WINDOWS

	#elif defined BLASSIC_USE_TERMINFO

	calltputs (strSaveCursorPos);

	#endif
}

void restorecursorpos ()
{
	#ifdef BLASSIC_USE_WINDOWS

	#elif defined BLASSIC_USE_TERMINFO

	calltputs (strRestoreCursorPos);

	#endif
}


#ifndef __WIN32__

namespace {

static const int newcolor []=
	{ 0, 4, 2, 6, 1, 5, 3, 7};

inline int mapcolor (int n)
{
	// Intensity bit unchanged
	return newcolor [n & 7] | (n & 8);
}

}

#endif

void textcolor (int color)
{
        #ifdef __WIN32__

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        attributes= (attributes & WORD (0xF0) ) | WORD (color & 0x0F);
        SetConsoleTextAttribute (h, attributes);

	#elif defined BLASSIC_USE_TERMINFO

	color= mapcolor (color & 0xF);
	bool intensity= color > 7;
	if (intensity)
	{
		color&= 7;
		calltputs (strEnterBold);
	}
	else
	{
		if (strExitBold)
		{
			calltputs (strExitBold);
			// sgr0 reset the background, then we need to set it.
			textbackground (background);
		}
	}
	//if (strForeground)
	//	calltputs (tparm ( (char *) strForeground, color) );
	calltparm (strForeground, color);

        #endif
}

void textbackground (int color)
{
        #ifdef __WIN32__
        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        attributes= (attributes & WORD (0xF) ) | WORD ( (color & 0xF) << 4);
        SetConsoleTextAttribute (h, attributes);

	#else

	background= color;
	color= mapcolor (color & 0xF);
	//if (strBackground)
	//	calltputs (tparm ( (char *) strBackground, color) );
	calltparm (strBackground, color);

        #endif
}

namespace {

enum ReadType { ReadWait, ReadNoWait };

#ifdef __WIN32__

std::string string_from_key_event (const KEY_EVENT_RECORD & kr)
{
        char c= kr.uChar.AsciiChar;
	if (c != '\0')
                return std::string (1, c);
        WORD k= kr.wVirtualKeyCode;
        std::string str= string_from_key (k);
        if (! str.empty () )
                return str;

        if (k != VK_SHIFT &&
                k != VK_CONTROL &&
                k != VK_MENU &&
                k != VK_CAPITAL &&
                k != VK_NUMLOCK &&
                k != VK_SCROLL)
        {
                std::string str (1, '\0');
                str+= char (kr.wVirtualScanCode);
                return str;
        }
        return std::string ();
}

std::string string_from_input (const INPUT_RECORD & input)
{
        std::string str;
        if (input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown)
        {
                str= string_from_key_event (input.Event.KeyEvent);
        }
        return str;
}

std::string readkey (ReadType type)
{
        std::string str;

        HANDLE h= GetStdHandle (STD_INPUT_HANDLE);
        DWORD mode, orgmode;
        GetConsoleMode (h, & mode);
        orgmode= mode;
        //mode&= ~ (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
        mode= 0;
        SetConsoleMode (h, mode);
        DWORD n= 0;
        INPUT_RECORD input;
        if (type == ReadNoWait)
                PeekConsoleInput (h, & input, 1, & n);
        else
                n= 1;
        if (n)
        {
        sigue:
                ReadConsoleInput (h, & input, 1, & n);
                str= string_from_input (input);
                if (type == ReadWait && str.empty () )
                        goto sigue;
        }
        SetConsoleMode (h, orgmode);

        return str;
}

#elif defined BLASSIC_USE_TERMINFO

class MapSpecial {
public:
	enum Result { NoMapped, Found, MoreNeeded };
	void addkey (const std::string & str, std::string::size_type pos,
		const std::string & keyname)
	{
		//TraceFunc tr ("MapSpecial::addkey");

		ASSERT (pos < str.size () );
		char c= str [pos];
		if (pos == str.size () - 1)
			kname [c]= keyname;
		else
		{
			//if (kmap.find (c) == kmap.end () )
			//	kmap [c]= MapSpecial ();
			kmap [c].addkey (str, pos + 1, keyname);
		}
	}
	Result findkey (const std::string & str, std::string::size_type pos,
		std::string & keyname, std::string::size_type & consumed)
	{
		if (pos >= str.size () )
			return MoreNeeded;
		char c= str [pos];
		//cout << "Buscando: " << c << endl;
		{
			std::map <char, std::string>::iterator it= kname.find (c);
			if (it != kname.end () )
			{
				keyname= it->second;
				consumed= pos;
				return Found;
			}
		}
		std::map <char, MapSpecial>::iterator it= kmap.find (c);
		if (it != kmap.end () )
			return it->second.findkey
				(str, pos + 1, keyname, consumed);
		else
			return NoMapped;
	}
private:
	std::map <char, std::string> kname;
	std::map <char, MapSpecial> kmap;
};

struct KeyDescription {
	const char * tiId;
	//const char * blName;
	const std::string & blName;
        KeyDescription (const char * tiId, const std::string & blName) :
                tiId (tiId),
                blName (blName)
        { }
};

const std::string
	strMULT ("*"),
	strMINUS ("-"),
	strPLUS ("+"),
	strDIV ("/");

const KeyDescription keyname [] = {
	KeyDescription ("kpp",   strPAGEUP),    // previous-page key
	KeyDescription ("knp",   strPAGEDOWN),  // next-page key
	KeyDescription ("kend",  strEND),       // end key
	KeyDescription ("kslt",  strEND),       // select key
	KeyDescription ("kc1",   strEND),       // lower left of keypad
	KeyDescription ("khome", strHOME),      // home key
	KeyDescription ("kfnd",  strHOME),      // find key
	KeyDescription ("ka1",   strHOME),      // upper left of keypad
	KeyDescription ("kcub1", strLEFT),      // left-arrow key
	KeyDescription ("kcuu1", strUP),        // up-arrow key
	KeyDescription ("kcuf1", strRIGHT),     // right-arrow key
	KeyDescription ("kcud1", strDOWN),      // down-arrow key
	KeyDescription ("kich1", strINSERT),    // insert-character key
	KeyDescription ("kdch1", strDELETE),    // delete-character key
	KeyDescription ("kent",  strENTER),     // enter/send key
	KeyDescription ("kf1",   strF1),        // F1 function key
	KeyDescription ("kf2",   strF2),        // F2 function key
	KeyDescription ("kf3",   strF3),        // F3 function key
	KeyDescription ("kf4",   strF4),        // F4 function key
	KeyDescription ("kf5",   strF5),        // F5 function key
	KeyDescription ("kf6",   strF6),        // F6 function key
	KeyDescription ("kf7",   strF7),        // F7 function key
	KeyDescription ("kf8",   strF8),        // F8 function key
	KeyDescription ("kf9",   strF9),        // F9 function key
	KeyDescription ("kf10",  strF10),       // F10 function key
	KeyDescription ("kf11",  strF11),       // F11 function key
	KeyDescription ("kf12",  strF12),       // F12 function key
	KeyDescription ("kf54",  strDIV),       // F54 function key, / in xterm
	KeyDescription ("kf55",  strMULT),      // F55 function key, * in xterm
	KeyDescription ("kf56",  strMINUS),     // F56 function key, - in xterm
	KeyDescription ("kf57",  strPLUS),      // f57 function key, + in xterm
};

#ifndef NDEBUG

bool checktable ()
{
	const size_t nkeys= util::dim_array (keyname);

	for (size_t i= 0; i < nkeys - 1; ++i)
		for (size_t j= i + 1; j < nkeys; ++j)
			if (strcmp (keyname [i].tiId, keyname [j].tiId) == 0)
			{
				std::cerr << "Code repeated in keyname: " <<
					keyname [i].tiId << std::endl;
				throw 1;
			}
	return true;
}

bool tablechecked= checktable ();

#endif

MapSpecial ms;

void initkeytable ()
{
	TraceFunc tr ("initkeytable");

	const size_t nkeys= util::dim_array (keyname);

	for (size_t i= 0; i < nkeys; ++i)
	{
		const KeyDescription & keydesc= keyname [i];
		const char * const strkey= keydesc.tiId;
		const char * str= calltigetstr (strkey);
		if (str != NULL)
		{
			#if 0
			cerr << keydesc.blName << "=";
			for (size_t i= 0, l= strlen (str); i < l; ++i)
			{
				char c= str [i];
				if (c >= 32) cerr << c;
				else cerr << "\\(" << hex << int (c) << ')';
			}
			cerr << endl;
			#endif
			tr.message (std::string ("Adding ") + keydesc.blName);
			ms.addkey (str, 0, keydesc.blName);
		}
	}

}

class PollInput {
public:
	PollInput ()
	{
		pfd.fd= STDIN_FILENO;
		pfd.events= POLLIN | POLLERR | POLLNVAL;
	}
	int poll ()
	{
		return ::poll (& pfd, 1, 100);
	}
private:
	struct pollfd pfd;
};

void wait_event ()
{
	PollInput pi;
	int r;
	do {
		graphics::idle ();
	} while ( (r= pi.poll () ) == 0);
	if (r < 0)
	{
		std::cerr << "Error in poll: " << strerror (errno) << std::endl;
	}
}

void do_poll ()
{
	PollInput ().poll ();
}

std::string readkey (ReadType type)
{
	checkinit ();

	static std::string charpending;
        std::string str;
	bool reset_blocking_mode= false;
	int l;
	//char c;
	const int lbuf= 32;
	char buffer [lbuf + 1];

	if (! charpending.empty () )
		goto check_it;

	#if 0

        if (type == ReadWait)
        {
		//fcntl (STDIN_FILENO, F_SETFL, 0);
		wait_event ();
	}
	else
	{
		fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);
		reset_blocking_mode= true;
	}

	//read_another:

	l= read (STDIN_FILENO, & c, 1);
	if (l == 1)
		str+= c;

	#else

	fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);
	reset_blocking_mode= true;
	//l= read (STDIN_FILENO, & c, 1);
	l= read (STDIN_FILENO, buffer, lbuf);
	//if (l != 1 && type == ReadWait)
	if (l < 1 && type == ReadWait)
	{
		do {
			//wait_event ();
			//l= read (STDIN_FILENO, & c, 1);
			do_poll ();
			l= read (STDIN_FILENO, buffer, lbuf);
		//} while (l != 1);
		} while (l < 1);
	}
	//if (l == 1)
	//	str+= c;
	if (l >= 1)
	{
		buffer [l]= '\0';
		str+= buffer;
	}

	#endif

	read_another:

	//std::cerr << "Adding: >" << str << '<' << std::endl;	
	charpending+= str;
	str.erase ();

	check_it:

	std::string keyname;
	std::string::size_type pos;
	if (! charpending.empty () )
	{
		MapSpecial::Result r=
			ms.findkey (charpending, 0, keyname, pos);
		switch (r)
		{
		case MapSpecial::NoMapped:
			str= charpending [0];
			charpending.erase (0, 1);
			break;
		case MapSpecial::Found:
			str= keyname;
			charpending.erase (0, pos + 1);
			break;
		case MapSpecial::MoreNeeded:
			fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);
			reset_blocking_mode= true;
			do_poll ();
			//l= read (STDIN_FILENO, & c, 1);
			//if (l == 1)
			//{
			//	str= c;
			//	goto read_another;
			//}
			l= read (STDIN_FILENO, buffer, lbuf);
			if (l >= 1)
			{
				buffer [l]= '\0';
				str= buffer;
				goto read_another;
			}
			str= charpending [0];
			charpending.erase (0, 1);
			break;
		}
	}

        if (type == ReadWait)
                cursorinvisible ();

	if (reset_blocking_mode)
		fcntl (STDIN_FILENO, F_SETFL, 0);

        return str;
}

#endif

} // namespace

std::string inkey ()
{
        if (graphics::ingraphicsmode () )
                return graphics::getkey ();
        return readkey (ReadNoWait);
}

std::string getkey ()
{
        if (graphics::ingraphicsmode () )
        {
                std::string str;
                do {
                        str= graphics::getkey ();
                } while (str.empty () );
                return str;
        }
        return readkey (ReadWait);
}


void clean_input ()
{
	TraceFunc tr ("clean_input");

	if (graphics::ingraphicsmode () )
	{
		graphics::clean_input ();
	}
	else
	{
		#ifdef __WIN32__

		Sleep (100);
		HANDLE h= GetStdHandle (STD_INPUT_HANDLE);
		INPUT_RECORD input;
		DWORD n= 0;
		PeekConsoleInput (h, & input, 1, & n);
		if (n && input.EventType == KEY_EVENT &&
				! input.Event.KeyEvent.bKeyDown)
			ReadConsoleInput (h, & input, 1, & n);

		#else

		fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);
		int l;
		const int lbuf= 32;
		char buffer [lbuf + 1];
		do
		{
			l= read (STDIN_FILENO, buffer, lbuf);
		} while (l > 0);
		fcntl (STDIN_FILENO, F_SETFL, 0);

		#endif
	}
}

void ring ()
{
	#ifdef BLASSIC_USE_WINDOWS

	MessageBeep (MB_ICONEXCLAMATION);

	#elif defined BLASSIC_USE_TERMINFO

	if (graphics::ingraphicsmode () )
		graphics::ring ();
	else
		calltputs (strBell);

	#endif
}

//************************************************
//	set_title
//************************************************

#ifdef BLASSIC_USE_TERMINFO

// Escape sequences from the "How to change the title of an xterm",
// by Ric Lister, http://www.tldp.org/HOWTO/mini/Xterm-Title.html

namespace
{

void set_title_xterm (const std::string & title)
{
	std::ostringstream oss;
	oss << "\x1B]0;" << title << "\x07";
	const std::string str= oss.str ();
	write (STDOUT_FILENO, str.c_str (), str.size () );
}

void set_title_iris_ansi (const std::string & title)
{
	std::ostringstream oss;
	oss << "\x1BP1.y" << title << "\x1B\\"; // Set window title
	oss << "\x1BP3.y" << title << "\x1B\\"; // Set icon title
	const std::string str= oss.str ();
	write (STDOUT_FILENO, str.c_str (), str.size () );
}

void set_title_sun_cmd (const std::string & title)
{
	std::ostringstream oss;
	oss << "\x1B]l;" << title << "\x1B\\"; // Set window title
	oss << "\x1B]L;" << title << "\x1B\\"; // Set icon title
	const std::string str= oss.str ();
	write (STDOUT_FILENO, str.c_str (), str.size () );
}

void set_title_hpterm (const std::string & title)
{
	std::ostringstream oss;
	const std::string::size_type l= title.size ();
	oss << "\x1B&f0k" << l << 'D' << title; // Set window title
	oss << "\x1B&f-1k" << l << 'D' << title; // Set icon title
	const std::string str= oss.str ();
	write (STDOUT_FILENO, str.c_str (), str.size () );
}

typedef void (* set_title_t) (const std::string &);

struct Cstring_less {
	bool operator () (const char * p, const char * q)
	{ return strcmp (p, q) < 0; }
};

typedef std::map <const char *, set_title_t, Cstring_less> maptitle_t;
maptitle_t maptitle;

bool initmaptitle ()
{
	maptitle ["xterm"]=     set_title_xterm;
	maptitle ["aixterm"]=   set_title_xterm;
	maptitle ["dtterm"]=    set_title_xterm;
	maptitle ["iris-ansi"]= set_title_iris_ansi;
	maptitle ["sun-cmd"]=   set_title_sun_cmd;
	maptitle ["hpterm"]=    set_title_hpterm;
	return true;
}

bool maptitle_inited= initmaptitle ();

}

#endif

void set_title (const std::string & title)
{
	TraceFunc tr ("set_title");

	if (graphics::ingraphicsmode () )
	{
		graphics::set_title (title);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

	SetConsoleTitle (title.c_str () );

	#elif defined BLASSIC_USE_TERMINFO

	if (! isatty (STDOUT_FILENO) )
		return;

	char * term= getenv ("TERM");
	if (term)
	{
		maptitle_t::iterator it= maptitle.find (term);
		if (it != maptitle.end () )
			(* it->second) (title);
		else
		{
			tr.message ("TERM not found");
		}
	}

	#endif
}

// Fin de cursor.cpp
