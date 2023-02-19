// blassic.cpp
// Revision 13-aug-2003

#include <assert.h>

#include <string>
#include <iostream>

#include "blassic.h"

#include "token.h"
#include "keyword.h"
#include "var.h"
//#include "codeline.h"
//#include "program.h"
#include "runner.h"
#include "cursor.h"
#include "file.h"
#include "graphics.h"
#include "sysvar.h"
#include "trace.h"
#include "util.h"

#include <fstream>
//#include <iomanip>
#include <sstream>
#include <signal.h>

#include "error.h"

#ifdef BLASSIC_USE_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined __BORLANDC__ && defined _Windows
        #include <condefs.h>
#else
        #define USEUNIT(a)
#endif

USEUNIT("codeline.cpp");
USEUNIT("error.cpp");
USEUNIT("keyword.cpp");
USEUNIT("program.cpp");
USEUNIT("runner.cpp");
USEUNIT("token.cpp");
USEUNIT("var.cpp");
USEUNIT("dim.cpp");
USEUNIT("file.cpp");
USEUNIT("cursor.cpp");
USEUNIT("graphics.cpp");
USEUNIT("sysvar.cpp");
USEUNIT("version.cpp");
USEUNIT("trace.cpp");
USEUNIT("socket.cpp");
USEUNIT("runnerline.cpp");
USEUNIT("function.cpp");
USEUNIT("key.cpp");
USEUNIT("charset.cpp");
USEUNIT("edit.cpp");
USEUNIT("directory.cpp");
USEUNIT("using.cpp");
//---------------------------------------------------------------------------
bool fInterrupted= false;

const std::string strPrompt ("Ok");

//************************************************
//	Local functions and classes
//************************************************

namespace {

// Workaround to problem in cygwin:
#ifndef SIGBREAK
const int SIGBREAK= 21;
#endif

void handle_sigint (int)
{
	fInterrupted= true;
        #ifdef BLASSIC_USE_WINDOWS
        signal (SIGINT, handle_sigint);
        signal (SIGBREAK, handle_sigint);
        #endif
}

void init_signal_handlers ()
{
	TraceFunc tr ("init_signal_handlers");
        #ifdef BLASSIC_USE_WINDOWS

        signal (SIGINT, handle_sigint);
        signal (SIGBREAK, handle_sigint);

        #else

	struct sigaction act;
	act.sa_handler= handle_sigint;
	act.sa_flags= 0;
	sigaction (SIGINT, & act, 0);

	signal (SIGUSR1, TraceFunc::show);

        #endif
}

class Initializer {
public:
        Initializer (const char * progname) :
        	detached_graphics (false),
        	detached_text (false)
	{
		TraceFunc tr ("Initializer::Initializer");

		initconsole ();
		graphics::initialize (progname);
	}
        ~Initializer ()
	{
		TraceFunc tr ("Initializer::~Initializer");

		if (! detached_graphics)
			graphics::uninitialize ();
		if (! detached_text)
			quitconsole ();
	}
	void detachgraphics () { detached_graphics= true; }
	void detachtext () { detached_text= true; }
private:
	bool detached_graphics;
	bool detached_text;
};

std::vector <std::string> args;

void setprogramargs (char * * argv, size_t n)
{
	TraceFunc tr ("setprogramargs");

	sysvar::set16 (sysvar::NumArgs, short (n) );
	args.clear ();
	std::copy (argv, argv + n, std::back_inserter (args) );
}

} // namespace

void setprogramargs (const std::vector <std::string> & nargs)
{
	TraceFunc tr ("setprogramargs");

	sysvar::set16 (sysvar::NumArgs, short (nargs.size () ) );
	args= nargs;
}

std::string getprogramarg (size_t n)
{
	//if (n >= num_args)
	if (n >= args.size () )
		return std::string ();
	//return std::string (args [n]);
	return args [n];
}

