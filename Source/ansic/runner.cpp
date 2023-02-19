// runner.cpp
// Revision 9-jun-2003

#include "runner.h"
#include "keyword.h"
#include "var.h"
#include "codeline.h"
#include "dim.h"
#include "cursor.h"
#include "graphics.h"
#include "sysvar.h"
#include "util.h"
#include "trace.h"
#include "edit.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include <typeinfo>

using std::for_each;

#include <cassert>
#define ASSERT assert

#if __BORLANDC__ >= 0x0560
#pragma warn -8091
#endif

//************************************************
//		Auxiliar
//************************************************

namespace {

const std::string strbreak ("**BREAK**");

void deletefile (GlobalRunner::ChanFile::value_type & chf)
{
        delete chf.second;
}

void deletefileifnotzero (GlobalRunner::ChanFile::value_type & chf)
{
	if (chf.first != BlChannel (0) )
		delete chf.second;
}

inline bool iscomp (BlCode code)
{
	return code == '=' || code == keyDISTINCT ||
		code == '<' || code == keyMINOREQUAL ||
		code == '>' || code == keyGREATEREQUAL;
}

} // namespace

//************************************************
//		GlobalRunner
//************************************************

GlobalRunner::GlobalRunner (Program & prog) :
	program (prog),
	fTron (false),
	datanumline (0),
	datachunk (0),
	dataelem (0),
	blnErrorGoto (0)
{
	chanfile [0]= new BlFileConsole (std::cin, std::cout);
}

GlobalRunner::~GlobalRunner ()
{
	std::for_each (chanfile.begin (), chanfile.end (), deletefile);
}

BlFile & GlobalRunner::getfile (BlChannel channel)
{
        ChanFile::iterator it= chanfile.find (channel);
        if (it == chanfile.end () )
                throw ErrFileNumber;
        return * it->second;
}

void GlobalRunner::setfile (BlChannel channel, BlFile * npfile)
{
        ChanFile::iterator it= chanfile.find (channel);
        if (it != chanfile.end () )
                delete it->second;
        chanfile [channel]= npfile;
}

void GlobalRunner::close_all ()
{
	#if 0
	std::for_each (chanfile.begin (), chanfile.end (), deletefile);
	chanfile.clear ();
	chanfile [0]= new BlFileConsole (std::cin, std::cout);
	#else
	BlFile * bfsave= chanfile [0];
	std::for_each (chanfile.begin (), chanfile.end (),
		deletefileifnotzero);
	chanfile.clear ();
	chanfile [0]= bfsave;
	#endif
}

void GlobalRunner::destroy_windows ()
{
	TraceFunc tr ("GlobalRunner::destroy_windows");

	std::vector <BlChannel> w;
	for (ChanFile::iterator it= chanfile.begin ();
		it != chanfile.end (); ++it)
	{
		BlFile * f= it->second;
		if (typeid (* f) == typeid (BlFileWindow) )
		{
			delete f;
			w.push_back (it->first);
		}
	}
	for (size_t i= 0, l= w.size (); i < l; ++i)
	{
		#ifndef NDEBUG
		{
			std::ostringstream oss;
			oss << "Destroyed window " << w [i];
			tr.message (oss.str () );
		}
		#endif
		size_t d= chanfile.erase (w [i] );
		ASSERT (d == 1);
	}
}

void GlobalRunner::closechannel (BlChannel channel)
{
        ChanFile::iterator it= chanfile.find (channel);
        if (it != chanfile.end () )
        {
                delete it->second;
                chanfile.erase (it);
        }
}

void GlobalRunner::windowswap (BlChannel ch1, BlChannel ch2)
{
	ChanFile::iterator it1= chanfile.find (ch1);
	ChanFile::iterator it2= chanfile.find (ch2);
	if (it2 == chanfile.end () || it1 == chanfile.end () )
		throw ErrFileNumber;
	if (typeid (* it1->second) != typeid (BlFileWindow) )
		throw ErrFileMode;
	if (typeid (* it2->second) != typeid (BlFileWindow) )
		throw ErrFileMode;
	std::swap (it1->second, it2->second);
}

void GlobalRunner::tronline (BlLineNumber n)
{
	BlFile & file= getfile (blcTron);
	if (fTronLine)
		program.list (n, n, file);
	else
		file << '[' << BlNumber (n) << ']';
	file.flush ();
}

//************************************************
//		LocalLevel
//************************************************

//class GosubElement::Internal {
//	friend class GosubElement; // Only to avoid a warning in gcc.
class LocalLevel::Internal {
	friend class LocalLevel; // Only to avoid a warning in gcc.
public:
	Internal () : nref (1) { }
	void addref () { ++nref; }
	void delref ()
	{
		if (--nref == 0)
			delete this;
	}
	void addlocal (const std::string & name);
	void freelocals ();
private:
	~Internal () { }
	size_t nref;
	std::map <std::string, BlResult> maploc;
};

