// edit.cpp

//#include "cursor.h"
#include "graphics.h"
#include "error.h"
#include "trace.h"
#include "util.h"

#include "edit.h"

using util::to_string;

#include <sstream>

#if 0

#if defined __linux__ || defined __unix__

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

#endif

#include <cassert>
#define ASSERT assert

namespace {

void showstring (BlFile & bf, const std::string & str)
{
	for (size_t i= 0, l= str.size (); i < l; ++i)
	{
		char c= str [i];
		if (c < 32)
			bf << '\\' << char (c + 'A');
		else
			bf << c;
	}
	bf.flush ();
}

class Edit {
public:
	Edit (BlFile & bf, std::string & str, size_t npos, size_t inicol) :
		bf (bf),
		str (str),
		npos (npos),
		inicol (inicol),
		width (bf.getwidth () ),
		textwindow (bf.istextwindow () )
	{
		graphics::synchronize_suspend ();
	}
	~Edit ()
	{
		graphics::synchronize_restart ();
	}
	bool do_it ();
private:
	BlFile & bf;
	std::string & str;
	size_t npos;
	size_t inicol;
	size_t width;
	bool textwindow;

	//void getwidth ();
	void back ();
	void forward ();
	void deletechar ();
	void showrest ();
	void showinitial ();
};

#if 0
void Edit::getwidth ()
{
	if (graphics::ingraphicsmode () )
		width= graphics::getlinewidth ();
	else
	{
		#ifdef __WIN32__
                HANDLE h= GetStdHandle (STD_INPUT_HANDLE);
                CONSOLE_SCREEN_BUFFER_INFO info;
                if (GetConsoleScreenBufferInfo (h, & info) )
                        width= info.dwSize.X;
                else
                        width= 80;
		#else
		STRUCT_WINSIZE win;
		ioctl (0, IOCTL_WINSIZE, & win);
		width= WINSIZE_COLS (win);
		#endif
	}

}
#endif

void Edit::back ()
{
	if (textwindow)
	{
		--npos;
		bf.movecharback ();
		return;
	}
	if ( (npos-- + inicol) % width != 0)
		bf.movecharback ();
	else
	{
		bf.movecharup ();
		bf.movecharforward (width - 1);
	}
}

void Edit::forward ()
{
	++npos;
	if (textwindow)
	{
		bf.movecharforward ();
		return;
	}
	if ( (npos + inicol) % width != 0)
		bf.movecharforward ();
	else
	{
		if (npos == str.size () )
		{
			//bf.movecharforward ();
			bf << '\n';
			bf.flush ();
		}
		else
		{
			//bf << '\r';
			//bf.flush ();
			bf.movecharback (width - 1);
			bf.movechardown ();
		}
	}
}

void Edit::deletechar ()
{
	if (npos < str.size () )
	{
		str.erase (npos, 1);
		// Vil chapuza.
		str+= ' ';
		showrest ();
		str.erase (str.size () - 1);
	}
}

void Edit::showrest ()
{
	TraceFunc tr ("Edit::showrest");

	if (npos == str.size () )
		return;
	//bf << str.substr (npos);
	//bf.flush ();
	showstring (bf, str.substr (npos) );

	size_t l= str.size ();
	size_t nlines= (l + inicol) / width;
	size_t actualline= (npos + inicol) / width;
	size_t actualcol= (npos + inicol) % width;
	size_t lastcol= (l + inicol) % width;
	if (lastcol == 0 && nlines > 0)
	{
		if (! textwindow)
		{
			#ifndef __WIN32__
			--nlines;
			// The cursor position remains
			// in the last valid column.
			lastcol= width - 1;
			//lastcol= width;
			#endif
		}
	}
	bf.movecharup (nlines - actualline);
	if (actualcol < lastcol)
	{
		tr.message (std::string ("moving back " +
			to_string (lastcol - actualcol) ) );
		bf.movecharback (lastcol - actualcol);
	}
	else if (actualcol > lastcol)
	{
		tr.message ("moving forward");
		bf.movecharforward (actualcol - lastcol);
	}
	else if (actualcol == width - 1)
	{
		// Without this the cursor sometimes keep in the next line.
		bf.movecharback ();
		bf.movecharforward ();
	}
}

void Edit::showinitial ()
{
	//bf << '\r' << str;
	//bf.flush ();
	bf << '\r';
	bf.movecharforward (inicol);
	showstring (bf, str);

	size_t l= str.size () + inicol;
	size_t nlines= l / width;
	size_t actualline= (npos + inicol) / width;
	size_t actualcol= (npos + inicol) % width;
	size_t lastcol= l % width;
        #ifndef __WIN32__
	if (lastcol == 0 && nlines > 0)
	{
		--nlines;
		lastcol= width - 1;
	}
        #endif

	bf.movecharup (nlines - actualline);
	if (actualcol < lastcol)
		bf.movecharback (lastcol - actualcol);
	else
		bf.movecharforward (actualcol - lastcol);
	//bf << '\r';
	//bf.flush ();
	//movecharforward (npos);
}

bool Edit::do_it ()
{
	showinitial ();

	bool editing= true;
	bool retval= true;
	while (editing)
	{
		bf.showcursor ();
		std::string key= bf.getkey ();
		bf.hidecursor ();
		if (key.size () == 1)
		{
			char c= key [0];
			switch (c)
			{
			case '\r': case '\n':
				editing= false;
				break;
			case '\x1B':
				editing= false;
				retval= false;
				break;
			case '\t':
				{
					//size_t n= npos + 1;
					//n+= 7 - (n % 8);
					//n-= npos ;
					size_t n= 8 - npos % 8;
					str.insert (npos, n, ' ');
					showrest ();
					for (size_t i= 0; i < n; ++i)
						forward ();
				}
				break;
			case '\3': // Ctrl-C
				editing= false;
				retval= false;
				break;
			case '\5': // Ctrl-E, test debug
				//bf << str.substr (npos) << '\n';
				//bf.flush ();
				showstring (bf, str.substr (npos) );
				bf << '\n';
				bf.flush ();
				showinitial ();
				break;
			case '\x8':
			case '\x7F':
				if (npos > 0)
				{
					back ();
					deletechar ();
				}
				break;
			default:
				if (c >= ' ')
				{
					str.insert (npos, 1, c);
					showrest ();
					forward ();
				}
			}
		}
		else if (key == "RIGHT")
		{
			if (npos < str.size () )
			{
				forward ();
			}
		}
		else if (key == "LEFT")
		{
			if (npos > 0)
			{
				back ();
			}
		}
		else if (key == "DELETE")
		{
			if (npos < str.size () )
				deletechar ();
		}
		else if (key == "HOME")
		{
			while (npos > 0)
				back ();
		}
		else if (key == "END")
		{
			const size_t l= str.size ();
			while (npos < l)
				forward ();
		}
	}
	//hidecursor ();

	// After exit, cursor must be positioned after the line edited.
	bf << str.substr (npos) << '\n';
	bf.flush ();
	
	return retval;
}

} // namespace