int blassic (int argc, char * * argv)
{
        using std::cin;
        using std::cout;
        using std::cerr;
        using std::endl;
        using std::flush;

	TraceFunc tr ("blassic");

        //cout << std::setprecision (16);
       	init_signal_handlers ();
	sysvar::init ();
	Initializer initializer (argv [0] );

	CodeLine code;
	Program program;
	GlobalRunner globalrunner (program);
	//Runner runner (program);
	Runner runner (globalrunner);

	int n= 1;
	while (n < argc)
	{
		if (strcmp (argv [n], "-e") == 0)
		{
			if (++n == argc)
				throw "Option e needs argument";
			std::string line;
			line= argv [n];
			code.scan (line);
			if (code.number () != 0)
			{
				program.insert (code);
				try
				{
					runner.run ();
				}
				catch (BlError & be)
				{
					cerr << be << endl;
				}
				return 0;
			}
			try
			{
				runner.runline (code);
			}
			catch (BlError & be)
			{
				cerr << be << endl;
			}
			return 0;
		}
		else if (strcmp (argv [n], "-a") == 0)
		{
			if (++n == argc)
				throw "Option a needs argument";
			std::istringstream iss (argv [n]);
			BlLineNumber ini;
			iss >> ini;
			char c= char (iss.get () );
			if (! iss.eof () )
			{
				if (c != ',')
					throw "Bad parameter";
				BlLineNumber inc;
				iss >> inc;
				if (! iss)
					throw "Bad parameter";
				iss >> c;
				if (! iss.eof () )
					throw "Bad parameter";
				sysvar::set32 (sysvar::AutoInc, inc);
			}
			sysvar::set32 (sysvar::AutoInit, ini);
			++n;
		}
		else if (strcmp (argv [n], "-d") == 0)
		{
			#ifdef BLASSIC_USE_WINDOWS

			FreeConsole ();

			#else

			switch (fork () )
			{
			case pid_t (-1):
				throw "Error en fork";
			case pid_t (0):
				//cerr << "child" << flush;
				for (int i= 0; i < 3; ++i)
					if (isatty (i) )
						close (i);
				initializer.detachtext ();
				break;
			default:
				//cerr << "parent" << flush;
				initializer.detachgraphics ();
				return 0;
			}

			#endif

			++n;
		}
		else if (strcmp (argv [n], "-m") == 0)
		{
			if (++n == argc)
				throw "Option m needs argument";
			int mode= atoi (argv [n] );
			std::string str (argv [n] );
			std::string::size_type x= str.find ('x');
			if (x != std::string::npos)
			{
				int mode2= atoi (str.c_str () + x + 1);
				graphics::setmode (mode, mode2, false);
			}
			else
				if (mode != 0)
					graphics::setmode (mode);
			++n;
			globalrunner.resetfile0 ();
		}
		else if (strcmp (argv [n], "-x") == 0)
		{
			if (++n == argc)
				throw "Option x needs argument";
			excludekeyword (argv [n] );
			++n;
		}
		else
		{
			try
			{
				short narg= short (argc - n - 1);
				setprogramargs (argv + n + 1, narg);
				program.load (argv [n] );
				set_title (argv [n] );
				graphics::set_default_title (argv [n] );
				runner.run ();
			}
			catch (BlError & be)
			{
				cerr << be << endl;
			}
			return 0;
		}
	}

	runner.interactive ();

	return 0;
}

//************************************************
//		main
//************************************************

int main (int argc, char * * argv)
{
	using std::cerr;
	using std::endl;

	TraceFunc tr ("main");
        int r;

	try {
		r= blassic (argc, argv);
		std::ostringstream oss;
		oss << "Returning " << r << " without exception.";
		tr.message (oss.str () );
	}
	catch (BlErrNo ben)
	{
		cerr << ErrStr (ben) << endl;
		tr.message (ErrStr (ben) );
		r= 127;
	}
	catch (BlError & be)
	{
		cerr << be << endl;
		tr.message (util::to_string (be) );
		r= 127;
	}
	catch (std::exception & e)
	{
		cerr << e.what () << endl;
		tr.message (e.what () );
		r= 127;
	}
	catch (Exit & e) {
		r= e.code ();
		tr.message (util::to_string (r) );
	}
	catch (const char * str) {
		cerr << str << endl;
		tr.message (str);
		r= 127;
	}
	catch (...) {
		cerr << "Unexpected error." << endl;
		tr.message ("Unexpected error.");
		r= 127;
        }
	return r;
}

// Fin de blassic.cpp