//void GosubElement::Internal::addlocal (const std::string & name)
void LocalLevel::Internal::addlocal (const std::string & name)
{
	if (maploc.find (name) != maploc.end () )
		return;
	BlResult result;
	switch (typeofvar (name) )
	{
	case VarNumber:
		{
			BlNumber n= 0;
			std::swap (n, * addrvarnumber (name) );
			result= n;
		}
		break;
	case VarInteger:
		{
			BlInteger n= 0;
			std::swap (n, * addrvarinteger (name) );
			result= n;
		}
	case VarString:
		{
			std::string str;
			swap (str, * addrvarstring (name) );
			result= str;
		}
		break;
	default:
		std::cerr << "Invalid local variable\n";
		throw ErrBlassicInternal;
	}
	maploc [name]= result;
}

namespace {

void freelocal (const std::pair <std::string, BlResult> & p)
{
	switch (p.second.type () )
	{
	case VarNumber:
		assignvarnumber (p.first, p.second.number () );
		break;
	case VarInteger:
		assignvarinteger (p.first, p.second.integer () );
		break;
	case VarString:
		assignvarstring (p.first, p.second.str () );
		break;
	default:
		throw ErrBlassicInternal;
	}
}

}

//void GosubElement::Internal::freelocals ()
void LocalLevel::Internal::freelocals ()
{
	for_each (maploc.begin (), maploc.end (), freelocal);
}

#if 0
GosubElement::GosubElement (ProgramPos pos) :
	Element (pos),
	pi (new Internal)
{
}
#endif

LocalLevel::LocalLevel () :
	pi (new Internal)
{
}

//GosubElement::GosubElement (const GosubElement & ge) :
LocalLevel::LocalLevel (const LocalLevel & ll) :
	//Element (ge),
	pi (ll.pi)
{
	pi->addref ();
}

//GosubElement::~GosubElement ()
LocalLevel::~LocalLevel ()
{
	pi->delref ();
}

//GosubElement & GosubElement::operator = (const GosubElement & ge)
LocalLevel & LocalLevel::operator = (const LocalLevel & ll)
{
	ll.pi->addref ();
	pi->delref ();
	pi= ll.pi;
	return * this;
}

//void GosubElement::freelocals ()
void LocalLevel::freelocals ()
{
	pi->freelocals ();
}

//void GosubElement::addlocal (const std::string & name)
void LocalLevel::addlocal (const std::string & name)
{
	pi->addlocal (name);
}

//************************************************
//		class Runner
//************************************************

//Runner::Runner (Program & prog) :
Runner::Runner (GlobalRunner & gr) :
	globalrunner (gr),
	//program (prog),
	program (gr.getprogram () ),
	status (ProgramEnded),
	//fTron (false),
	fInElse (false),
	fInWend (false),
	runnerline (* this, line, program),
	//blnErrorGoto (0),
	breakstate (BreakStop),
	blnAuto (0)
{
	//setreadline (0);
        //chanfile [0]= new BlFileConsole (std::cin, std::cout);
}

Runner::Runner (const Runner & runner) :
	globalrunner (runner.globalrunner),
	program (runner.program),
	status (ProgramEnded),
	//fTron (runner.fTron),
	//fTronLine (runner.fTronLine),
	//blcTron (runner.blcTron),
	fInElse (false),
	fInWend (false),
	runnerline (* this, line, program),
	//blnErrorGoto (runner.blnErrorGoto),
	breakstate (runner.breakstate),
	blnAuto (0)
{
        //chanfile [0]= new BlFileConsole (std::cin, std::cout);
}

Runner::~Runner ()
{
        //std::for_each (chanfile.begin (), chanfile.end (), deletefile);
}

void Runner::clear ()
{
	close_all ();
	while (! forstack.empty () )
		for_pop ();
	gosubstack.erase ();
	while (! repeatstack.empty () )
		repeatstack.pop ();
	while (! whilestack.empty () )
		whilestack.pop ();
}

#if 0
BlFile & Runner::getfile (BlChannel channel)
{
        ChanFile::iterator it= chanfile.find (channel);
        if (it == chanfile.end () )
                throw ErrFileNumber;
        return * it->second;
}

void Runner::setfile (BlChannel channel, BlFile * npfile)
{
        ChanFile::iterator it= chanfile.find (channel);
        if (it != chanfile.end () )
                delete it->second;
        chanfile [channel]= npfile;
}

void Runner::close_all ()
{
	#if 0
	std::for_each (chanfile.begin (), chanfile.end (), deletefile);
	chanfile.clear ();
	chanfile [0]= new BlFileConsole (std::cin, std::cout);
	#else
	BlFile * bfsave= chanfile [0];
	std::for_each (chanfile.begin (), chanfile.end (),
		deletefileifnotzero);
	chanfile.clear ();
	chanfile [0]= bfsave;
	#endif
}

