// runner.h
// Revision 13-ago-2003

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

enum TrigonometricMode { TrigonometricRad, TrigonometricDeg };

class GlobalRunner {
public:
	GlobalRunner (Program & prog);
	~GlobalRunner ();
	Program & getprogram () { return program; }

	// TRON stuff
	bool istron () const { return fTron; }
	void tron (bool fLine, BlChannel blc)
	{
		fTron= true;
		fTronLine= fLine;
		blcTron= blc;
	}
	void troff () { fTron= false; }

	// Channel stuff
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
	void resetfile0 ();
	void close_all ();
	void destroy_windows ();
	void closechannel (BlChannel channel);
	void windowswap (BlChannel ch1, BlChannel ch2);
	void tronline (BlLineNumber n);

	// DATA / READ stuff
	BlLineNumber & getdatanumline () { return datanumline; }
	BlChunk & getdatachunk () { return datachunk; }
	unsigned short & getdataelem () { return dataelem; }
	void setreadline (BlLineNumber bln)
	{
		datanumline= 0;
		datachunk= 0;
		dataelem= 0;
	}

	// ON ERROR GOTO stuff.
	void seterrorgoto (BlLineNumber line) { blnErrorGoto= line; }
	BlLineNumber geterrorgoto () { return blnErrorGoto; }

	// Control of depth of fn calls.
	size_t fn_level () { return fn_current_level; }
	void inc_fn_level ();
	void dec_fn_level ();

	// Trigonometric mode.
	TrigonometricMode trigonometric_mode () { return trigmode; }
	void trigonometric_default () { trigmode= TrigonometricRad; }
	void trigonometric_mode (TrigonometricMode tm)
	{ trigmode= tm; }

private:
	Program & program;

	bool fTron, fTronLine;
	BlChannel blcTron;

	ChanFile chanfile;

	BlLineNumber datanumline;
	BlChunk datachunk;
	unsigned short dataelem;

	BlLineNumber blnErrorGoto;

	size_t fn_current_level;

	TrigonometricMode trigmode;
};

class Element {
public:
	Element (ProgramPos ppos) :
		ppos (ppos)
	{ }
	void nextchunk () { ppos.nextchunk (); }
	void nextline () { ppos.nextline (); }
	ProgramPos getpos () const { return ppos; }
private:
	ProgramPos ppos;
};

class ForElement : public Element {
public:
	ForElement (const std::string & nvar, ProgramPos pos) :
		Element (pos),
		varname (nvar)
	{ }
	virtual ~ForElement () { }
	virtual bool next ()= 0;
	bool isvar (const std::string & nvar)
	{ return varname == nvar; }
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
	#if 0
	bool next ()
	{
		* varaddr+= step;
		if (step >= 0)
			return * varaddr <= max;
		else
			return *varaddr >= max;
	}
	#endif
protected:
        BlNumber * varaddr, max, step;
};

class ForElementNumberInc : public ForElementNumber {
public:
	ForElementNumberInc (const std::string & var, ProgramPos pos,
			BlNumber initial, BlNumber max, BlNumber step) :
		ForElementNumber (var, pos, initial, max, step)
	{ }
	bool next ()
	{
		* varaddr+= step;
		return * varaddr <= max;
	}
};

class ForElementNumberDec : public ForElementNumber {
public:
	ForElementNumberDec (const std::string & var, ProgramPos pos,
			BlNumber initial, BlNumber max, BlNumber step) :
		ForElementNumber (var, pos, initial, max, step)
	{ }
	bool next ()
	{
		* varaddr+= step;
		return * varaddr >= max;
	}
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
	#if 0
	bool next ()
	{
		* varaddr+= step;
		if (step >= 0)
			return * varaddr <= max;
		else
			return * varaddr >= max;
	}
	#endif
protected:
	BlInteger * varaddr, max, step;
};

class ForElementIntegerInc : public ForElementInteger {
public:
	ForElementIntegerInc (const std::string & var, ProgramPos pos,
			BlInteger initial, BlInteger max, BlInteger step) :
		ForElementInteger (var, pos, initial, max, step)
	{ }
	bool next ()
	{
		* varaddr+= step;
		return * varaddr <= max;
	}
};

class ForElementIntegerDec : public ForElementInteger {
public:
	ForElementIntegerDec (const std::string & var, ProgramPos pos,
			BlInteger initial, BlInteger max, BlInteger step) :
		ForElementInteger (var, pos, initial, max, step)
	{ }
	bool next ()
	{
		* varaddr+= step;
		return * varaddr >= max;
	}
};

inline ForElementNumber * newForElementNumber (const std::string & var,
	ProgramPos pos, BlNumber initial, BlNumber max, BlNumber step)
{
	if (step >= 0.0)
		return new ForElementNumberInc (var, pos, initial, max, step);
	else
		return new ForElementNumberDec (var, pos, initial, max, step);
}

