// edit.cpp
// Revision 31-jul-2003

//#include "cursor.h"
#include "graphics.h"
#include "error.h"
#include "trace.h"
#include "util.h"
#include "sysvar.h"

#include "edit.h"

using util::to_string;

#include <sstream>
#include <deque>

#include <cassert>
#define ASSERT assert

namespace {

std::deque <std::string> history;

size_t getmaxhistory ()
{
	return sysvar::get16 (sysvar::MaxHistory);
}

void showstring (BlFile & bf, const std::string & str)
{
	for (size_t i= 0, l= str.size (); i < l; ++i)
	{
		unsigned char c= str [i];
		if (c < 32)
			bf << '\\' << char (c + 'A');
		else
			bf << static_cast <char> (c);
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
		textwindow (bf.istextwindow () ),
		histsize (history.size () ),
		histpos (histsize),
		intagmode (bf.istagactive () )
	{
		graphics::synchronize_suspend ();
		if (intagmode)
			bf.tagoff ();
	}
	~Edit ()
	{
		graphics::synchronize_restart ();
		if (intagmode)
			bf.tag ();
	}
	bool do_it ();
private:
	BlFile & bf;
	std::string & str;
	size_t npos;
	size_t inicol;
	size_t width;
	bool textwindow;
	size_t histsize;
	size_t histpos;
	bool intagmode;

	std::string savestr;

	//void getwidth ();
	void back ();
	void forward ();
	void deletechar ();
	void showrest ();
	void showinitial ();
	void up ();
	void down ();
};

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
			//bf << '\n';
			bf.endline ();
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
	TraceFunc tr ("Edit::showinitial");

	//bf << '\r' << str;
	//bf.flush ();

	//bf << '\r';
	//bf.movecharforward (inicol);

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

void Edit::up ()
{
	if (histpos > 0)
	{
		if (histpos == histsize)
			savestr= str;
		std::string::size_type oldsize= str.size ();
		--histpos;
		str= history [histpos];
		while (npos > 0)
			back ();
		std::string::size_type
			actualsize= str.size ();
		if (oldsize > actualsize)
		{
			showstring (bf, std::string (oldsize, ' ') );
			npos= oldsize;
			for (size_t i= 0; i < oldsize; ++i)
				back ();
		}
		showinitial ();
	}
}

void Edit::down ()
{
	if (histpos < histsize)
	{
		std::string::size_type oldsize= str.size ();
		++histpos;
		if (histpos < histsize)
			str= history [histpos];
		else
			str= savestr;
		while (npos > 0)
			back ();
		std::string::size_type
			actualsize= str.size ();
		if (oldsize > actualsize)
		{
			showstring (bf, std::string (oldsize, ' ') );
			npos= oldsize;
			for (size_t i= 0; i < oldsize; ++i)
				back ();
		}
		showinitial ();
	}
}

bool Edit::do_it ()
{
	TraceFunc tr ("Edit::do_it");

	showinitial ();

	bool editing= true;
	bool retval= true;
	while (editing)
	{
		bf.showcursor ();
		tr.message ("Waiting key");
		std::string key= bf.getkey ();
		tr.message ("Received key");
		bf.hidecursor ();
		if (key.size () == 1)
		{
			unsigned char c= key [0];
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
				//bf << '\n';
				bf.endline ();
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
		else if (key == "UP")
			up ();
		else if (key == "DOWN")
			down ();
	}
	//hidecursor ();

	// After exit, cursor must be positioned after the line edited.
	//bf << str.substr (npos) << '\n';
	bf << str.substr (npos);
	bf.endline ();
	bf.flush ();

	if (retval)
	{
		size_t maxhist= getmaxhistory ();
		if (maxhist == 0)
			maxhist= 1;
		if (histsize >= maxhist)
		{
			history.erase (history.begin (),
				history.begin () + histsize - maxhist + 1);
		}
		if (! str.empty () && (history.empty () ||
				history [history.size () - 1] != str) )
			history.push_back (str);
	}
	
	return retval;
}

} // namespace

bool editline (BlFile & bf, std::string & str, size_t npos, size_t inicol)
{
	TraceFunc tr ("editline - string");
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
	TraceFunc tr ("editline - line number->string");

	std::string buffer;
	{
		BlFileOutString bfos;
		program.list (bln, bln, bfos);
		buffer= bfos.str ();
		if (buffer.empty () )
		{
			//bfos << bln << " \n";
			bfos << bln << ' ';
			bfos.endline ();
			buffer= bfos.str ();
		}
	}
	buffer.erase (buffer.size () - 1);
	tr.message (std::string (1, '\'') + buffer + '\'');

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
	TraceFunc tr ("editline - line number");

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