void Runner::destroy_windows ()
{
	TraceFunc tr ("Runner::destroy_windows");

	std::vector <BlChannel> w;
	for (ChanFile::iterator it= chanfile.begin ();
		it != chanfile.end (); ++it)
	{
		BlFile * f= it->second;
		if (typeid (* f) == typeid (BlFileWindow) )
		{
			delete f;
			w.push_back (it->first);
		}
	}
	for (size_t i= 0, l= w.size (); i < l; ++i)
	{
		#ifndef NDEBUG
		{
			std::ostringstream oss;
			oss << "Destroyed window " << w [i];
			tr.message (oss.str () );
		}
		#endif
		size_t d= chanfile.erase (w [i] );
		ASSERT (d == 1);
	}
}

void Runner::closechannel (BlChannel channel)
{
        ChanFile::iterator it= chanfile.find (channel);
        if (it != chanfile.end () )
        {
                delete it->second;
                chanfile.erase (it);
        }
}

void Runner::windowswap (BlChannel ch1, BlChannel ch2)
{
	ChanFile::iterator it1= chanfile.find (ch1);
	ChanFile::iterator it2= chanfile.find (ch2);
	if (it2 == chanfile.end () || it1 == chanfile.end () )
		throw ErrFileNumber;
	if (typeid (* it1->second) != typeid (BlFileWindow) )
		throw ErrFileMode;
	if (typeid (* it2->second) != typeid (BlFileWindow) )
		throw ErrFileMode;
	std::swap (it1->second, it2->second);
}
#endif

#if 0
void Runner::setreadline (BlLineNumber bln)
{
	datanumline= bln;
	datachunk= 0;
	dataelem= 0;
}
#endif

void Runner::goto_line (BlLineNumber dest)
{
	//posgoto= dest;
        //status= ProgramReadyToRun;
	goto_to (dest);
}

void Runner::gosub_line (BlLineNumber dest, ProgramPos posgosub)
{
        //run_to (dest);
	goto_to (dest);
	gosubstack.push (posgosub);
}


void Runner::getline (std::string & line)
{
        clean_input ();
        BlFile & file= getfile (0);
        file.getline (line);
}

void Runner::run ()
{
	// Una chapuza por ahora.
	std::string str ("RUN");
	CodeLine code;
	code.scan (str);
	runline (code);
}

#if 0
void Runner::tronline (BlLineNumber n)
{
	BlFile & file= getfile (blcTron);
	if (fTronLine)
		program.list (n, n, file);
	else
		file << '[' << BlNumber (n) << ']';
	file.flush ();
}
#endif

inline bool Runner::checkstatus (CodeLine & line, const CodeLine & line0)
{
	switch (status)
	{
	case ProgramReadyToRun:
		{
			BlLineNumber gline= posgoto.getnum ();
			if (gline == 0)
				line= program.getfirstline ();
                
			else
			{
				CodeLine aux;
				program.getline (posgoto, aux);
				if (aux.number () != gline)
					throw ErrLineNotExist;
				line= aux;
			}
			if (line.number () == 0)
				status= ProgramEnded;
			else
			{
				status= ProgramRunning;
				return true;
			}
		}
		break;
	case ProgramJump:
		{
			const BlLineNumber gotoline= posgoto.getnum ();
			if (gotoline != 0)
			{
				//if (actualline != gotoline)
				//if (posactual.getnum () != gotoline)
				//if (getposactual().getnum () != gotoline)
				if (runnerline.number () != gotoline)
				{
					//line= program.getline (gotoline);
					//program.getline (gotoline, line);
					program.getline (posgoto, line);
				}
				else
					line.gotochunk (posgoto.getchunk () );
			}
			else
			{
				if (line.number () != 0)
					line= line0;
				line.gotochunk (posgoto.getchunk () );
			}
		}
		status= ProgramRunning;
		//line.gotochunk (gotochunk);
		//line.gotochunk (posgoto.getchunk () );
		return true;
	case ProgramGoto:
		{
			const BlLineNumber gotoline= posgoto.getnum ();
			if (gotoline == 0)
				throw ErrLineNotExist;
			if (runnerline.number () != gotoline)
			{
				CodeLine aux;
				program.getline (posgoto, aux);
				BlLineNumber l= aux.number ();
				if (l != gotoline)
					throw ErrLineNotExist;
				line= aux;
			}
			else
				line.gotochunk (posgoto.getchunk () );
		}
		status= ProgramRunning;
		return true;
        case ProgramRunning:
                if (line.number () != 0)
                        //line= program.getnextline (line);
			program.getnextline (line);
                if (line.number () == 0)
                        status= ProgramEnded;
                else
                        return true;
        default:
                ;
        }
        return false;
}