inline ForElementInteger * newForElementInteger (const std::string & var,
	ProgramPos pos, BlInteger initial, BlInteger max, BlInteger step)
{
	if (step >= 0)
		return new ForElementIntegerInc (var, pos, initial, max, step);
	else
		return new ForElementIntegerDec (var, pos, initial, max, step);
}

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
	//Runner (Program & prog);
	Runner (GlobalRunner & gr);
	Runner (const Runner & runner);
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
	BlError geterror () const { return berrLast; }
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
	void goto_to (BlLineNumber line)
	{
		posgoto= line;
		status= ProgramGoto;
	}
	void goto_to (ProgramPos pos)
	{
		posgoto= pos;
		status= ProgramGoto;
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

	#if 0	
	void tron (bool fLine, BlChannel blc)
	{
		fTron= true;
		fTronLine= fLine;
		blcTron= blc;
	}
	void troff () { fTron= false; }
	#else
	void tron (bool fLine, BlChannel blc)
	{ globalrunner.tron (fLine, blc); }
	void troff () { globalrunner.troff (); }
	#endif

	void jump_break ()
	{
		if (! posbreak)
			throw ErrNoContinue;
		posgoto= posbreak;
		posbreak= 0;
		status= ProgramJump;
	}
	void set_break (ProgramPos pos) { posbreak= pos; }

	BlLineNumber & getdatanumline ()
	{ return globalrunner.getdatanumline (); }
	BlChunk & getdatachunk ()
	{ return globalrunner.getdatachunk (); }
	unsigned short & getdataelem ()
	{ return globalrunner.getdataelem (); }

	void seterrorgoto (BlLineNumber line)
	{ globalrunner.seterrorgoto (line); }
	BlLineNumber geterrorgoto ()
	{ return globalrunner.geterrorgoto (); }

	//typedef std::map <BlChannel,BlFile *> ChanFile;

	#if 0
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
	#else
	void assign_channel_var
		(const std::string & var, const std::string & value,
			BlFile::Align align)
	{ globalrunner.assign_channel_var (var, value, align); }
	#endif
        //bool isfileopen (BlChannel channel) const
        //{ return chanfile.find (channel) != chanfile.end (); }
        bool isfileopen (BlChannel channel) const
        { return globalrunner.isfileopen (channel); }
	BlFile & getfile (BlChannel channel)
	{ return globalrunner.getfile (channel); }
        void setfile (BlChannel channel, BlFile * npfile)
        { globalrunner.setfile (channel, npfile); }
        void resetfile0 ()
        { globalrunner.resetfile0 (); }
        void close_all ()
        { globalrunner.close_all (); }
        void destroy_windows ()
        { globalrunner.destroy_windows (); }
        void closechannel (BlChannel channel)
        { globalrunner.closechannel (channel); }
        void windowswap (BlChannel ch1, BlChannel ch2)
        { globalrunner.windowswap (ch1, ch2); }

	void setreadline (BlLineNumber bln)
	{ globalrunner.setreadline (bln); }

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
	void inc_fn_level ()
	{ globalrunner.inc_fn_level (); }
	void dec_fn_level ()
	{ globalrunner.dec_fn_level (); }

	// Trigonometric mode.
	TrigonometricMode trigonometric_mode ()
	{ return globalrunner.trigonometric_mode (); }
	void trigonometric_default ()
	{ globalrunner.trigonometric_default (); }
	void trigonometric_mode (TrigonometricMode tm)
	{ globalrunner.trigonometric_mode (tm); }
private:
	GlobalRunner & globalrunner;
	Program & program;
	StatusProgram status;
	//bool fTron, fTronLine;
	//BlChannel blcTron;
	bool fInElse;
        bool fInWend;
	//ProgramPos posactual;
	ProgramPos posgoto;

	//BlLineNumber datanumline;
	//BlChunk datachunk;
	//unsigned short dataelem;

	CodeLine line;
	RunnerLine runnerline;
        //ChanFile chanfile;
	BlCode codprev;
        BlError berrLast;

        //BlLineNumber blnErrorGoto;

	BreakState breakstate;
	BlLineNumber breakgosubline;
	ProgramPos posbreak;

	BlLineNumber blnAuto, blnAutoInc;

	std::stack <ForElement *, std::vector <ForElement *> > forstack;
	GosubStack gosubstack;
	std::stack <RepeatElement> repeatstack;
        std::stack <WhileElement> whilestack;

        //void tronline (BlLineNumber n);
	void tronline (BlLineNumber n) { globalrunner.tronline (n); }
        bool checkstatus (CodeLine & line, const CodeLine & line0);
        bool processline (const std::string & line);
};

#endif

// Fin de runner.h
