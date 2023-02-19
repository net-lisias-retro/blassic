// runner.h
// Revision 22-may-2003

#ifndef RUNNER_H_
#define RUNNER_H_

#include "program.h"
#include "error.h"
#include "file.h"
#include "result.h"
#include "runnerline.h"

#include <stack>
#include <vector>
#include <map>

class Element {
public:
	Element (ProgramPos ppos) :
		ppos (ppos)
	{ }
	ProgramPos getpos () { return ppos; }
private:
	ProgramPos ppos;
};

class ForElement : public Element {
public:
	ForElement (const std::string & nvar, ProgramPos pos) :
		Element (pos),
		varname (nvar)
	{ }
	virtual bool next ()= 0;
	bool isvar (const std::string & nvar)
	{ return varname == nvar; }
	virtual ~ForElement () { }
private:
	ForElement (const ForElement &); // Forbidden
	ForElement & operator = (const ForElement &); // Forbidden
	std::string varname;
};

class ForElementNumber : public ForElement {
public:
	ForElementNumber (const std::string & nvar,
			ProgramPos pos,
		        BlNumber initial, BlNumber nmax, BlNumber nstep) :
		ForElement (nvar, pos),
		max (nmax),
		step (nstep)
	{
		varaddr= addrvarnumber (nvar);
		* varaddr= initial;
	}
	bool next ()
	{
		* varaddr+= step;
		if (step >= 0)
			return * varaddr <= max;
		else
			return *varaddr >= max;
	}
private:
        BlNumber * varaddr, max, step;
};

class ForElementInteger : public ForElement {
public:
	ForElementInteger (const std::string & nvar,
			ProgramPos pos,
			BlInteger initial, BlInteger nmax, BlInteger nstep) :
		ForElement (nvar, pos),
		max (nmax),
		step (nstep)
	{
		varaddr= addrvarinteger (nvar);
		* varaddr= initial;
	}
	bool next ()
	{
		* varaddr+= step;
		if (step >= 0)
			return * varaddr <= max;
		else
			return * varaddr >= max;
	}
private:
	BlInteger * varaddr, max, step;
};

class RepeatElement : public Element {
public:
	RepeatElement (ProgramPos pos) :
		Element (pos)
	{ }
};

class WhileElement : public Element {
public:
	WhileElement (ProgramPos pos) :
		Element (pos)
	{ }
};

class LocalLevel {
public:
	LocalLevel ();
	LocalLevel (const LocalLevel & ll);
	~LocalLevel ();
	LocalLevel & operator= (const LocalLevel & ll);
	void addlocal (const std::string & name);
	void freelocals ();
private:
	class Internal;
	Internal * pi;
};

class GosubElement : public Element, public LocalLevel {
public:
	GosubElement (ProgramPos pos) :
		Element (pos)
	{ }
	GosubElement (LocalLevel & ll) :
		Element (0),
		LocalLevel (ll)
	{ }
};

class GosubStack {
public:
	void push (ProgramPos pos)
	{
		st.push (GosubElement (pos) );
	}
	void push (LocalLevel & ll)
	{
		st.push (GosubElement (ll) );
	}
	void pop (ProgramPos & ppos)
	{
		if (st.empty () )
			throw ErrReturnWithoutGosub;
		GosubElement go= st.top ();
		ppos= go.getpos ();
		go.freelocals ();
		st.pop ();
	}
	void addlocal (const std::string & name)
	{
		if (st.empty () )
			throw ErrMisplacedLocal;
		GosubElement go= st.top ();
		go.addlocal (name);
	}
	void erase ()
	{
		// Unnecessary restore local vars here, when is called
		// vars will be cleared after.
		while (! st.empty () )
			st.pop ();
	}
private:
	std::stack <GosubElement> st;
};

class Runner {
public:
	Runner (Program & prog);
        ~Runner ();
        void clear ();
        void getline (std::string & line);
	void runline (CodeLine & codeline);
	void run ();
	void interactive ();

	BlErrNo geterr () const { return berrLast.geterr (); }
	ProgramPos geterrpos () const { return berrLast.getpos (); }
	BlLineNumber geterrline () const
		{ return berrLast.getpos ().getnum (); }
	void clearerror () { berrLast= BlError (); }
	void seterror (const BlError & er) { berrLast= er; }

	//ProgramPos getposactual () const { return posactual; }
	ProgramPos getposactual () const { return runnerline.getposactual (); }
	//void setposactual (ProgramPos pos) { posactual= pos; }
	//void setchunkactual (BlChunk chunk) { posactual.setchunk (chunk); }