bool editline (BlFile & bf, std::string & str, size_t npos, size_t inicol)
{
	TraceFunc tr ("editline");
	{
		std::ostringstream oss;
		oss << "Inicol: " << inicol;
		tr.message (oss.str () );
	}

	Edit edit (bf, str, npos, inicol);
	return edit.do_it ();
}

bool editline (BlFile & bf, Program & program, BlLineNumber bln,
	std::string & str)
{
	std::string buffer;
	{
		BlFileOutString bfos;
		program.list (bln, bln, bfos);
		buffer= bfos.str ();
		if (buffer.empty () )
		{
			bfos << bln << " \n";
			buffer= bfos.str ();
		}
	}
	buffer.erase (buffer.size () - 1);
	static const std::string number ("01234567890");
	size_t inipos= buffer.find_first_of (number);
	ASSERT (inipos != std::string::npos);
	inipos= buffer.find_first_not_of (number, inipos);
	ASSERT (inipos != std::string::npos);
	++inipos;

	bool r;
	if ( (r= editline (bf, buffer, inipos) ) == true)
		str= buffer;
	return r;
}

bool editline (BlFile & bf, Program & program, BlLineNumber bln)
{
	std::string buffer;

	bool r;
	if ( (r= editline (bf, program, bln, buffer) ) == true)
	{
		CodeLine code;
		code.scan (buffer);
		BlLineNumber nline= code.number ();
		if (nline == 0)
			throw ErrBlassicInternal;
		else
		{
			if (code.empty () )
				program.deletelines (nline, nline);
			else
				program.insert (code);
		}
	}
	return r;
}

// End of edit.cpp
