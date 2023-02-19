// runner.cpp

#include "runner.h"
#include "runnerline.h"
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

using std::for_each;

#include <cassert>
#define ASSERT assert

#if __BORLANDC__ >= 0x0560
#pragma warn -8091
#endif

inline bool iscomp (BlCode code)
{
	return code == '=' || code == keyDISTINCT ||
		code == '<' || code == keyMINOREQUAL ||
		code == '>' || code == keyGREATEREQUAL;
}

namespace {

void deletefile (Runner::ChanFile::value_type & chf)
{
        delete chf.second;
}

} // namespace

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

Runner::Runner (Program & prog) :
	program (prog),
	status (ProgramEnded),
	fTron (false),
	fInElse (false),
	fInWend (false),
	blnErrorGoto (0),
	breakstate (BreakStop),
	blnAuto (0)
{
	setreadline (0);
        chanfile [0]= new BlFileConsole (std::cin, std::cout);
}

Runner::~Runner ()
{
        std::for_each (chanfile.begin (), chanfile.end (), deletefile);
}

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
	std::for_each (chanfile.begin (), chanfile.end (), deletefile);
	chanfile.clear ();
	chanfile [0]= new BlFileConsole (std::cin, std::cout);
}

void Runner::closechannel (BlChannel channel)
{
        ChanFile::iterator it= chanfile.find (channel);
        if (it != chanfile.end () ) {
                delete it->second;
                chanfile.erase (it);
        }
}

void Runner::setreadline (BlLineNumber bln)
{
	datanumline= bln;
	datachunk= 0;
	dataelem= 0;
}

void Runner::goto_line (BlLineNumber dest)
{
	posgoto= dest;
        status= ProgramReadyToRun;
}

void Runner::gosub_line (BlLineNumber dest, ProgramPos posgosub)
{
        run_to (dest);
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

void Runner::tronline (BlLineNumber n)
{
	BlFile & file= getfile (blcTron);
	if (fTronLine)
		program.list (n, n, file);
	else
		file << '[' << BlNumber (n) << ']';
	file.flush ();
}

inline bool Runner::checkstatus (CodeLine & line, const CodeLine & line0)
{
        switch (status)
        {
        case ProgramReadyToRun:
                //if (gotoline == 0)
		if (posgoto.getnum () == 0)
                        line= program.getfirstline ();
                else
                        //line= program.getline (gotoline);
			line= program.getline (posgoto.getnum () );
                if (line.number () == 0)
                        status= ProgramEnded;
                else
                {
                        status= ProgramRunning;
                        //line.gotochunk (gotochunk);
			line.gotochunk (posgoto.getchunk () );
			return true;
                }
                break;
	case ProgramJump:
		{
			const BlLineNumber gotoline= posgoto.getnum ();
			if (gotoline != 0)
			{
				//if (actualline != gotoline)
				if (posactual.getnum () != gotoline)
					line= program.getline (gotoline);
			}
			else
				if (line.number () != 0)
					line= line0;
		}
		status= ProgramRunning;
		//line.gotochunk (gotochunk);
		line.gotochunk (posgoto.getchunk () );
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
	do
	{
		try
		{
			try
			{
				do {
					RunnerLine runnerline
						(* this, line, program);
			        	if (fTron && line.number () != 0)
						tronline (line.number () );
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
			if (geterrorgoto () == 0)
				throw;
			jump_to (geterrorgoto () );
		}
		catch (BlBreak &)
		{
			fInterrupted= false;
			ProgramPos actual= getposactual ();
			switch (getbreakstate () )
			{
			case BreakStop:
				std::cerr << "**BREAK**";
				if (line.number () != 0)
					std::cerr << " in " << line.number ();
				std::cerr << std::endl;
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
				throw BlError (ErrLineExhausted, blnAuto);
			}
			else
				blnAuto+= blnAutoInc;
		}
		else
		{
			runline (code);
			return true;
		}
	}
	else
	{
		if (code.empty () )
			program.deletelines (nline, nline);
		else
			program.insert (code);
		if (blnAuto != 0)
			blnAuto= code.number () + blnAutoInc;
	}
	return false;
}

void Runner::interactive ()
{
	TraceFunc tr ("Runner::interactive");

        using std::cin;
	using std::cout;
	using std::flush;

	cout << "\nBlassic " <<
		version::Major << '.' << version::Minor << '.' <<
		version::Release << "\n"
		"(C) 2001-2002 Julian Albo\n"
		"\n";
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
			getfile (0) << "**BREAK**\n";
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
			std::ostringstream oss;
			oss << be;
			getfile (0) << oss.str ();
		}
	} // for
}

// Fin de runner.cpp