void Runner::runline (CodeLine & codeline)
{
	line= codeline;
	CodeLine line0= codeline;
	//RunnerLine runnerline (* this, line, program);
	do
	{
		try
		{
			try
			{
				do {
					runnerline.setline (line);
					if (globalrunner.istron () &&
						line.number () != 0)
					{
						tronline (line.number () );
					}
					runnerline.execute ();
				} while (checkstatus (line, line0) );
			}
			catch (std::bad_alloc &)
			{
				throw BlError (ErrOutMemory, getposactual () );
			}
			catch (SocketError &)
			{
				throw BlError (ErrSocket, getposactual () );
			}
			catch (BlErrNo e)
			{
				throw BlError (e, getposactual () );
			}
		}
		catch (BlError & berr)
		{
			seterror (berr);
			BlLineNumber errorgoto= geterrorgoto ();
			if (errorgoto == 0)
				throw;
			CodeLine aux;
			program.getline (errorgoto, aux);
			if (aux.number () != errorgoto)
			{
				BlError newerr (berr, ErrLineNotExist);
				seterror (newerr);
				throw newerr;
			}
			jump_to (errorgoto);
		}
		catch (BlBreak &)
		{
			fInterrupted= false;
			ProgramPos actual= getposactual ();
			switch (getbreakstate () )
			{
			case BreakStop:
				{
					BlFile & bf= getfile (0);
					bf << strbreak;
					if (line.number () != 0)
						bf << " in " <<
						BlNumber (line.number () );
					bf << '\n';
				}
				set_break (actual);
				setstatus (ProgramStopped);
				break;
			case BreakCont:
				throw BlError (ErrBlassicInternal, actual);
			case BreakGosub:
				gosub_line (getbreakgosub (), actual);
				break;
			}
		}
	} while (checkstatus (line, line0) );
}

bool Runner::processline (const std::string & line)
{
	CodeLine code;
	code.scan (line);
	BlLineNumber nline= code.number ();
	if (nline == 0)
	{
		if (blnAuto != 0)
		{
			// This probably must be changed.
			if (code.empty () )
				program.deletelines (blnAuto, blnAuto);
			else
			{
				code.setnumber (blnAuto);
				program.insert (code);
			}
			if (blnAuto > BlMaxLineNumber - blnAutoInc)
			{
				blnAuto= 0;
				throw BlError (ErrLineExhausted, 0);
			}
			else
				blnAuto+= blnAutoInc;
		}
		else
		{
			if (code.empty () )
				return false;
			runline (code);
			return true;
		}
	}
	else
	{
		if (nline > BlMaxLineNumber)
			throw BlError (ErrLineExhausted, 0);
		if (code.empty () )
			program.deletelines (nline, nline);
		else
			program.insert (code);
		if (blnAuto != 0)
		{
			blnAuto= code.number ();
			if (blnAuto > BlMaxLineNumber - blnAutoInc)
			{
				blnAuto= 0;
				throw BlError (ErrLineExhausted, 0);
			}
			else
				blnAuto+= blnAutoInc;
		}
	}
	return false;
}

void Runner::interactive ()
{
	TraceFunc tr ("Runner::interactive");

        using std::cin;
	using std::cout;
	using std::flush;

	set_title ("blassic");
	{
		std::ostringstream oss;
		oss << "\nBlassic " <<
			version::Major << '.' << version::Minor << '.' <<
			version::Release << "\n"
			"(C) 2001-2002 Julian Albo\n"
			"\n";
		getfile (0) << oss.str ();
	}
	bool showprompt= true;
	for (;;)
	{
		std::string line;
		if (blnAuto)
		{
			//cout << std::setw (7) << blnAuto << ' '
			//	<< flush;
			if (! editline (getfile (0), program, blnAuto, line) )
				fInterrupted= true;
		}
		else
		{
			if (showprompt)
				getfile (0) << strPrompt;
			//cout << "] " << flush;
			#if 0
			cursorvisible ();
			getline (line);
			cursorinvisible ();
			#else
			if (! editline (getfile (0), line, 0) )
				fInterrupted= true;
			#endif
		}

		if (fInterrupted)
		{
			fInterrupted= false;
			getfile (0) << strbreak << '\n';
			cin.clear ();
			blnAuto= 0;
			continue;
		}
                if (! cin)
			break;

		#ifndef _Windows

		if (! line.empty () && line [line.size () - 1] == '\r')
			line= line.substr (0, line.size () - 1);

		#endif

		try
		{
			showprompt= processline (line);
		}
		catch (BlError & be)
		{
			getfile (0) << util::to_string (be);
			setstatus (ProgramStopped);
		}
	} // for
}

// Fin de runner.cpp