	void setstatus (StatusProgram stat) { status= stat; }
	void run_to (BlLineNumber line)
	{
		posgoto= line;
		status= ProgramReadyToRun;
	}
	void run_to (ProgramPos pos)
	{
		posgoto= pos;
		status= ProgramReadyToRun;
	}
	void jump_to (BlLineNumber line)
	{
		posgoto= line;
		status= ProgramJump;
	}
	void jump_to (ProgramPos pos)
	{
		posgoto= pos;
		status= ProgramJump;
	}

	void push_for (ForElement * pfe)
	{
		forstack.push (pfe);
	}
	//bool for_empty () const { return forstack.empty (); }
	ForElement & for_top ()
	{
		if (forstack.empty () )
			throw ErrNextWithoutFor;
		return * forstack.top ();
	}
	void for_pop () { delete forstack.top (); forstack.pop (); }

	void gosub_pop (ProgramPos & pos) { gosubstack.pop (pos); }
	void gosub_addlocal (const std::string & str)
		{ gosubstack.addlocal (str); }
	void gosub_push (LocalLevel & ll)
		{ gosubstack.push (ll); }

	bool repeat_empty () const { return repeatstack.empty (); }
	void repeat_pop () { repeatstack.pop (); }
	RepeatElement & repeat_top () { return repeatstack.top (); }
	void repeat_push (const RepeatElement & re) { repeatstack.push (re); }

	bool in_wend () { return fInWend; }
	void in_wend (bool f) { fInWend= f; }
	bool while_empty () { return whilestack.empty (); }
	void while_pop () { whilestack.pop (); }
	WhileElement & while_top () { return whilestack.top (); }
	void while_push (const WhileElement & we) { whilestack.push (we); }
	
	void tron (bool fLine, BlChannel blc)
	{
		fTron= true;
		fTronLine= fLine;
		blcTron= blc;
	}
	void troff () { fTron= false; }
	void jump_break ()
	{
		if (! posbreak)
			throw ErrNoContinue;
		posgoto= posbreak;
		posbreak= 0;
		status= ProgramJump;
	}
	void set_break (ProgramPos pos) { posbreak= pos; }

	BlLineNumber & getdatanumline () { return datanumline; }
	BlChunk & getdatachunk () { return datachunk; }
	unsigned short & getdataelem () { return dataelem; }

	void seterrorgoto (BlLineNumber line) { blnErrorGoto= line; }
	BlLineNumber geterrorgoto () { return blnErrorGoto; }

	typedef std::map <BlChannel,BlFile *> ChanFile;
	void assign_channel_var
		(const std::string & var, const std::string & value,
			BlFile::Align align)
	{
		for (ChanFile::iterator it= chanfile.begin ();
			it != chanfile.end ();
			++it)
		{
			it->second->assign (var, value, align);
		}
	}
        bool isfileopen (BlChannel channel) const
        	{ return chanfile.find (channel) != chanfile.end (); }
        BlFile & getfile (BlChannel channel);
        void setfile (BlChannel channel, BlFile * npfile);
        void close_all ();
        void destroy_windows ();
        void closechannel (BlChannel channel);

	void setreadline (BlLineNumber bln);
        void goto_line (BlLineNumber dest);
        void gosub_line (BlLineNumber dest, ProgramPos posgosub);
	enum BreakState { BreakStop, BreakCont, BreakGosub };
	void setbreakstate (BreakState newstate)
	{
		breakstate= newstate;
	}
	BreakState getbreakstate () { return breakstate; }
	void setbreakgosub (BlLineNumber bln)
	{
		breakstate= BreakGosub;
		breakgosubline= bln;
	}
	BlLineNumber getbreakgosub () { return breakgosubline; }
	void setauto (BlLineNumber line, BlLineNumber inc)
	{ blnAuto= line; blnAutoInc= inc; }
private:
	Program & program;
	StatusProgram status;
	bool fTron, fTronLine;
	BlChannel blcTron;
	bool fInElse;
        bool fInWend;
	//ProgramPos posactual;
	ProgramPos posgoto;

	BlLineNumber datanumline;
	BlChunk datachunk;
	unsigned short dataelem;
	CodeLine line;
	RunnerLine runnerline;
        ChanFile chanfile;
	BlCode codprev;
        BlError berrLast;
        BlLineNumber blnErrorGoto;

	BreakState breakstate;
	BlLineNumber breakgosubline;
	ProgramPos posbreak;

	BlLineNumber blnAuto, blnAutoInc;

	std::stack <ForElement *, std::vector <ForElement *> > forstack;
	GosubStack gosubstack;
	std::stack <RepeatElement> repeatstack;
        std::stack <WhileElement> whilestack;

        void tronline (BlLineNumber n);
        bool checkstatus (CodeLine & line, const CodeLine & line0);
        bool processline (const std::string & line);
};

#endif

// Fin de runner.h
