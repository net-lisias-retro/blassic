// runnerline.cpp
// Revision 26-may-2003

#include "runnerline.h"

#include "error.h"
#include "dynamic.h"
#include "runner.h"
#include "cursor.h"
#include "sysvar.h"
#include "graphics.h"
#include "function.h"
#include "edit.h"
#include "util.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <memory>
using std::auto_ptr;
#include <cmath>
#include <cstdio>
#include <cerrno>
#include <ctime>
#include <cctype>
using std::isalpha;

#include <iostream>
using std::cerr;
using std::endl;

#if defined __unix__ || defined __linux__ // Kylix defines only __linux__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#else
// Define unlink?
using std::unlink;
#include <dir.h>
#endif

#include <cassert>
#define ASSERT assert

namespace {

#if defined __unix__ || defined __linux__

const char * os_family= "unix";

#else

const char * os_family= "windows";

#endif

// We encapsulate the random generator in a class to have all code
// related to it in a place, and to be able to change it easily.

// random in some systems may not use the RAND_MAX value,
// then we use it only in linux.

class RandomGenerator {
public:
	BlNumber operator () ()
	{
		#if defined __linux__
		return BlNumber (random () ) / (RAND_MAX + 1.0);
	        #else
		return BlNumber (rand () ) / (RAND_MAX + 1.0);
		#endif
	}
	void seed (unsigned int value)
	{
		#if defined __linux__
		srandom (value);
		#else
		srand (value);
		#endif
	}
};

RandomGenerator randgen;

inline bool iscomp (BlCode code)
{
	return code == '=' || code == keyDISTINCT ||
		code == '<' || code == keyMINOREQUAL ||
		code == '>' || code == keyGREATEREQUAL;
}

typedef std::map <std::string, Function> mapfunction_t;
mapfunction_t mapfunction;

} // namespace

// **********  Hyperbolic trigonometric arc functions  **********

#if ! (defined __unix__ || defined __linux__)

double asinh (double x)
{
	return log (x + sqrt (x * x + 1) );
}

double acosh (double x)
{
	errno= 0;
	double r= sqrt (x * x - 1);
	if (errno != 0)
		return 0;
	return log (x + r );
}

double atanh (double x)
{
	return log ( (1 + x) / (1 - x) ) / 2;
}

#endif

const RunnerLine::mapfunc_t RunnerLine::mapfunc= initmapfunc ();

// Avoid call mapfunc.end every time.
const RunnerLine::mapfunc_t::const_iterator
	RunnerLine::mapend= mapfunc.end ();

RunnerLine::mapfunc_t RunnerLine::initmapfunc ()
{
	mapfunc_t m;
	m [':']=             & RunnerLine::do_empty_sentence;
	m [keyENDLINE]=      & RunnerLine::do_endline;
	m [keyNUMBER]=       & RunnerLine::do_number;
        m [keyINTEGER]=      & RunnerLine::do_number;
	m [keyEND]=          & RunnerLine::do_end;
	m [keyLIST]=         & RunnerLine::do_list;
	m [keyREM]=          & RunnerLine::do_rem;
	m [keyLOAD]=         & RunnerLine::do_load;
	m [keySAVE]=         & RunnerLine::do_save;
	m [keyNEW]=          & RunnerLine::do_new;
	m [keyEXIT]=         & RunnerLine::do_exit;
	m [keyRUN]=          & RunnerLine::do_run;
	m [keyPRINT]=        & RunnerLine::do_print;
	m [keyFOR]=          & RunnerLine::do_for;
	m [keyNEXT]=         & RunnerLine::do_next;
	m [keyIF]=           & RunnerLine::do_if;
	m [keyTRON]=         & RunnerLine::do_tron;
	m [keyTROFF]=        & RunnerLine::do_troff;
	m [keyLET]=          & RunnerLine::do_let;
	m [keyIDENTIFIER]=   & RunnerLine::do_let;
	m [keyGOTO]=         & RunnerLine::do_goto;
	m [keySTOP]=         & RunnerLine::do_stop;
	m [keyCONT]=         & RunnerLine::do_cont;
	m [keyCLEAR]=        & RunnerLine::do_clear;
	m [keyGOSUB]=        & RunnerLine::do_gosub;
	m [keyRETURN]=       & RunnerLine::do_return;
	m [keyPOKE]=         & RunnerLine::do_poke;
	m [keyREAD]=         & RunnerLine::do_read;
	m [keyDATA]=         & RunnerLine::do_data;
	m [keyRESTORE]=      & RunnerLine::do_restore;
	m [keyINPUT]=        & RunnerLine::do_input;
	m [keyLINE]=         & RunnerLine::do_line;
	m [keyRANDOMIZE]=    & RunnerLine::do_randomize;
	m [keyAUTO]=         & RunnerLine::do_auto;
	m [keyDIM]=          & RunnerLine::do_dim;
	m [keySYSTEM]=       & RunnerLine::do_system;
	m [keyON]=           & RunnerLine::do_on;
	m [keyERROR]=        & RunnerLine::do_error;
	m [keyOPEN]=         & RunnerLine::do_open;
	m [keyCLOSE]=        & RunnerLine::do_close;
	m [keyLOCATE]=       & RunnerLine::do_locate;
	m [keyCLS]=          & RunnerLine::do_cls;
	m [keyWRITE]=        & RunnerLine::do_write;
	m [keyMODE]=         & RunnerLine::do_mode;
	m [keyMOVE]=         & RunnerLine::do_move;
	m [keyCOLOR]=        & RunnerLine::do_color;
	m [keyGET]=          & RunnerLine::do_get;
	m [keyLABEL]=        & RunnerLine::do_label;
	m [keyDELIMITER]=    & RunnerLine::do_delimiter;
	m [keyREPEAT]=       & RunnerLine::do_repeat;
	m [keyUNTIL]=        & RunnerLine::do_until;
	m [keyWHILE]=        & RunnerLine::do_while;
	m [keyWEND]=         & RunnerLine::do_wend;
	m [keyPLOT]=         & RunnerLine::do_plot;
        // Open and popen use same function.
        m [keyPOPEN]=        & RunnerLine::do_open;
        m [keyRESUME]=       & RunnerLine::do_resume;
	m [keyDELETE]=       & RunnerLine::do_delete;
	m [keyLOCAL]=        & RunnerLine::do_local;
	m [keyPUT]=          & RunnerLine::do_put;
        m [keyFIELD]=        & RunnerLine::do_field;
        // Lset and rset use same function.
        m [keyLSET]=         & RunnerLine::do_lset;
        m [keyRSET]=         & RunnerLine::do_lset;
        m [keySOCKET]=       & RunnerLine::do_socket;
        m [keyMID_S]=        & RunnerLine::do_mid_s;
        m [keyDRAW]=         & RunnerLine::do_draw;
        m [keyDEF]=          & RunnerLine::do_def;
        m [keyFN]=           & RunnerLine::do_fn;
        m [keyPROGRAMARG_S]= & RunnerLine::do_programarg_s;
        m [keyERASE]=        & RunnerLine::do_erase;
        m [keySWAP]=         & RunnerLine::do_swap;
        m [keySYMBOL]=       & RunnerLine::do_symbol;
        m [keyZONE]=         & RunnerLine::do_zone;
        m [keyPOP]=          & RunnerLine::do_pop;
	m [keyNAME]=         & RunnerLine::do_name;
	m [keyKILL]=         & RunnerLine::do_kill;
	m [keyFILES]=        & RunnerLine::do_files;
	m [keyPAPER]=        & RunnerLine::do_paper;
	m [keyPEN]=          & RunnerLine::do_pen;
	m [keySHELL]=        & RunnerLine::do_shell;
	m [keyMERGE]=        & RunnerLine::do_merge;
	m [keyCHDIR]=        & RunnerLine::do_chdir;
	m [keyMKDIR]=        & RunnerLine::do_mkdir;
	m [keyRMDIR]=        & RunnerLine::do_rmdir;
	m [keySYNCHRONIZE]=  & RunnerLine::do_synchronize;
	m [keyPAUSE]=        & RunnerLine::do_pause;
	m [keyCHAIN]=        & RunnerLine::do_chain;
        m [keyENVIRON]=      & RunnerLine::do_environ;
        m [keyEDIT]=         & RunnerLine::do_edit;
        m [keyDRAWR]=        & RunnerLine::do_drawr;
        m [keyPLOTR]=        & RunnerLine::do_plotr;
        m [keyMOVER]=        & RunnerLine::do_mover;
        m [keyPOKE16]=       & RunnerLine::do_poke16;
        m [keyPOKE32]=       & RunnerLine::do_poke32;
        m [keyRENUM]=        & RunnerLine::do_renum;
        m [keyCIRCLE]=       & RunnerLine::do_circle;
        m [keyMASK]=         & RunnerLine::do_mask;
        m [keyWINDOW]=       & RunnerLine::do_window;
        m [keyGRAPHICS]=     & RunnerLine::do_graphics;
	m [keyBEEP]=         & RunnerLine::do_beep;
	// DEFINT, DEFSTR, DEFREAL, DEFSNG and DEFDBL use same function.
	m [keyDEFINT]=       & RunnerLine::do_defint;
	m [keyDEFSTR]=       & RunnerLine::do_defint;
	m [keyDEFREAL]=      & RunnerLine::do_defint;
	m [keyDEFSNG]=       & RunnerLine::do_defint;
	m [keyDEFDBL]=       & RunnerLine::do_defint;
	m [keyINK]=          & RunnerLine::do_ink;
	return m;
}

#if 0 // Use macros instead to avoid strange errors on hp-ux.
inline void RunnerLine::requiretoken (BlCode code) const throw (BlErrNo)
{
        if (token.code != code)
                throw ErrSyntax;
}

inline void RunnerLine::expecttoken (BlCode code) throw (BlErrNo)
{
        gettoken ();
        requiretoken (code);
}
#else
#define requiretoken(c) if (token.code == c) ; else throw ErrSyntax
#define expecttoken(c) do { \
                gettoken (); \
                if (token.code != c) throw ErrSyntax; \
                } while (0)
#endif

void RunnerLine::getnextchunk ()
{
	while (! endsentence () )
		gettoken ();
	if (token.code != keyENDLINE)
		gettoken ();
}

inline BlFile & RunnerLine::getfile (BlChannel channel)
{
	return runner.getfile (channel);
}

inline BlNumber RunnerLine::evalnum ()
{
	BlResult result;
	eval (result);
	return result.number ();
}

inline BlNumber RunnerLine::expectnum ()
{
	gettoken ();
	return evalnum ();
}

inline BlInteger RunnerLine::evalinteger ()
{
	BlResult result;
	eval (result);
	return result.integer ();
}

inline BlInteger RunnerLine::expectinteger ()
{
	gettoken ();
	return evalinteger ();
}

inline BlChannel RunnerLine::evalchannel ()
{
	BlResult result;
	eval (result);
	return util::checked_cast <BlChannel> (result.integer (), ErrMismatch);
}

inline BlChannel RunnerLine::expectchannel ()
{
	gettoken ();
	return evalchannel ();
}

inline std::string RunnerLine::evalstring ()
{
	BlResult result;
	eval (result);
	return result.str ();
}

inline std::string RunnerLine::expectstring ()
{
	gettoken ();
	return evalstring ();
}

inline VarPointer RunnerLine::evalvalpointer ()
{
	requiretoken (keyIDENTIFIER);
	std::string varname= token.str;
	gettoken ();
	Dimension d;
	bool isarray= false;
	if (token.code == '(')
	{
		d= expectdims ();
		isarray= true;
	}
	VarPointer vp;
	vp.type= typeofvar (varname);
	switch (vp.type)
	{
	case VarNumber:
		vp.pnumber= isarray ?
			addrdimnumber (varname, d) :
			addrvarnumber (varname);
		break;
	case VarInteger:
		vp.pinteger= isarray ?
			addrdiminteger (varname, d) :
			addrvarinteger (varname);
		break;
	case VarString:
		vp.pstring= isarray ?
			addrdimstring (varname, d) :
			addrvarstring (varname);
		break;
	default:
		throw ErrBlassicInternal;
	}
	return vp;
}

inline VarPointer RunnerLine::eval_let ()
{
	VarPointer vp= evalvalpointer ();
	requiretoken ('=');
	BlResult result;
	expect (result);
	switch (vp.type)
	{
	case VarNumber:
		* vp.pnumber= result.number ();
		break;
	case VarInteger:
		* vp.pinteger= result.integer ();
		break;
	case VarString:
		* vp.pstring= result.str ();
		break;
	default:
		throw ErrBlassicInternal;
	}
	return vp;
}

void RunnerLine::parenarg (BlResult & result)
{
	expect (result);
	requiretoken (')');
	gettoken ();
}

void RunnerLine::getparenarg (BlResult & result)
{
	expecttoken ('(');
	expect (result);
	requiretoken (')');
	gettoken ();
}

void RunnerLine::getparenarg (BlResult & result, BlResult & result2)
{
	expecttoken ('(');
	expect (result);
	requiretoken (',');
	expect (result2);
	requiretoken (')');
	gettoken ();
}

namespace {

inline double callnumericfunc (double (* f) (double), double n)
{
	errno= 0;
	n= f (n);
	switch (errno)
	{
	case 0:
		#if defined __unix__ || defined __linux__
		// Some errors not establish errno.
		if (! finite (n) )
			throw ErrDomain;
		#endif
		break;
	case EDOM:
		throw ErrDomain;
	case ERANGE:
		throw ErrRange;
	default:
		std::cerr << "Math error, errno= " << errno << std::endl;
		throw ErrBlassicInternal;
	}
	return n;
}

inline double callnumericfunc (double (* f) (double, double),
	double n, double n2)
{
	errno= 0;
	n= f (n, n2);
	switch (errno)
	{
	case 0:
		#if defined __unix__ || defined __linux__
		// Some errors not establish errno.
		if (! finite (n) )
			throw ErrDomain;
		#endif
		break;
	case EDOM:
		throw ErrDomain;
	case ERANGE:
		throw ErrRange;
	default:
		std::cerr << "Math error, errno= " << errno << std::endl;
		throw ErrBlassicInternal;
	}
	return n;
}

}

void RunnerLine::valnumericfunc (double (* f) (double), BlResult & result)
{
	getparenarg (result);
	BlNumber n= result.number ();
	result= callnumericfunc (f, n);
}

void RunnerLine::valnumericfunc2 (double (* f) (double, double),
	BlResult & result)
{
	BlResult result2;
	getparenarg (result, result2);
	BlNumber n= result.number ();
	BlNumber n2= result2.number ();
	result= callnumericfunc (f, n, n2);
}

namespace { // Auxiliary math functions

double auxFIX (double n)
{
	double r;
	std::modf (n, & r);
	return r;
}

#ifdef __WIN32__

double auxCINT (double n)
{
        // Provisional.
        double r;
        n= std::modf (n, & r);
        if (n >= 0.5) r+= 1.0;
        else if (n <= -0.5) r-= 1.0;
        return r;
}

#endif

} // namespace

void RunnerLine::valasc (BlResult & result)
{
	getparenarg (result);
	const std::string & str= result.str ();
	if (str.empty () ) result= 0;
	else result= BlInteger ( (unsigned char) str [0] );
}

void RunnerLine::vallen (BlResult & result)
{
	getparenarg (result);
	result= result.str ().size ();
}

void RunnerLine::valpeek (BlResult & result)
{
	getparenarg (result);
	BlChar * addr= (BlChar *) size_t (result.number () );
	//result= BlNumber (size_t (* addr) );
	result= BlInteger (size_t (* addr) );
}

void RunnerLine::valpeek16 (BlResult & result)
{
	getparenarg (result);
	BlChar * addr= (BlChar *) size_t (result.number () );
	result= peek16 (addr);
}

void RunnerLine::valpeek32 (BlResult & result)
{
	getparenarg (result);
	BlChar * addr= (BlChar *) size_t (result.number () );
	result= BlNumber (static_cast <unsigned long> (peek32 (addr) ) );
}

void RunnerLine::valprogramptr (BlResult & result)
{
	gettoken ();
	result= BlNumber (size_t (program.programptr () ) );
}

void RunnerLine::valrnd (BlResult & result)
{
	BlNumber n;
	gettoken ();
	if (token.code == '(')
	{
		parenarg (result);
		n= result.number ();
	}
	else n= 1;

	static BlNumber previous= 0;

	if (n == 0)
	{
		result= previous;
		return;
	}

	if (n < 0)
		srand (time (0) );

	BlNumber r;

	#if 0
	#if defined __unix__ || defined __linux__
	r= BlNumber (random () ) / (RAND_MAX + 1.0);
        #else
        r= BlNumber (rand () ) / (RAND_MAX + 1.0);
        #endif
        #else
        r= randgen ();
        #endif

	result= r;
	previous= r;
}

namespace
{
	const bool instr_direct= false;
	const bool instr_reverse= true;
	const bool find_first= true;
	const bool find_last= false;
	const bool find_yes= true;
	const bool find_not= false;
}

void RunnerLine::valinstr (BlResult & result, bool reverse)
{
	expecttoken ('(');
	std::string str;
	std::string::size_type init= reverse ? std::string::npos : 0;
	
	expect (result);
	switch (result.type () )
	{
	case VarString:
		str= result.str ();
		break;
	case VarNumber:
		init= std::string::size_type (result.number () );
		if (init > 0)
			--init;
                requiretoken (',');
		str= expectstring ();
		break;
	case VarInteger:
		init= result.integer ();
		if (init > 0)
			--init;
		requiretoken (',');
		str= expectstring ();
		break;
	default:
		throw ErrBlassicInternal;
	}
	requiretoken (',');
	std::string tofind= expectstring ();
	requiretoken (')');
	gettoken ();
	std::string::size_type pos;
	if (tofind.empty () )
	{
		if (str.empty () )
			pos= 0;
		else
			if (init < str.size () )
				pos= init + 1;
			else
				pos= 0;
	}
	else
	{
		pos= reverse ?
			str.rfind (tofind, init) :
			str.find (tofind, init);
		if (pos == std::string::npos)
			pos= 0;
		else ++pos;
	}
	result= BlInteger (pos);
}

void RunnerLine::valfindfirstlast (BlResult & result, bool first, bool yesno)
{
	expecttoken ('(');
	std::string str;
	std::string::size_type init= first ? 0 : std::string::npos;

	expect (result);
	switch (result.type () )
	{
	case VarString:
		str= result.str ();
		break;
	case VarNumber:
		init= std::string::size_type (result.number () );
		if (init > 0)
			--init;
                requiretoken (',');
		str= expectstring ();
		break;
	case VarInteger:
		init= result.integer ();
		if (init > 0)
			--init;
		requiretoken (',');
		str= expectstring ();
		break;
	default:
		throw ErrBlassicInternal;
	}
	requiretoken (',');
	std::string tofind= expectstring ();
	requiretoken (')');
	gettoken ();
	std::string::size_type pos;
	if (tofind.empty () )
	{
		if (str.empty () )
			pos= 0;
		else
			if (init < str.size () )
				pos= init + 1;
			else
				pos= 0;
	}
	else
	{
		pos= first ?
			( yesno ? str.find_first_of (tofind, init) :
			str.find_first_not_of (tofind, init) )
			:
			(yesno ? str.find_last_of (tofind, init) :
			str.find_last_not_of (tofind, init) );
		if (pos == std::string::npos)
			pos= 0;
		else ++pos;
	}
	result= BlInteger (pos);
}

namespace {

class GuardHandle {
public:
        GuardHandle (DynamicHandle * pn) :
                phandle (pn)
        { }
        ~GuardHandle ()
        {
                if (* phandle)
                        dynamicclose (* phandle);
        }
private:
        DynamicHandle * phandle;
};

} // namespace

void RunnerLine::valusr (BlResult & result)
{
	expecttoken ('(');
	void * symaddr;
	DynamicHandle libhandle= 0;
        GuardHandle guard (& libhandle);

	expect (result);
	switch (result.type () )
	{
	case VarNumber:
		symaddr= reinterpret_cast <void *>
			(size_t (result.number () ) );
		break;
	case VarInteger:
		symaddr= reinterpret_cast <void  *> (result.integer () );
		break;
	case VarString:
		{
			std::string libname= result.str ();
                        requiretoken (',');
			std::string funcname= expectstring ();
                        libhandle= dynamicload (libname);
			if (! libhandle)
				throw ErrNoDynamicLibrary;
                        symaddr= dynamicaddr (libhandle, funcname);
                        #ifdef _Windows
                        if (! symaddr)
                        {
                                funcname= std::string (1, '_') + funcname;
                                symaddr= dynamicaddr (libhandle, funcname);
                        }
                        #endif
			if (! symaddr)
				throw ErrNoDynamicSymbol;
		}
		break;
	default:
		throw ErrBlassicInternal;
	}

        int nparams= 0;
        std::vector <int> vparam;
        while (token.code == ',')
        {
                //BlNumber newpar= expectnum ();
                //vparam.push_back (int (newpar) );
		expect (result);
		vparam.push_back (result.integer () );
                ++nparams;
        }
	requiretoken (')');
        gettoken ();
	util::auto_buffer <int> param;
	if (nparams)
	{
		param.alloc (nparams);
		std::copy (vparam.begin (), vparam.end (), param.begin () );
	}

	// reinterpret_cast produces a warning in gcc.
	//DynamicUsrFunc f= reinterpret_cast <DynamicUsrFunc> (symaddr);
	DynamicUsrFunc f= (DynamicUsrFunc) symaddr;
	result= static_cast <BlInteger> (f (nparams, param) );
}

void RunnerLine::valval (BlResult & result)
{
        getparenarg (result);
        std::string str= result.str ();
        switch (sysvar::get (sysvar::TypeOfVal) )
	{
	case 0:
		// VAL simple.
		{
			#if 0
			size_t i= 0, l= str.size ();
			while (i < l && str [i] == ' ')
				++i;
			#else
			std::string::size_type i=
				str.find_first_not_of (" \t");
			if (i > 0)
				if (i == std::string::npos)
					str.erase ();
				else
					str= str.substr (i);
			#endif
		}
		result= CodeLine::Token::number (str);
		break;
	case 1:
		// VAL with expression evaluation (Sinclair ZX)
		if (str.find_first_not_of (" \t")
			== std::string::npos)
		{
			result= 0;
			break;
		}
		str= std::string ("0 ") + str;
		{
			CodeLine code;
			code.scan (str);
			RunnerLine runline (runner, code, program);
			runline.expect (result);
			if (runline.token.code != keyENDLINE)
				throw ErrSyntax;
		}
		if (! result.is_numeric () )
			throw ErrMismatch;
		break;
	default:
		throw ErrNotImplemented;
	}
}

void RunnerLine::valeof (BlResult & result)
{
	getparenarg (result);
	BlChannel channel= BlChannel (result.number () );
	BlFile & file= getfile (channel);
	result= BlInteger (file.eof () ? -1 : 0);
}

void RunnerLine::valvarptr (BlResult & result)
{
	expecttoken ('(');
	expecttoken (keyIDENTIFIER);
	std::string varname (token.str);
	VarType type= typeofvar (varname);
	size_t addr= 0;
	gettoken ();
	switch (token.code)
	{
	case ')':
		// Simple
		switch (type)
		{
		case VarNumber:
			addr= reinterpret_cast <size_t>
				(addrvarnumber (varname) );
			break;
		case VarInteger:
			addr= reinterpret_cast <size_t>
				(addrvarinteger (varname) );
                        break;
		case VarString:
			addr= reinterpret_cast <size_t>
				(addrvarstring (varname) );
			break;
		default:
			throw ErrBlassicInternal;
		}
		result= 0;
		gettoken ();
		break;
	case '(':
		// Array
		{
		Dimension dims= expectdims ();
		result= 0;
		requiretoken (')');
		switch (type)
		{
		case VarNumber:
			addr= reinterpret_cast <size_t>
				(addrdimnumber (varname, dims) );
			break;
		case VarInteger:
			addr= reinterpret_cast <size_t>
				(addrdiminteger (varname, dims) );
			break;
		case VarString:
			addr= reinterpret_cast <size_t>
				(addrdimstring (varname, dims) );
			break;
		default:
			throw ErrBlassicInternal;
		}
		gettoken ();
		}
		break;
	default:
		throw ErrSyntax;
	}
	result= BlNumber (addr);
}

void RunnerLine::valsgn (BlResult & result)
{
	getparenarg (result);
	BlNumber d= result.number ();
	result= d < 0 ? -1 : d > 0 ? 1 : 0;
}

void RunnerLine::valcvi (BlResult & result)
{
        getparenarg (result);
        std::string str (result.str () );
        if (str.size () < 2)
                throw ErrFunctionCall;
	result= BlInteger (short ( (unsigned char) str [0] ) |
		short ( ( (unsigned char) str [1] ) << 8) );
}

void RunnerLine::valcvs (BlResult & result)
{
	#define SIZE_S 4
	COMPILE_ASSERT (sizeof (float) == 4);
	getparenarg (result);
	std::string str (result.str () );
	if (str.size () < SIZE_S)
		throw ErrFunctionCall;
	BlNumber bn= BlNumber
		(* reinterpret_cast <const float *> (str.data () ) );
	result= bn;
	#undef SIZE_S
}

void RunnerLine::valcvd (BlResult & result)
{
	#define SIZE_D 8
	COMPILE_ASSERT (sizeof (double) == 8);
	getparenarg (result);
	std::string str (result.str () );
	if (str.size () < SIZE_D)
		throw ErrFunctionCall;
	BlNumber bn= BlNumber (* reinterpret_cast <const double *> (str.data () ) );
	result= bn;
	#undef SIZE_D
}

void RunnerLine::valcvl (BlResult & result)
{
        getparenarg (result);
        std::string str (result.str () );
        if (str.size () < 4)
                throw ErrFunctionCall;
	result=
                long ( (unsigned char) str [0] ) |
                long ( ( (unsigned char) str [1] ) << 8) |
                long ( ( (unsigned char) str [2] ) << 16) |
                long ( ( (unsigned char) str [3] ) << 24);
}

void RunnerLine::valmin (BlResult & result)
{
        expecttoken ('(');
        BlNumber bnMin= expectnum ();
        while (token.code == ',')
                bnMin= std::min (bnMin, expectnum () );
        requiretoken (')');
        gettoken ();
        result= bnMin;
}

void RunnerLine::valmax (BlResult & result)
{
        expecttoken ('(');
        BlNumber bnMax= expectnum ();
        while (token.code == ',')
                bnMax= std::max (bnMax, expectnum () );
        requiretoken (')');
        gettoken ();
        result= bnMax;
}

void RunnerLine::valtest (BlResult & result, bool relative)
{
	expecttoken ('(');
	int x= expectinteger ();
	requiretoken (',');
	int y= expectinteger ();
	requiretoken (')');
	gettoken ();
	result= graphics::test (x, y, relative);
}

void RunnerLine::valmid_s (BlResult & result)
{
	expecttoken ('(');
	std::string str= expectstring ();
	requiretoken (',');
	BlNumber blfrom= expectnum ();
	size_t from= size_t (blfrom) - 1;
	size_t len;
	if (token.code == ',')
	{
		BlNumber bllen= expectnum ();
		len= size_t (bllen);
	}
	else
		len= std::string::npos;
	requiretoken (')');
	if (from >= str.size () )
		result= std::string ();
	else
		result= str.substr (from, len);
	gettoken ();
}

void RunnerLine::valleft_s (BlResult & result)
{
	expecttoken ('(');
	std::string str= expectstring ();
	requiretoken (',');
	BlNumber blfrom= expectnum ();
	requiretoken (')');
	size_t from= size_t (blfrom);
	result= str.substr (0, from);
	gettoken ();
}

void RunnerLine::valright_s (BlResult & result)
{
	expecttoken ('(');
	std::string str= expectstring ();
	requiretoken (',');
	BlNumber blfrom= expectnum ();
	requiretoken (')');
	size_t from= size_t (blfrom);
	size_t l= str.size ();
	if (from < l)
		result= str.substr (str.size () - from);
	else
		result= str;
	gettoken ();
}

void RunnerLine::valchr_s (BlResult & result)
{
	getparenarg (result);
	result= std::string (1, (unsigned char) result.number () );
}

void RunnerLine::valenviron_s (BlResult & result)
{
	getparenarg (result);
	char * str= getenv (result.str ().c_str () );
	if (str)
		result= std::string (str);
	else
		result= std::string ();
}

void RunnerLine::valstring_s (BlResult & result)
{
	expecttoken ('(');
	expect (result);
	size_t rep= result.integer ();
	requiretoken (',');
	expect (result);
	requiretoken (')');
	gettoken ();
	BlChar charrep= '\0';
	switch (result.type () )
	{
	case VarNumber:
		charrep= BlChar ( (unsigned int) result.number () );
		break;
	case VarInteger:
		charrep= BlChar (result.integer () );
		break;
	case VarString:
		{
			const std::string & aux= result.str ();
			if (aux.empty () )
				charrep= '\0';
			else
				charrep= aux [0];
		}
		break;
	default:
		throw ErrBlassicInternal;
	}
	result= std::string (rep, charrep);
}

void RunnerLine::valosfamily_s (BlResult & result)
{
	gettoken ();
	result= std::string (os_family);
}

void RunnerLine::valosname_s (BlResult & result)
{
	gettoken ();

	#ifdef _Windows

	result= "Windows";

	#elif defined __unix__

	struct utsname buf;
	if (uname (&buf) != 0)
		result= "unknown";
	else
		result= buf.sysname;

	#else

	result= "unknown";

	#endif
}

void RunnerLine::valhex_s (BlResult & result)
{
	expecttoken ('(');
	expect (result);
	BlInteger n= result.integer ();
	size_t w= 0;
	if (token.code == ',')
	{
		expect (result);
		w= result.integer ();
	}
	requiretoken (')');
	gettoken ();
	std::ostringstream oss;
	oss.setf (std::ios::uppercase);
	oss << std::hex <<
		std::setw (w) << std::setfill ('0') <<
		n;
	result= oss.str ();
}

void RunnerLine::valspace_s (BlResult & result)
{
        getparenarg (result);
        result= std::string (size_t (result.number () ), ' ');
}

void RunnerLine::valupper_s (BlResult & result)
{
        getparenarg (result);
        std::string & str= result.str ();
        std::transform (str.begin (), str.end (), str.begin (), toupper);
}

void RunnerLine::vallower_s (BlResult & result)
{
        getparenarg (result);
        std::string & str= result.str ();
        std::transform (str.begin (), str.end (), str.begin (), tolower);
}

void RunnerLine::valstr_s (BlResult & result)
{
        getparenarg (result);
        std::ostringstream oss;
        oss << std::setprecision (10) << result.number ();
        result= oss.str ();
}

void RunnerLine::valoct_s (BlResult & result)
{
	expecttoken ('(');
	BlNumber n= expectnum ();
	size_t w= 0;
	if (token.code == ',')
	{
		BlNumber blw= expectnum ();
		w= size_t (blw);
	}
	requiretoken (')');
	gettoken ();
	std::ostringstream oss;
	oss.setf (std::ios::uppercase);
	oss << std::oct <<
                std::setw (w) << std::setfill ('0') <<
		(unsigned long) n;
	result= oss.str ();
}

void RunnerLine::valbin_s (BlResult & result)
{
	expecttoken ('(');
	BlNumber bn= expectnum ();
	size_t w= 0;
	if (token.code == ',')
	{
		BlNumber blw= expectnum ();
		w= size_t (blw);
	}
	requiretoken (')');
	gettoken ();
        unsigned long n= (unsigned long) bn;
        std::string str;
        while (n)
        {
                str= ( (n & 1) ? '1' : '0') + str;
                n/= 2;
        }
        if (str.empty () )
                str= std::string (1, '0');
        if (w > 0 && str.size () < w)
                str= std::string (w - str.size (), '0') + str;
	result= str;
}

void RunnerLine::valinkey_s (BlResult & result)
{
        gettoken ();
        result= inkey ();
}

void RunnerLine::valprogramarg_s (BlResult & result)
{
	getparenarg (result);
	result= getprogramarg (size_t (result.number () - 1) );
}

void RunnerLine::valdate_s (BlResult & result)
{
        using std::setw;
        using std::setfill;

	gettoken ();
	std::time_t t= time (NULL);
	struct std::tm * ptm= std::localtime (& t);
	std::ostringstream oss;
	oss << setw (2) << setfill ('0') << (ptm->tm_mon + 1) << '-' <<
		setw (2) << setfill ('0') << ptm->tm_mday << '-' <<
		(ptm->tm_year + 1900);
	result= oss.str ();
}

void RunnerLine::valtime_s (BlResult & result)
{
        using std::setw;
        using std::setfill;

	gettoken ();
	std::time_t t= time (NULL);
	struct std::tm * ptm= std::localtime (& t);
	std::ostringstream oss;
	oss << setw (2) << setfill ('0') << ptm->tm_hour << ':' <<
		setw (2) << setfill ('0') << ptm->tm_min << ':' <<
		setw (2) << setfill ('0') << ptm->tm_sec;
	result= oss.str ();
}

void RunnerLine::valinput_s (BlResult & result)
{
	expecttoken ('(');
	BlNumber bn= expectnum ();
	BlChannel channel= 0;
	switch (token.code)
	{
	case ')':
		break;
	case ',':
		gettoken ();
		if (token.code == '#')
			channel= expectchannel ();
		requiretoken (')');
		break;
	default:
		throw ErrSyntax;
	}
	gettoken ();
	BlFile & in= getfile (channel);
	result= in.read (size_t (bn) );
}

void RunnerLine::valmki_s (BlResult & result)
{
        getparenarg (result);
        BlNumber bn= result.number ();
        unsigned short s= (unsigned short) short (bn);
        std::string str;
        str= char (s & 255);
        str+= char (s >> 8);
        result= str;
}

void RunnerLine::valmks_s (BlResult & result)
{
	getparenarg (result);
	float f= result.number ();
	std::string str (reinterpret_cast <char *> (& f), sizeof (float) );
	result= str;
}

void RunnerLine::valmkd_s (BlResult & result)
{
	getparenarg (result);
	double f= result.number ();
	std::string str (reinterpret_cast <char *> (& f), sizeof (double) );
	result= str;
}

void RunnerLine::valmkl_s (BlResult & result)
{
        getparenarg (result);
        BlNumber bn= result.number ();
        unsigned int s= (unsigned int) int (bn);
        std::string str;
        str= char (s & 255);
        str+= char ( (s >> 8) & 255);
        str+= char ( (s >> 16) & 255);
        str+= char ( (s >> 24) & 255);
        result= str;
}

void RunnerLine::valtrim (BlResult & result)
{
	using std::string;

	bool tleft= false, tright= false;
	switch (token.code)
	{
	case keyTRIM_S:
		tleft= true; tright= true;
		break;
	case keyLTRIM_S:
		tleft= true;
		break;
	case keyRTRIM_S:
		tright= true;
		break;
	default:
		cerr << "Llamada erronea a valtrim" << endl;
		throw ErrBlassicInternal;
	}
	getparenarg (result);
	string str= result.str ();
	if (tleft)
	{
		string::size_type
			inipos= str.find_first_not_of (' ');
		if (inipos > 0)
			if (inipos == string::npos)
				str= string ();
			else
				//str= str.substr (inipos);
				str.erase (0, inipos);
	}
	if (tright)
	{
		string::size_type
			endpos=  str.find_last_not_of (' ');
		if (endpos != string::npos)
			//str= str.substr (0, endpos + 1);
			str.erase (endpos + 1);
		else str= string ();
	}
	result= str;
}

void RunnerLine::valcopychr_s (BlResult & result)
{
	expecttoken ('(');
	expecttoken ('#');
	BlChannel c= expectchannel ();
	requiretoken (')');
	BlFile & window= getfile (c);
	result= window.copychr ();
	gettoken ();
}

void RunnerLine::valfn (BlResult & result)
{
        expecttoken (keyIDENTIFIER);
	std::string fname= token.str;
        Function f= Function::get (fname);
        gettoken ();
        // Abusamos de GosubElement para aprovechar la gestion
        // de variables locales.
        //GosubElement ge (0);
        // Ya no
	LocalLevel ll;
        const ParameterList & param= f.getparam ();
        size_t l= param.size ();
        if (l != 0)
        {
		requiretoken ('(');
                for (size_t i= 0; i < l; ++i)
                {
                        BlResult aux;
                        expect (result);
                        //const std::string & var= param [i];
                        std::string var= param [i];
                        ll.addlocal (var);
                        VarType type= typeofvar (var);
                        switch (type)
                        {
                        case VarNumber:
                                assignvarnumber (var, result.number () );
                                break;
			case VarInteger:
				assignvarinteger (var, result.integer () );
				break;
                        case VarString:
                                assignvarstring (var, result.str () );
                                break;
                        default:
                                throw ErrBlassicInternal;
                        }
                        if (i < l - 1)
				requiretoken (',');
                }
		requiretoken (')');
                gettoken ();
        }

	switch (f.getdeftype () )
	{
	case Function::DefSingle:
		{
			CodeLine & code= f.getcode ();
			code.gotochunk (0);
			RunnerLine fnrunline (runner, code, program);
			// Prevents lock on recursive calls.
			if (fInterrupted)
			{
				if (runner.getbreakstate () !=
						Runner::BreakCont)
					throw BlBreak ();
				else
					fInterrupted= false;
			}
			fnrunline.expect (result);
			ll.freelocals ();
		}
		break;
	case Function::DefMulti:
		{
			ll.addlocal (fname);
			Runner fnrun (program);
			// We reuse the break position
			// to jump to the fn definition.
			fnrun.set_break (f.getpos () );
			fnrun.gosub_push (ll);
			CodeLine jumpline;
			jumpline.scan ("CONT");
			fnrun.runline (jumpline);
			switch (typeofvar (fname) )
			{
			case VarNumber:
				result= evaluatevarnumber (fname);
				break;
			case VarInteger:
				result= evaluatevarinteger (fname);
				break;
			case VarString:
				result= evaluatevarstring (fname);
				break;
			default:
				throw ErrBlassicInternal;
			}
			ProgramPos notused;
			fnrun.gosub_pop (notused);
		}
		break;
	}
}

void RunnerLine::valsubindex (const std::string & varname, BlResult & result)
{
        Dimension dims= expectdims ();
        switch (typeofvar (varname) )
        {
        case VarNumber:
                result= valuedimnumber (varname, dims);
                break;
        case VarInteger:
                result= valuediminteger (varname, dims);
                break;
        case VarString:
                result= valuedimstring (varname, dims);
                break;
        default:
                throw ErrBlassicInternal;
        }
}

void RunnerLine::valbase (BlResult & result)
{
	switch (token.code)
	{
	case keyLET: // LET as operator
		{
			gettoken ();
			VarPointer vp= eval_let ();
			switch (vp.type)
			{
			case VarNumber:
				result= * vp.pnumber;
				break;
			case VarInteger:
				result= * vp.pinteger;
				break;
			case VarString:
				result= * vp.pstring;
				break;
			default:
				throw ErrBlassicInternal;
			}
		}
		break;
	case keyIDENTIFIER:
		{
			std::string varname (token.str);
			gettoken ();
			if (token.code == '(')
				valsubindex (varname, result);
			else
				switch (typeofvar (varname) )
				{
				case VarNumber:
					result= evaluatevarnumber (varname);
					break;
				case VarInteger:
					result= evaluatevarinteger (varname);
					break;
				case VarString:
					result= evaluatevarstring (varname);
					break;
				default:
					throw ErrBlassicInternal;
				}
		}
		break;
	case keySTRING:
		result= token.str;
		gettoken ();
		break;
	case keyNUMBER:
		result= token.number ();
		gettoken ();
		break;
        case keyINTEGER:
                result= token.integer ();
                gettoken ();
                break;
	case keyMID_S:
		valmid_s (result);
		break;
	case keyLEFT_S:
		valleft_s (result);
		break;
	case keyRIGHT_S:
		valright_s (result);
		break;
	case keyCHR_S:
		valchr_s (result);
		break;
	case keyENVIRON_S:
		valenviron_s (result);
		break;
	case keySTRING_S:
		valstring_s (result);
		break;
	case keyOSFAMILY_S:
		valosfamily_s (result);
		break;
	case keyHEX_S:
		valhex_s (result);
		break;
        case keySPACE_S:
                valspace_s (result);
                break;
        case keyUPPER_S:
                valupper_s (result);
                break;
        case keyLOWER_S:
                vallower_s (result);
                break;
        case keySTR_S:
                valstr_s (result);
                break;
        case keyOCT_S:
                valoct_s (result);
                break;
        case keyBIN_S:
                valbin_s (result);
                break;
        case keyINKEY_S:
                valinkey_s (result);
                break;
	case keyPROGRAMARG_S:
		valprogramarg_s (result);
		break;
	case keyDATE_S:
		valdate_s (result);
		break;
	case keyTIME_S:
		valtime_s (result);
		break;
	case keyINPUT_S:
		valinput_s (result);
		break;
        case keyMKI_S:
                valmki_s (result);
                break;
	case keyMKS_S:
		valmks_s (result);
		break;
	case keyMKD_S:
		valmkd_s (result);
		break;
        case keyMKL_S:
                valmkl_s (result);
                break;
	case keyOSNAME_S:
		valosname_s (result);
		break;
	case keyTRIM_S:
	case keyLTRIM_S:
	case keyRTRIM_S:
		valtrim (result);
		break;
	case keyFINDFIRST_S:
		getparenarg (result);
		result= directory.findfirst (result.str () );
		break;
	case keyFINDNEXT_S:
		result= directory.findnext ();
		gettoken ();
		break;
	case keyCOPYCHR_S:
		valcopychr_s (result);
		break;
	case keyASC:
		valasc (result);
		break;
	case keyLEN:
		vallen (result);
		break;
	case keyPEEK:
		valpeek (result);
		break;
	case keyPROGRAMPTR:
		valprogramptr (result);
		break;
	case keySYSVARPTR:
		gettoken ();
		result= sysvar::address ();
		break;
	case keyRND:
		valrnd (result);
		break;
	case keyINT:
		valnumericfunc (std::floor, result);
		break;
	case keySIN:
		valnumericfunc (std::sin, result);
		break;
	case keyCOS:
		valnumericfunc (std::cos, result);
		break;
	case keyPI:
		result= M_PI;
		gettoken ();
		break;
	case keyTAN:
		valnumericfunc (std::tan, result);
		break;
	case keySQR:
		valnumericfunc (std::sqrt, result);
		break;
	case keyASIN:
		valnumericfunc (std::asin, result);
		break;
	case keyACOS:
		valnumericfunc (std::acos, result);
		break;
	case keyINSTR:
		valinstr (result, instr_direct);
		break;
	case keyATAN:
		valnumericfunc (std::atan, result);
		break;
	case keyABS:
		valnumericfunc (std::fabs, result);
		break;
	case keyUSR:
		valusr (result);
		break;
        case keyVAL:
                valval (result);
                break;
	case keyEOF:
		valeof (result);
		break;
	case keyVARPTR:
		valvarptr (result);
		break;
	case keySGN:
		valsgn (result);
		break;
	case keyLOG:
		valnumericfunc (std::log, result);
		break;
	case keyLOG10:
		valnumericfunc (std::log10, result);
		break;
	case keyEXP:
		valnumericfunc (std::exp, result);
		break;
	case keyTIME:
		result= BlInteger (time (NULL) );
		gettoken ();
		break;
        case keyERR:
		result= runner.geterr ();
                gettoken ();
                break;
        case keyERL:
		result= BlNumber (runner.geterrline () );
                gettoken ();
                break;
        case keyCVI:
                valcvi (result);
                break;
	case keyCVS:
		valcvs (result);
		break;
	case keyCVD:
		valcvd (result);
		break;
        case keyCVL:
                valcvl (result);
                break;
        case keyMIN:
                valmin (result);
                break;
        case keyMAX:
                valmax (result);
                break;
	case keyCINT:
                #ifdef __WIN32__
                valnumericfunc (auxCINT, result);
                #else
		valnumericfunc (rint, result);
                #endif
		break;
	case keyFIX:
		valnumericfunc (auxFIX, result);
		break;
	case keyXMOUSE:
		result= graphics::xmouse ();
		gettoken ();
		break;
	case keyYMOUSE:
		result= graphics::ymouse ();
		gettoken ();
		break;
	case keyXPOS:
		result= graphics::xpos ();
		gettoken ();
		break;
	case keyYPOS:
		result= graphics::ypos ();
		gettoken ();
		break;
	case keyPEEK16:
		valpeek16 (result);
		break;
	case keyPEEK32:
		valpeek32 (result);
		break;
	case keyRINSTR:
		valinstr (result, instr_reverse);
		break;
	case keyFIND_FIRST_OF:
		valfindfirstlast (result, find_first, find_yes);
		break;
	case keyFIND_LAST_OF:
		valfindfirstlast (result, find_last, find_yes);
		break;
	case keyFIND_FIRST_NOT_OF:
		valfindfirstlast (result, find_first, find_not);
		break;
	case keyFIND_LAST_NOT_OF:
		valfindfirstlast (result, find_last, find_not);
		break;
	case keySINH:
		valnumericfunc (std::sinh, result);
		break;
	case keyCOSH:
		valnumericfunc (std::cosh, result);
		break;
	case keyTANH:
		valnumericfunc (std::tanh, result);
		break;
	case keyASINH:
		valnumericfunc (asinh, result);
		break;
	case keyACOSH:
		valnumericfunc (acosh, result);
		break;
	case keyATANH:
		valnumericfunc (atanh, result);
		break;
	case keyATAN2:
		valnumericfunc2 (atan2, result);
		break;
	case keyTEST:
		valtest (result, false);
		break;
	case keyTESTR:
		valtest (result, true);
		break;
        case keyFN:
                valfn (result);
                break;
	default:
		throw ErrSyntax;
	}
}

void RunnerLine::valparen (BlResult & result)
{
	if (token.code == '(')
	{
		gettoken ();
		eval (result);
		if (token.code != ')')
			throw ErrSyntax;
		gettoken ();
	}
	else
		valbase (result);
}

void RunnerLine::valexponent (BlResult & result)
{
	valparen (result);
	while (token.code == '^')
	{
		gettoken ();
		BlResult guard;
		//valparen (guard);
		valunary (guard);
		//result= std::pow (result.number (), guard.number () );
		result= callnumericfunc (std::pow,
			result.number (), guard.number () );
	}
}

void RunnerLine::valmod (BlResult & result)
{
	valexponent (result);
	while (token.code == keyMOD)
	{
		gettoken ();
		BlResult guard;
		valexponent (guard);
		#if 0
		if (! guard.number () )
			throw ErrDivZero;
		result= BlNumber ( (long) result.number ()  % (long) guard.number () );
		#else
		result%= guard;
		#endif
	}
}

void RunnerLine::valunary (BlResult & result)
{
	BlCode op= 0;
	if (token.code == '+' || token.code == '-' || token.code == keyNOT)
	{
		op= token.code;
		gettoken ();
		valunary (result);
	}
	else valmod (result);

	switch (op)
	{
	case 0:
		break;
	case '+':
		if (! result.is_numeric () )
			throw ErrMismatch;
		break;
	case '-':
		result= -result;
		break;
	case keyNOT:
		//result= BlNumber (~ long (result.number () ) );
                result= ~ result.integer ();
		break;
	default:
		throw ErrBlassicInternal;
	}
}

void RunnerLine::valmuldiv (BlResult & result)
{
	valunary (result);
	BlCode op= token.code;
	while (op == '*' || op == '/')
	{
		BlResult guard;
		gettoken ();
		valunary (guard);
		switch (op)
		{
		case '*':
			result*= guard;
			break;
		case '/':
			result/= guard;
			break;
		default:
			;
		}
		op= token.code;
	}
}

void RunnerLine::valplusmin (BlResult & result)
{
	valmuldiv (result);
	BlCode op= token.code;
	while (op == '+' || op == '-')
	{
		BlResult guard;
		gettoken ();
		valmuldiv (guard);
		switch (op)
		{
		case '+':
			result+= guard;
			break;
		case '-':
			result-= guard;
			break;
		}
		op= token.code;
	}
}

void RunnerLine::valcomp (BlResult & result)
{
	valplusmin (result);
	BlCode op= token.code;
	while (iscomp (op) )
	{
		BlResult guard;
		bool r;
		gettoken ();
		valplusmin (guard);
		switch (op)
		{
		case '=':
			r= (result == guard);
			break;
		case keyDISTINCT:
			r= (result != guard);
			break;
		case '<':
			r= (result < guard);
			break;
		case keyMINOREQUAL:
			r= (result <= guard);
			break;
		case '>':
			r= (result > guard);
			break;
		case keyGREATEREQUAL:
			r= (result >= guard);
			break;
		default:
			throw ErrBlassicInternal;
		}
		result= r ? -1 : 0;
		op= token.code;
	}
}

void RunnerLine::valorand (BlResult & result)
{
	valcomp (result);
	BlCode op= token.code;
	while (op == keyOR || op == keyAND || op == keyXOR)
	{
		BlResult guard;
		gettoken ();
		valcomp (guard);
		long n1= long (result.number () );
		long n2= long (guard.number () );
		switch (op)
		{
		case keyOR:
			result= BlNumber (n1 | n2);
			break;
		case keyAND:
			result= BlNumber (n1 & n2);
			break;
		case keyXOR:
			result= BlNumber (n1 ^ n2);
			break;
		default:
			throw ErrBlassicInternal;
		}
		op= token.code;
	}
}

void RunnerLine::eval (BlResult & result)
{
	valorand (result);
}

void RunnerLine::expect (BlResult & result)
{
	gettoken ();
	eval (result);
}

BlLineNumber RunnerLine::evallinenumber ()
{
	BlLineNumber bln;
	switch (token.code)
	{
	case keyNUMBER:
		bln= BlLineNumber (token.number () );
		break;
        case keyINTEGER:
                bln= BlLineNumber (token.integer () );
                break;
	case keyIDENTIFIER:
		bln= program.getlabel (token.str);
		if (bln == 0)
			throw ErrNoLabel;
		break;
	default:
		throw ErrSyntax;
	}
	gettoken ();
	return bln;
}

void RunnerLine::evallinerange (BlLineNumber & blnBeg, BlLineNumber & blnEnd)
{
	blnBeg= 0;
	blnEnd= BlMaxLineNumber;
	if (! endsentence () && token.code != ',')
	{
		if (token.code != '-')
		{
			blnBeg= evallinenumber ();
			if (token.code == '-')
			{
				gettoken ();
				if (! endsentence () && token.code != ',')
					blnEnd= evallinenumber ();
			}
			else
				blnEnd= blnBeg;
		}
		else
		{
			gettoken ();
			blnEnd= evallinenumber ();
		}
	}
}

Dimension RunnerLine::expectdims ()
{
	Dimension dims;
	requiretoken ('(');
	for (;;)
	{
		#if 0
		BlNumber n= expectnum ();
		dims.add (size_t (n) );
		#else
		BlResult result;
		expect (result);
		dims.add (result.integer () );
		#endif
		if (token.code != ',')
			break;
	}
	requiretoken (')');
	gettoken ();
	return dims;
}

void RunnerLine::errorifparam ()
{
	gettoken ();
	require_endsentence ();
}

void RunnerLine::gosub_line (BlLineNumber dest)
{
        //ProgramPos posgosub= runner.getposactual ();
	ProgramPos posgosub (getposactual () );
        posgosub.nextchunk ();
        #if 1
	if (token.code == keyENDLINE && posgosub.getnum () != 0)
	{
		#if 1
		posgosub.nextline ();
		#else
		BlLineNumber num= program.getnextnum (line);
		if (num != 0)
			posgosub= num;
		#endif
	}
	#endif
        runner.gosub_line (dest, posgosub);
}

void RunnerLine::getinkparams ()
{
	if (endsentence () )
		return;
	requiretoken (',');
	gettoken ();
	if (token.code != ',')
	{
		BlInteger ink= evalinteger ();
		graphics::setcolor (ink);
		if (token.code != ',')
		{
			require_endsentence ();
			return;
		}
	}
	gettoken ();
	BlInteger inkmode= evalinteger ();
	require_endsentence ();
	graphics::setdrawmode (inkmode);
}

void RunnerLine::getdrawargs (BlInteger & y)
{
	requiretoken (',');
	y= expectinteger ();
	getinkparams ();
}

void RunnerLine::getdrawargs (BlInteger & x, BlInteger & y)
{
	x= expectinteger ();
	getdrawargs (y);
}

void RunnerLine::make_clear ()
{
	runner.clear ();
	runner.seterrorgoto (0);
	clearvars ();
	Function::clear ();
	definevar (VarNumber, 'A', 'Z');
}

bool RunnerLine::do_empty_sentence ()
{
	return false;
}

bool RunnerLine::do_endline ()
{
	return true;
}

bool RunnerLine::do_number ()
{
	// goto omitido
	if (codprev != keyELSE && codprev != keyTHEN)
		throw ErrSyntax;
	BlLineNumber bln= evallinenumber ();
	require_endsentence ();
	runner.goto_line (bln);
	return true;
}

bool RunnerLine::do_end ()
{
	errorifparam ();
	runner.setstatus (ProgramEnded);
	return true;
}

bool RunnerLine::do_list ()
{
	gettoken ();
	BlChannel nfile= 0;
	if (token.code == '#')
        {
		nfile= expectchannel ();
		if (!endsentence () )
                {
                        requiretoken (',');
			gettoken ();
		}
	}

	BlLineNumber iniline, endline;
	evallinerange (iniline, endline);

	require_endsentence ();

	program.list (iniline, endline, getfile (nfile) );
	return false;
}

bool RunnerLine::do_rem ()
{
	return true;
}

bool RunnerLine::do_load ()
{
        using std::ifstream;
        using std::ios;

	std::string progname= expectstring ();
	if (endsentence () )
	{
		program.load (progname);
		runner.setstatus (ProgramEnded);
		return true;
	}
        requiretoken (',');
	gettoken ();
	if (token.code == keyIDENTIFIER &&
			typeofvar (token.str) == VarString)
	{
		std::string varname= token.str;
		gettoken ();
		require_endsentence ();

		ifstream is (progname.c_str (), ios::binary | ios::in);
		// Without explicit in read doesn't work on hp-ux,
		// I don't know why.

		if (! is.is_open () )
			throw ErrFileNotFound;
		is.seekg (0, std::ios::end);
		size_t size= is.tellg ();
		is.seekg (0, std::ios::beg);
		util::auto_buffer <char> aux (size);
		is.read (aux, size);
		std::string result (aux, size);
		assignvarstring (varname, result);
	}
	else
	{
		char * init;
		{
			BlNumber bn= evalnum ();
			init= reinterpret_cast <char *> (size_t (bn) );
		}
		size_t len= 0;
		if (token.code == ',')
		{
			BlNumber bn= expectnum ();
			len= size_t (bn);
		}
		require_endsentence ();

		ifstream is (progname.c_str (), std::ios::binary);
		if (! is.is_open () )
			throw ErrFileNotFound;
		if (len == 0)
		{
			is.seekg (0, std::ios::end);
			len= is.tellg ();
			is.seekg (0, std::ios::beg);
		}
		is.read (init, len);
	}
	return false;
}

bool RunnerLine::do_save ()
{
        using std::ofstream;

	std::string progname= expectstring ();
	if (endsentence () )
        	program.save (progname);
        else
        {
                requiretoken (',');
                expecttoken (keyIDENTIFIER);
		if (token.str.size () != 1)
			throw ErrSyntax;
		char c= char (toupper (token.str [0] ) );
		switch (c)
		{
		case 'A':
                        gettoken ();
			require_endsentence ();
			{
				BlFileRegular fout (progname, BlFile::Output);
				program.list (0, BlMaxLineNumber, fout);
			}
			break;
		case 'B':
			gettoken ();
			if (token.code != ',')
				throw ErrSyntax;
			gettoken ();
			{
			const char * init;
			size_t len;
			if (token.code == keyIDENTIFIER &&
				typeofvar (token.str) == VarString)
			{
				std::string * pstr= addrvarstring (token.str);
				init= pstr->data ();
				len= pstr->size ();
				gettoken ();
			}
			else
			{
				BlNumber bn= evalnum ();
				init= reinterpret_cast <const char *>
					(size_t (bn) );
				if (token.code != ',')
					throw ErrSyntax;
				bn= expectnum ();
				len= size_t (bn);
			}
			require_endsentence ();
			ofstream os (progname.c_str (), std::ios::binary);
			if (! os.is_open () )
				throw ErrFileNotFound;
			os.write (init, len);
			}
			break;
		default:
			throw ErrSyntax;
		}
        }
	return false;
}

bool RunnerLine::do_new ()
{
	errorifparam ();
	program.renew ();
        //clearvars ();
        //Function::clear ();
	make_clear ();
	runner.setreadline (0);
	return true;
}

bool RunnerLine::do_exit ()
{
	gettoken ();
	int r= 0;
	if (! endsentence () )
	{
		BlNumber n= evalnum ();
		if (! endsentence () )
			throw ErrSyntax;
		r= int (n);
	}
	throw Exit (r);
}

bool RunnerLine::do_run ()
{
	gettoken ();
	BlLineNumber dest= 0;
	if (token.code == keyNUMBER || token.code == keyINTEGER)
	{
		dest= evallinenumber ();
		require_endsentence ();
	}
	else if (! endsentence () )
	{
		std::string progname= evalstring ();
		require_endsentence ();
		program.renew ();
		program.load (progname);
	}
	//clearvars ();
	make_clear ();
	runner.setreadline (0);
	runner.run_to (dest);
	return true;
}

namespace {

class usingformat {
public:
	virtual ~usingformat () { }
	virtual bool isliteral () { return false; }
	virtual void putliteral (BlFile & out) { throw ErrBlassicInternal; }
	virtual void putnumeric (BlFile & out, BlNumber n)
	{ throw ErrBlassicInternal; }
	virtual void putstring (BlFile & out, const std::string & str)
	{ throw ErrBlassicInternal; }
};

class usingliteral : public usingformat {
public:
	usingliteral (const std::string & str) :
		str (str)
	{ }
	bool isliteral () { return true; }
	void putliteral (BlFile & out)
	{ out << str; }
private:
	std::string str;
};

class usingnumeric : public usingformat {
public:
	usingnumeric (size_t digit, size_t decimal) :
		digit (digit), decimal (decimal)
	{ }
	void putnumeric (BlFile & out, BlNumber n)
	{
		int numdig= 0;
                if (n != 0)
                        numdig= int (std::log10 (std::fabs (n) * 10) );
		if (numdig < 0) numdig= 0;
		size_t w= numdig;
		if (digit > w) w= digit;
		if (decimal > 0)
			w+= decimal + 1;
		std::ostringstream oss;
		if (decimal > 0)
			oss.setf (std::ios::showpoint);
		oss << std::setw (w) <<
			std::setprecision (decimal + numdig) <<
			n;
		out << oss.str ();
	}
private:
	size_t digit, decimal;
};

class usingstring : public usingformat {
public:
	usingstring (size_t n) :
		n (n)
	{ }
	void putstring (BlFile & out, const std::string & str)
	{
		if (n > 0)
			out << str.substr (0, n);
		else
			out << str;
	}
private:
	size_t n;
};

// Someone will call this "abuse of inheritance", but...

class vectorusing : public std::vector <usingformat *>
{
	static void delete_it (const usingformat * uf) { delete uf; }
public:
	vectorusing () { }
	~vectorusing ()
	{
		std::for_each (begin (), end (), delete_it);
	}
private:
	vectorusing (const vectorusing &); // Forbidden
	vectorusing & operator = (const vectorusing &); // Forbidden
};

bool ischarformat (char c)
{
	return c == '#' || c == '\\' || c == '&' || c == '!'; // De momento
}

void getusingformat (const std::string & str, vectorusing & f)
{
	const std::string::size_type l= str.size ();
	std::string::size_type i= 0;
	std::string lit;
	for (;;)
	{
		while (i < l && ! ischarformat (str [i]) )
		{
			lit+= str [i];
			++i;
		}
		if (! lit.empty () || l == 0)
		{
			auto_ptr <usingliteral> pul (new usingliteral (lit) );
			f.push_back (& * pul);
			pul.release ();
			lit.erase ();
		}
		if (i == l)
			break;
		switch (str [i] )
		{
		case '#':
			{
				size_t digit= 1, decimal= 0;
				while (++i < l && str [i] == '#')
					++digit;
				if (i < l && str [i] == '.')
					while (++i < l && str [i] == '#')
						++decimal;
				// We use auto_ptr to avoid memory leak
				// if push_back fail.
				auto_ptr <usingnumeric>
					pun (new usingnumeric
						(digit, decimal) );
				f.push_back (& * pun);
				pun.release ();
			}
			break;
		case '\\':
			{
				size_t n= 1;
				while (++i < l && str [i] == ' ')
					++n;
				if (i < l && str [i] == '\\')
					{ ++n; ++i; }
				auto_ptr <usingstring>
					pus (new usingstring (n) );
				f.push_back (& * pus);
				pus.release ();
			}
			break;
		case '&':
			{
				++i;
				auto_ptr <usingstring>
					pus (new usingstring (0) );
				f.push_back (& * pus);
				pus.release ();
			}
			break;
		case '!':
			{
				++i;
				auto_ptr <usingstring>
					pus (new usingstring (1) );
				f.push_back (& * pus);
				pus.release ();
			}
			break;
		default:
			throw ErrBlassicInternal;
		}
	}
}

} // namespace

void RunnerLine::print_using (BlFile & out)
{
	std::string format= expectstring ();
	vectorusing usingf;
	getusingformat (format, usingf);
	if (token.code == ',' || token.code == ';')
		gettoken ();
	const size_t l= usingf.size ();
	size_t ind= 0;
        usingformat * pf= usingf [ind];
	for (;;)
	{
		if (ind == 0 && pf->isliteral () )
                {
			pf->putliteral (out);
			ind= (ind + 1) % l;
			if (ind == 0)
				throw ErrFunctionCall;
			pf= usingf [ind];
		}
		BlResult result;
		eval (result);
		switch (result.type () )
		{
		case VarNumber:
			pf->putnumeric (out, result.number () );
			break;
                case VarInteger:
                        pf->putnumeric (out, result.number () );
                        break;
		case VarString:
			pf->putstring (out, result.str () );
			break;
		default:
			throw ErrBlassicInternal;
		}
		ind= (ind + 1) % l;
		pf= usingf [ind];
                if (ind != 0 && pf->isliteral () )
		{
			pf->putliteral (out);
			ind= (ind + 1) % l;
			// Seen unnecessary, and is erroneous.
			//if (ind == 0)
			//	throw ErrFunctionCall;
			pf= usingf [ind];
		}
		if (endsentence () )
		{
			out << '\n';
			break;
		}
		if (token.code == ';' || token.code == ',')
		{
			gettoken ();
			if (endsentence () )
				break;
		}
	}
}

bool RunnerLine::do_print ()
{
	gettoken ();
        BlChannel channel= 0;
        if (token.code == '#')
	{
                channel= expectchannel ();
		if (token.code == ',')
			gettoken ();
		else
			require_endsentence ();
        }
	BlFile & out= getfile (channel);
	if (token.code == '@')
	{
		BlResult result;
		expect (result);
		BlInteger pos= result.integer ();
		requiretoken (',');
		gettoken ();
		#if 0
		BlInteger row= (pos / 32) + 1;
		BlInteger col= (pos % 32) + 1;
		if (graphics::ingraphicsmode () )
			graphics::locate (row, col);
		else
			locate (row, col);
		#else
		out.gotoxy (pos % 32, pos / 32);
		#endif
	}
	if (endsentence () )
	{
		out << '\n';
		return false;
	}
	BlResult result;
        size_t n;
        bool ended= false;
	do {
                switch (token.code) {
                case ',':
                case ';':
                	// Line if it were preceded by an empty statement
                	break;
		case keyUSING:
			print_using (out);
			ended= true;
			break;
                case keyTAB:
                        //getparenarg (result);
			// Not required to improve Sinclair ZX compatibility.
			expect (result);
                        n= result.integer ();
			out.tab (n);
                        break;
                case keySPC:
                        getparenarg (result);
                        n= result.integer ();
                        //out << std::string (n, ' ');
			out.putspaces (n);
                        break;
                case keyAT:
                	out.flush ();
                        {
				expect (result);
				//BlInteger row= result.integer () + 1;
				BlInteger row= result.integer ();
				requiretoken (',');
				expect (result);
				//BlInteger col= result.integer () + 1;
				BlInteger col= result.integer ();
				#if 0
				if (graphics::ingraphicsmode () )
					graphics::locate (row, col);
				else
					locate (row, col);
				#else
				out.gotoxy (col, row);
				#endif
                        }
                        break;
                default:
                        eval (result);
                        switch (result.type () ) {
                        case VarString:
                                out << result.str ();
                                break;
                        case VarNumber:
                                out << result.number ();
                                break;
			case VarInteger:
				out << result.integer ();
				break;
                        default:
                                throw ErrBlassicInternal;
                                ;
                        }
                }
		if (ended)
			break;
                if (endsentence () )
                {
                        out << '\n';
                        break;
                }
		if (token.code != ';' && token.code != ',')
			throw ErrSyntax;
		if (token.code == ',')
                        out << '\x09';
		gettoken ();
	} while (! endsentence () );
	out.flush ();
	return false;
}

bool RunnerLine::do_for ()
{
	//ProgramPos posfor= runner.getposactual ();
	ProgramPos posfor (getposactual () );
	//posfor.nextchunk ();
	expecttoken (keyIDENTIFIER);
	std::string varname= token.str;
	expecttoken ('=');

	//BlNumber initial= expectnum ();
	BlResult initial;
	expect (initial);

	requiretoken (keyTO);

	//BlNumber final= expectnum ();
	BlResult final;
	expect (final);

	//BlNumber step= (token.code == keySTEP) ? expectnum () : 1;
	BlResult step;
	if (token.code == keySTEP)
		expect (step);
	else
		step= 1;

	switch (token.code)
	{
	case ':':
		posfor.nextchunk ();
		break;
	case keyENDLINE:
		#if 1
		posfor.nextline ();
		#else
		{
			BlLineNumber num= program.getnextnum (line);
			if (num != 0)
				posfor= num;
		}
		#endif
		break;
	default:
		throw ErrSyntax;
	}
	switch (typeofvar (varname) )
	{
	case VarNumber:
		runner.push_for (new ForElementNumber
			(varname, posfor, initial.number (),
				final.number (), step.number () ) );
			//(varname, posfor, initial,
			//	final, step) );
		break;
	case VarInteger:
		runner.push_for (new ForElementInteger
			(varname, posfor, initial.integer (),
				final.integer (), step.integer () ) );
			//(varname, posfor, initial,
			//	final, step) );
		break;
	case VarString:
		throw ErrMismatch;
	default:
		throw ErrBlassicInternal;
	}
	return false;
}

bool RunnerLine::do_next ()
{
	gettoken ();
	//std::string varname;
	if (endsentence () )
        {
		//if (runner.for_empty () )
		//	throw ErrNextWithoutFor;
		ForElement & fe= runner.for_top ();
		if (fe.next () )
		{
			runner.jump_to (fe.getpos () );
			return true;
		}
		runner.for_pop ();
		return false;
	}

	for (;;)
	{
		requiretoken (keyIDENTIFIER);
		//varname= token.str;
		//gettoken ();
		//if (runner.for_empty () )
		//	throw ErrNextWithoutFor;
		ForElement * pfe= & runner.for_top ();
		//if (! varname.empty () && ! fe.isvar (varname) )
		if (! pfe->isvar (token.str) )
		{
			if (sysvar::get (sysvar::TypeOfNextCheck) == 0)
				throw ErrNextWithoutFor;
			else
			{
				// In ZX style NEXT can be omitted.
				do {
					runner.for_pop ();
					pfe= & runner.for_top ();
				} while (! pfe->isvar (token.str) );
			}
		}
		if (pfe->next () )
		{
			runner.jump_to (pfe->getpos () );
			return true;
		}
		runner.for_pop ();
		gettoken ();
		if (endsentence () )
			break;
		requiretoken (',');
		//expecttoken (keyIDENTIFIER);
		//varname= token.str;
		gettoken ();
	}
	return false;
}

bool RunnerLine::do_if ()
{
	BlNumber result= expectnum ();
	requiretoken (keyTHEN);
	if (result != 0) return false;
	else {
		unsigned int iflevel= 1;
		do {
			gettoken ();
			if (token.code == keyIF)
				++iflevel;
			if (token.code == keyELSE)
				--iflevel;
		} while (iflevel > 0 && token.code != keyENDLINE);
		if (token.code == keyELSE)
                {
			fInElse= true;
			return false;
		}
		return true;
	}
}

bool RunnerLine::do_tron ()
{
	gettoken ();
	bool fLine= false;
	if (token.code == keyLINE)
	{
		fLine= true;
		gettoken ();
	}
	BlChannel channel= 0;
	if (token.code == '#')
		channel= expectchannel ();
	require_endsentence ();
	runner.tron (fLine, channel);
	return false;
}

bool RunnerLine::do_troff ()
{
	errorifparam ();
	runner.troff ();
	return false;
}

void RunnerLine::letsubindex (const std::string & varname)
{
        Dimension dims= expectdims ();
	requiretoken ('=');
        BlResult result;
        expect (result);
        require_endsentence ();
	switch (typeofvar (varname) )
	{
	case VarNumber:
                assigndimnumber (varname, dims, result.number () );
		break;
        case VarInteger:
                assigndiminteger (varname, dims, result.integer () );
                break;
	case VarString:
                assigndimstring (varname, dims, result.str () );
		break;
	default:
		throw ErrBlassicInternal;
	}
}

bool RunnerLine::do_let ()
{
	if (token.code == keyLET)
		gettoken ();
	#if 0
	requiretoken (keyIDENTIFIER);
	std::string varname= token.str;
	gettoken ();
	if (token.code == '(')
	{
		letsubindex (varname);
		return false;
	}
	//requiretoken ('=');
	// Crash on hp-ux when failed. Compiler fault?
	// Workaround:
	if (token.code != '=')
		throw ErrSyntax;

        BlResult result;
        expect (result);
	require_endsentence ();
	switch (typeofvar (varname) )
        {
	case VarNumber:
                assignvarnumber (varname, result.number () );
		break;
	case VarInteger:
                assignvarinteger (varname, result.integer () );
		break;
	case VarString:
                assignvarstring (varname, result.str () );
		break;
	default:
		throw ErrBlassicInternal;
	}
	#else
	eval_let ();
	require_endsentence ();
	#endif
	return false;
}

bool RunnerLine::do_goto ()
{
	gettoken ();
	BlLineNumber bln= evallinenumber ();
	require_endsentence ();
	runner.goto_line (bln);
	return true;
}

bool RunnerLine::do_stop ()
{
	errorifparam ();
	BlFile & f= getfile (0);
	f << "**Stopped**";
	//if (line.number () != 0)
	//	f << " in " << line.number ();
	if (pline->number () != 0)
		f << " in " << pline->number ();
	f << '\n';
	//ProgramPos posbreak (runner.getposactual () );
	ProgramPos posbreak (getposactual () );
	posbreak.nextchunk ();
	runner.set_break (posbreak);
	runner.setstatus (ProgramStopped);
	return true;
}

bool RunnerLine::do_cont ()
{
	runner.jump_break ();
        return true;
}

bool RunnerLine::do_clear ()
{
	errorifparam ();
	//clearvars ();
	make_clear ();
	return false;
}

bool RunnerLine::do_gosub ()
{
	gettoken ();
        BlLineNumber dest= evallinenumber ();
	require_endsentence ();
        gosub_line (dest);
	return true;
}

bool RunnerLine::do_return ()
{
	errorifparam ();
	ProgramPos pos;
	runner.gosub_pop (pos);
	runner.jump_to (pos);
	return true;
}

bool RunnerLine::do_poke ()
{
	BlNumber bnAddr= expectnum ();
	requiretoken (',');
	BlChar * addr= (BlChar *) (unsigned int) bnAddr;
	BlNumber bnValue= expectnum ();
	require_endsentence ();
	BlChar value= (BlChar) (unsigned int) bnValue;
	* addr= value;
	return false;
}

bool RunnerLine::do_read ()
{
	BlLineNumber & datanumline= runner.getdatanumline ();
	BlChunk & datachunk= runner.getdatachunk ();
	unsigned short & dataelem= runner.getdataelem ();

	//CodeLine dataline= program.getline (datanumline);
	CodeLine dataline;
	program.getline (datanumline, dataline);
	CodeLine::Token datatok;
otra:
	if (dataline.number () == 0)
		throw ErrDataExhausted;
	if (dataline.number () > datanumline)
	{
		datachunk= 0;
		dataelem= 0;
	}
	//datatok= dataline.gettoken ();
	dataline.gettoken (datatok);
        if (dataline.chunk () < datachunk)
        {
        	while (dataline.chunk () < datachunk)
        	{
	        	//datatok= dataline.gettoken ();
			dataline.gettoken (datatok);
		        if (datatok.code == keyENDLINE)
			        break;
	        }
	        if (datatok.code != keyENDLINE)
			//datatok= dataline.gettoken ();
			dataline.gettoken (datatok);
        }
	if (datatok.code == keyENDLINE)
	{
		cerr << "Read searching next line" << endl;
		//dataline= program.getnextline (dataline);
		program.getnextline (dataline);
		goto otra;
	}
otra2:
	while (datatok.code != keyDATA)
	{
		dataelem= 0;
		datachunk= dataline.chunk ();
		do {
			//datatok= dataline.gettoken ();
			dataline.gettoken (datatok);
		} while (datatok.code != keyENDLINE &&
			dataline.chunk () == datachunk);
		if (datatok.code == keyENDLINE)
		{
			//cerr << "Read searching next line" << endl;
			//dataline= program.getnextline (dataline);
			program.getnextline (dataline);
			if (dataline.number () == 0)
				throw ErrDataExhausted;
			//datatok= dataline.gettoken ();
			dataline.gettoken (datatok);
		}
                else
                        //datatok= dataline.gettoken ();
			dataline.gettoken (datatok);
	}
        datatok= dataline.getdata ();
	unsigned short elem= 0;
otra3:
	while (elem < dataelem)
	{
		//datatok= dataline.gettoken ();
		dataline.gettoken (datatok);
		if (datatok.code == ':')
		{
                        //datatok= dataline.gettoken ();
			dataline.gettoken (datatok);
                        dataelem= 0;
			goto otra2;
                }
		if (datatok.code == keyENDLINE)
		{
			//cerr << "Read searching next line" << endl;
			//dataline= program.getnextline (dataline);
			program.getnextline (dataline);
			goto otra;
		}
		if (datatok.code != ',')
			throw ErrSyntax;
                datatok= dataline.getdata ();
                if (datatok.isendsentence () )
                	throw ErrSyntax;
		++elem;
	}

	//cerr << "(Data en linea " << dataline.number () <<
	//	' ' << datatok.str << ')';

	datanumline= dataline.number ();
        datachunk= dataline.chunk ();
	dataelem= (unsigned short) (elem + 1);
	gettoken ();
	if (token.code != keyIDENTIFIER)
		throw ErrSyntax;
	std::string varname (token.str);
	gettoken ();
	if (token.code == '(')
	{
		Dimension dims= expectdims ();
		switch (typeofvar (varname) )
		{
		case VarNumber:
			assigndimnumber (varname, dims, datatok.number () );
			break;
		case VarInteger:
			assigndiminteger (varname, dims,
				BlInteger (datatok.number () ) );
			break;
		case VarString:
			switch (datatok.code)
			{
			case keySTRING:
				assigndimstring (varname, dims, datatok.str);
				break;
			case keyINTEGER:
				assigndimstring (varname, dims,
					util::to_string (datatok.valueint) );
				break;
			default:
				throw ErrBlassicInternal;
			}
			break;
		default:
			cerr << "Unexpected array type in READ" << endl;
			throw ErrBlassicInternal;
		}
	}
	else
	{
		switch (typeofvar (varname) )
		{
		case VarNumber:
			assignvarnumber (varname, datatok.number () );
			break;
		case VarInteger:
			assignvarinteger (varname,
				BlInteger (datatok.number () ) );
			break;
		case VarString:
			switch (datatok.code)
			{
			case keySTRING:
				assignvarstring (varname, datatok.str);
				break;
			case keyINTEGER:
				assignvarstring (varname,
					util::to_string (datatok.valueint) );
				break;
			default:
				throw ErrBlassicInternal;
			}
			break;
		default:
			cerr << "Unexpected var type in READ" << endl;
			throw ErrBlassicInternal;
		}
	}
        if (token.code == ',')
                goto otra3;
	require_endsentence ();
	return false;
}

bool RunnerLine::do_data ()
{
	do {
		gettoken ();
	} while (! endsentence () );
	return false;
}

bool RunnerLine::do_restore ()
{
	gettoken ();
	BlLineNumber bln= 0;
	if (! endsentence () )
	{
		bln= evallinenumber ();
		require_endsentence ();
	}
	runner.setreadline (bln);
	return false;
}

namespace {

#if 0
struct VarPointer {
	VarType type;
	union {
		BlNumber * pnumber;
		BlInteger * pinteger;
		std::string * pstring;
	};
	int clearvar () const
        // returns an int because Borland can't use mem_fun_ref
        // with a void function.
	{
		switch (type)
                {
		case VarNumber:
			* pnumber= 0;
			break;
		case VarInteger:
			* pinteger= 0;
			break;
		case VarString:
			pstring->erase ();
			break;
		default:
			throw ErrBlassicInternal;
		}
                return 0;
	}
};
#else
struct clearvar {
	void operator () (VarPointer & vt) const
	{
		switch (vt.type)
		{
		case VarNumber:
			* vt.pnumber= 0;
			break;
		case VarInteger:
			* vt.pinteger= 0;
			break;
		case VarString:
			vt.pstring->erase ();
			break;
		default:
			throw ErrBlassicInternal;
		}
	}
};
#endif

bool isdelimdiscardingwhite (std::istream & is, char delim)
{
	char c;
	while (is.get (c) )
	{
		if (c == delim)
		{
			is.putback (c);
			return true;
		}
		if (! isspace (c) )
		{
			is.putback (c);
			return false;
		}
	}
	return true; // We count eof as delimiter
}

} // namespace

bool RunnerLine::do_input ()
{
	gettoken ();
	BlChannel channel= 0;
	if (token.code == '#')
	{
		channel= expectchannel ();
		requiretoken (',');
		gettoken ();
	}
        std::string prompt;
	switch (token.code)
	{
	case keySTRING:
                prompt= token.str;
		gettoken ();
		switch (token.code)
                {
		case ';':
			break;
		case ',':
                        prompt+= "? ";
			break;
		default:
			throw ErrSyntax;
		}
		std::cout << std::flush;
		gettoken ();
		break;
	default:
                prompt= "? ";
	}

	// Parse the list of variables.

        #define WORKAROUND_SYNTERR 1
        // To avoid an strange error on Borland Builder.
        #if WORKAROUND_SYNTERR
        bool synterr= false;
        #endif
	std::vector <VarPointer> inputvars;
	for (;;)
	{
		#if 0
		requiretoken (keyIDENTIFIER);
		std::string varname= token.str;
		gettoken ();
		Dimension d;
		bool isarray= false;
		if (token.code == '(')
		{
			d= expectdims ();
			isarray= true;
		}
		VarPointer vp;
		vp.type= typeofvar (varname);
		switch (vp.type)
		{
		case VarNumber:
			vp.pnumber= isarray ?
				addrdimnumber (varname, d) :
				addrvarnumber (varname);
			break;
		case VarInteger:
			vp.pinteger= isarray ?
				addrdiminteger (varname, d) :
				addrvarinteger (varname);
			break;
		case VarString:
			vp.pstring= isarray ?
				addrdimstring (varname, d) :
				addrvarstring (varname);
			break;
		default:
			throw ErrBlassicInternal;
		}
		#else
		VarPointer vp= evalvalpointer ();
		#endif
		inputvars.push_back (vp);
		if (endsentence () )
			break;
                #if WORKAROUND_SYNTERR
                if (token.code != ',')
                {
                        synterr= true;
                        break;
                }
                #else
		requiretoken (',');
                #endif
		gettoken ();
	}
        #if WORKAROUND_SYNTERR
        if (synterr)
                throw ErrSyntax;
        #endif

	// Prepare input channel

	std::string input;
	BlFile & in= getfile (channel);
	char quote= in.quote ();
	char delimiter= in.delimiter ();

	// Accept line from input

	for (;;)
	{
		if (channel == 0)
		{
			clean_input ();
			cursorvisible ();
		}
		if (! prompt.empty () )
		{
			if (channel == 0 || in.istextwindow () )
				in << prompt;
		}
		in.getline (input);
		if (channel == 0)
			cursorinvisible ();

		if (fInterrupted)
        	{
			std::cin.clear ();
			if (runner.getbreakstate () != Runner::BreakCont)
				// This throw causes an error in windows,
				// we return and let the caller catch
				// the interrupted condition instead.
				//throw BlBreak ();
				return false;
			else
			{
				fInterrupted= false;
				in << '\n';
				continue;
			}
		}
		break;
	}

	// Process the line entered

	// We must take into account that an whitespace can be a delimiter
	// (tipically a tab).

	const size_t l= inputvars.size ();
	size_t i;
	std::istringstream iss (input);
	iss.unsetf (std::ios::skipws);
	for (i= 0; i < l; ++i)
        {
		const VarPointer & vp= inputvars [i];
		switch (vp.type)
                {
		case VarNumber:
			{
				BlNumber n;
				if (isdelimdiscardingwhite (iss, delimiter) )
					n= 0;
				else
				{
					iss >> n;
					if (! iss)
						throw ErrMismatch;
				}
				* vp.pnumber= n;
			}
			break;
		case VarInteger:
			{
				BlInteger n;
				if (isdelimdiscardingwhite (iss, delimiter) )
					n= 0;
				else
				{
					iss >> n;
					if (! iss)
						throw ErrMismatch;
				}
				* vp.pinteger= n;
			}
			break;
		case VarString:
			{
				std::string str;
				char c;
				bool atend= isdelimdiscardingwhite
					(iss, delimiter);
				if (! atend)
				{
					iss >> c;
					if (! iss)
						atend= true;
				}
				if (! atend)
				{
					if (c == quote)
					{
						#if 0
						while (iss >> c && c != quote)
						{
							str+= c;
						}
						#else
						while (iss >> c)
						{
							if (c != quote)
								str+= c;
							else
							{
								iss >> c;
								if (! iss)
									break;
								if (c == quote)
									str+= c;
								else
								{
									iss.unget ();
									break;
								}
							}
						}
						#endif
					}
					else
					{
						while (iss && c != delimiter)
						{
							str+= c;
							iss >> c;
						}
						if (iss)
							iss.unget ();
					}
				}
				* vp.pstring= str;
			}
			break;
		default:
			throw ErrBlassicInternal;
		}
		if (isdelimdiscardingwhite (iss, delimiter) )
		{
			if (iss.eof () )
			{
				++i;
				break;
			}
			else
				iss.get ();
		}
		else
			throw ErrMismatch;
	}

	// If not enough data entered, clear remaining vars.

	#if 0
	std::for_each (inputvars.begin () + i, inputvars.end (),
		std::mem_fun_ref (& VarPointer::clearvar) );
	#else
	std::for_each (inputvars.begin () + i, inputvars.end (),
		clearvar () );
	#endif

	return false;
}

void RunnerLine::do_line_input ()
{
	gettoken ();
	BlChannel channel= 0;
	if (token.code == '#')
        {
		channel= expectchannel ();
		requiretoken (',');
		gettoken ();
	}
	requiretoken (keyIDENTIFIER);
	std::string varname= token.str;
	gettoken ();
	require_endsentence ();
	if (typeofvar (varname) != VarString)
		throw ErrMismatch;

	std::string value;
	BlFile & in= getfile (channel);
	if (channel == 0)
	{
		clean_input ();
		cursorvisible ();
	}

	for (;;)
	{
		in.getline (value);
		if (fInterrupted)
        	{
			std::cin.clear ();
			if (runner.getbreakstate () != Runner::BreakCont)
				return;
			else
			{
				fInterrupted= false;
				in << '\n';
				continue;
			}
		}
		break;
	}

	if (channel == 0)
		cursorinvisible ();

	assignvarstring (varname, value);
}

bool RunnerLine::do_line ()
{
	gettoken ();
	if (token.code == keyINPUT)
	{
		do_line_input ();
		return false;
	}
	graphics::Point from;
	if (token.code == '-')
		from= graphics::getlast ();
	else
	{
		requiretoken ('(');
		BlInteger x= expectinteger ();
		requiretoken (',');
		BlInteger y= expectinteger ();
		requiretoken (')');
		expecttoken ('-');
		from= graphics::Point (x, y);
	}
	expecttoken ('(');
	BlInteger x= expectinteger ();
	requiretoken (',');
	BlInteger y= expectinteger ();
	requiretoken (')');
	gettoken ();
	enum Type { TypeLine, TypeRect, TypeFillRect };
	Type type= TypeLine;
	if (! endsentence () )
	{
		requiretoken (',');
		gettoken ();
		if (token.code != ',')
		{
			BlInteger color= evalinteger ();
			graphics::setcolor (color);
		}
		if (token.code == ',')
		{
			gettoken ();
			requiretoken (keyIDENTIFIER);
			if (token.str == "B")
				type= TypeRect;
			else if (token.str == "BF")
				type= TypeFillRect;
			else throw ErrSyntax;
			gettoken ();
		}
		require_endsentence ();
	}

	switch (type)
	{
	case TypeLine:
		graphics::move (from.x, from.y);
		graphics::line (x, y);
		break;
	case TypeRect:
		graphics::rectangle (from, graphics::Point (x, y) );
		break;
	case TypeFillRect:
		graphics::rectanglefilled (from, graphics::Point (x, y) );
		break;
	}

	return false;
}

bool RunnerLine::do_randomize ()
{
	gettoken ();
	unsigned int seedvalue;
	if (endsentence () )
		seedvalue= time (NULL);
	else
	{
		BlNumber result= evalnum ();
		//errorifparam ();
                require_endsentence ();
		//seedvalue= static_cast <unsigned int> (result);
		seedvalue= util::checked_cast <unsigned int>
			(result, ErrBadSubscript);
	}

	#if 0
	srand (seedvalue);
	#else
	randgen.seed (seedvalue);
	#endif

	return false;
}

bool RunnerLine::do_auto ()
{
	//if (line.number () != 0)
	if (pline->number () != 0)
		throw ErrInvalidCommand;
	BlLineNumber
		auto_ini= sysvar::get32 (sysvar::AutoInit),
		auto_inc= sysvar::get32 (sysvar::AutoInc);
	gettoken ();
	if (! endsentence () )
	{
		if (token.code != ',')
		{
			switch (token.code)
			{
			case keyNUMBER:
				auto_ini= (BlLineNumber) token.number ();
				break;
			case keyINTEGER:
				auto_ini= token.integer ();
				break;
			default:
				throw ErrSyntax;
			}
			gettoken ();
		}
		if (token.code == ',')
		{
			gettoken ();
			switch (token.code)
			{
			case keyNUMBER:
				auto_inc= (BlLineNumber) token.number ();
				break;
			case keyINTEGER:
				auto_inc= token.integer ();
				break;
			default:
				throw ErrSyntax;
			}
			gettoken ();
		}
		require_endsentence ();
	}
	#if 0
	blnAuto= auto_ini;
	blnAutoInc= auto_inc;
	#else
	runner.setauto (auto_ini, auto_inc);
	#endif
	return false;
}

bool RunnerLine::do_dim ()
{
	do {
		expecttoken (keyIDENTIFIER);
		std::string varname= token.str;
		gettoken ();
                Dimension dims= expectdims ();
                switch (typeofvar (varname) )
                {
                case VarNumber:
                        dimvarnumber (varname, dims);
                        break;
                case VarInteger:
                        dimvarinteger (varname, dims);
                        break;
                case VarString:
                        dimvarstring (varname, dims);
                        break;
                default:
                        throw ErrBlassicInternal;
                }
	} while (token.code == ',');
	require_endsentence ();
	return false;
}

bool RunnerLine::do_system ()
{
	errorifparam ();
	throw Exit (0);
}

bool RunnerLine::do_on ()
{
        enum TypeOn { OnNoValid, OnGoto, OnGosub };
        gettoken ();
	switch (token.code)
	{
	case keyERROR:
		{
			expecttoken (keyGOTO);
			gettoken ();
			BlLineNumber bln= evallinenumber ();
			errorifparam ();
			runner.seterrorgoto (bln);
		}
		return false;
	case keyBREAK:
		gettoken ();
		switch (token.code)
		{
		case keySTOP:
			runner.setbreakstate (Runner::BreakStop);
			break;
		case keyCONT:
			runner.setbreakstate (Runner::BreakCont);
			break;
		case keyGOSUB:
			{
				gettoken ();
				BlLineNumber bln= evallinenumber ();
				errorifparam ();
				runner.setbreakgosub (bln);
			}
			break;
		default:
			throw ErrSyntax;
		}
		return false;
	default:
		break;
	}
        BlNumber bn= evalnum ();
        size_t n= (size_t) bn;

        TypeOn type= OnNoValid;
        switch (token.code)
        {
        case keyGOTO:
                type= OnGoto;
                break;
        case keyGOSUB:
                type= OnGosub;
                break;
        default:
                throw ErrSyntax;
        }
	BlLineNumber bln;
        std::vector <BlLineNumber> line;
        do {
		gettoken ();
		bln= evallinenumber ();
		line.push_back (bln);
        } while (token.code == ',');
	require_endsentence ();
        size_t l= line.size ();
        if (n > 0 && n <= l)
        {
                switch (type)
                {
                case OnGoto:
			runner.goto_line (line [n - 1] );
                        break;
                case OnGosub:
			gosub_line (line [n - 1] );
                        break;
		default:
			; // Avoid a warning
                }
                return true;
        }
        return false;
}

bool RunnerLine::do_error ()
{
        BlNumber blCod= expectnum ();
	require_endsentence ();
        throw BlErrNo (blCod);
}

bool RunnerLine::do_open ()
{
        BlCode op= token.code;
        BlChannel channel;
        std::string filename;
        BlFile::OpenMode mode= BlFile::Input;
	size_t record_len= 128;

	gettoken ();
	if (token.code == keyERROR)
	{
		if (op != keyOPEN)
			throw ErrSyntax;
		op= keyERROR;
		expecttoken (keyAS);
		gettoken ();
		if (token.code == '#')
			gettoken ();
		channel= evalchannel ();
	}
	else
	{
		BlResult result;
		eval (result);
		std::string firstarg= result.str ();
		switch (token.code)
		{
		case keyFOR:
		// Newer syntax.
			filename= firstarg;
			gettoken ();
			switch (token.code)
                        {
			case keyINPUT:
				mode= BlFile::Input;
				break;
			case keyOUTPUT:
				mode= BlFile::Output;
				break;
			case keyAPPEND:
				mode= BlFile::Append;
				break;
			case keyRANDOM:
				mode= BlFile::Random;
				break;
			default:
				throw ErrSyntax;
			}
			expecttoken (keyAS);
			gettoken ();
			if (token.code == '#')
				gettoken ();
			channel= evalchannel ();

			if (token.code == keyLEN)
                        {
				expecttoken ('=');
				gettoken ();
				BlNumber bn= evalnum ();
				record_len= size_t (bn);
				if (mode != BlFile::Random)
					throw ErrFileMode;
			}
			break;
		case ',':
		// Older syntax.
			if (firstarg == "I" || firstarg == "i")
				mode= BlFile::Input;
			else if (firstarg == "O" || firstarg == "o")
				mode= BlFile::Output;
			else if (firstarg == "A" || firstarg == "a")
				mode= BlFile::Append;
			else if (firstarg == "R" || firstarg == "r")
				mode= BlFile::Random;
			else
				throw ErrFileMode;
			gettoken ();
			if (token.code == '#')
				gettoken ();
			channel= evalchannel ();
			requiretoken (',');
			filename= expectstring ();
			if (token.code == ',')
			{
				if (mode != BlFile::Random)
					throw ErrFileMode;
				BlNumber bn= expectnum ();
				record_len= size_t (bn);
			}
			break;
		default:
			throw ErrSyntax;
		} // switch
	} // if

	require_endsentence ();
	if (channel == 0)
		throw ErrFileNumber;

        BlFile * pbf;
        switch (op)
        {
        case keyOPEN:
		if (mode == BlFile::Random)
			pbf= new BlFileRandom (filename, record_len);
		else
			pbf= new BlFileRegular (filename, mode);
		break;
        case keyPOPEN:
                if (mode != BlFile::Input && mode != BlFile::Output)
                        throw ErrFileMode;
                pbf= new BlFilePopen (filename, mode);
                break;
	case keyERROR:
		pbf= new BlFileOutput (cerr);
		break;
        default:
                throw ErrBlassicInternal;
        }

	runner.setfile (channel, pbf);
	return false;
}

bool RunnerLine::do_close ()
{
        gettoken ();
	if (endsentence () )
	{
		runner.close_all ();
		return false;
	}
	for (;;)
	{
		if (token.code == '#')
			gettoken ();
		BlChannel channel= evalchannel ();
		if (channel == 0)
			throw ErrFileNumber;
		runner.closechannel (channel);
		if (token.code != ',')
		{
			if (endsentence () )
				break;
			throw ErrSyntax;
		}
		gettoken ();
	}
	return false;
}

bool RunnerLine::do_locate ()
{
	gettoken ();
	BlChannel ch= 0;
	if (token.code == '#')
	{
		ch= expectchannel ();
		requiretoken (',');
		gettoken ();
	}
	BlResult brRow;
	eval (brRow);
	requiretoken (',');
	//BlNumber bnCol= expectnum ();
	BlResult brCol;
	expect (brCol);
	require_endsentence ();
	#if 0
        if (graphics::ingraphicsmode () )
                //graphics::locate (int (bnRow), int (bnCol) );
		graphics::locate (brRow.integer (), brCol.integer () );
        else
	        //locate (int (bnRow), int (bnCol) );
		locate (brRow.integer (), brCol.integer () );
	#else
	BlFile & out= getfile (ch);
	out.gotoxy (brCol.integer () - 1, brRow.integer () - 1);
	#endif
	return false;
}

bool RunnerLine::do_cls ()
{
	gettoken ();
	BlChannel ch= 0;
	if (token.code == '#')
	{
		ch= expectchannel ();
	}
	require_endsentence ();
	BlFile & out= getfile (ch);
	out.cls ();
	return false;
}

bool RunnerLine::do_write ()
{
	gettoken ();
	BlChannel channel= 0;
	if (token.code == '#')
        {
                channel= expectchannel ();
		requiretoken (',');
                gettoken ();
        }
	BlFile & out= getfile (channel);
        BlResult result;
        char quote= out.quote ();
        for (;;)
        {
        	if (token.code != ',')
        	{
			eval (result);
			switch (result.type () )
			{
			case VarNumber:
				out << result.number ();
				break;
			case VarInteger:
				out<< result.integer ();
				break;
			case VarString:
				if (quote) out << quote;
				out << result.str ();
				if (quote) out << quote;
				break;
			default:
				throw ErrBlassicInternal;
			}
		}
                if (token.code == ',')
                {
                        out << out.delimiter ();
                        gettoken ();
                }
                else break;
        }
	require_endsentence ();
	out << '\n';
	return false;
}

bool RunnerLine::do_mode ()
{
	//BlNumber mode= expectnum ();
	BlResult result;
	expect (result);

	BlInteger mode;
	if (result.type () == VarString)
	{
		require_endsentence ();
		graphics::setmode (result.str () );
		mode= 1;
	}
	else
	{
		mode= result.integer ();
		if (endsentence () )
			graphics::setmode (mode);
		else
		{
			requiretoken (',');
			//BlNumber height= expectnum ();
			expect (result);
			BlInteger height= result.integer ();
			bool inverty= false;
			if (token.code == ',')
			{
				expect (result);
				inverty= result.integer ();
			}
			require_endsentence ();
			graphics::setmode (mode, height, inverty);
		}
	}
	runner.destroy_windows ();
	if (mode != 0)
		runner.setfile (0, new BlFileWindow (0) );
	else
		runner.setfile (0, new BlFileConsole (std::cin, std::cout) );
	return false;
}

bool RunnerLine::do_move ()
{
	#if 0
	BlNumber x= expectnum ();
	requiretoken (',');
	BlNumber y= expectnum ();
	require_endsentence ();
	graphics::move (int (x), int (y) );
	#else
	BlInteger x, y;
	getdrawargs (x, y);
	graphics::move (x, y);
	#endif
	return false;
}

bool RunnerLine::do_color ()
{
        gettoken ();
        if (token.code != ',')
        {
	        BlNumber color= evalnum ();
	        if (graphics::ingraphicsmode () )
			graphics::setcolor (int (color) );
		else
			textcolor (int (color) );
                if (endsentence () )
                        return false;
                requiretoken (',');
        }
        gettoken ();
        if (token.code != ',')
        {
                BlNumber mode= evalnum ();
		if (graphics::ingraphicsmode () )
			graphics::setbackground (int (mode) );
		else
			textbackground (int (mode) );
                if (endsentence () )
                        return false;
                requiretoken (',');
        }
        gettoken ();
        // Border color in Gwbasic. Ignored.
        evalnum ();
	require_endsentence ();
	return false;
}

bool RunnerLine::do_get ()
{
        gettoken ();
        if (token.code != keyIDENTIFIER ||
                typeofvar (token.str) != VarString)
        {
                // GET #
                if (token.code == '#')
                        gettoken ();
                BlChannel channel= evalchannel ();
                size_t pos= 0;
        	if (token.code == ',')
                {
	        	BlNumber bnPos= expectnum ();
        		pos= size_t (bnPos);
                        if (pos == 0)
                                throw ErrBadRecord;
	        }
		require_endsentence ();
		BlFile & out= getfile (channel);
	        out.get (pos);
                return false;
        }
        // GET var$
        std::string varname= token.str;
        gettoken ();
	require_endsentence ();
        std::string str= getkey ();
        assignvarstring (varname, str);
	return false;
}

bool RunnerLine::do_label ()
{
        expecttoken (keyIDENTIFIER);
	//std::string label= token.str;
        gettoken ();
	require_endsentence ();
	return false;
}

bool RunnerLine::do_delimiter ()
{
	gettoken ();
        BlChannel channel= 0;
        if (token.code == '#')
        {
                channel= expectchannel ();
                requiretoken (',');
                gettoken ();
        }
	BlFile & in= getfile (channel);
	std::string str= evalstring ();
	char delim= 0, quote= 0, escape= 0;
	if (! str.empty () )
		delim= str [0];
	if (! endsentence () )
	{
		requiretoken (',');
		str= expectstring ();
		if (! str.empty () )
			quote= str [0];
		if (! endsentence () )
		{
			requiretoken(',');
			str= expectstring ();
			if (! str.empty () )
				escape= str [0];
			require_endsentence ();
		}
	}
	in.delimiter (delim);
	in.quote (quote);
	in.escape (escape);
	return false;
}

bool RunnerLine::do_repeat ()
{
	errorifparam ();
	//ProgramPos posrepeat (runner.getposactual () );
	ProgramPos posrepeat (getposactual () );
	if (token.code == keyENDLINE) {
		// ***REVISAR***
		if (posrepeat.getnum () != 0)
			posrepeat.nextline ();
		else
			posrepeat.nextchunk ();
	}
	else
		posrepeat.nextchunk ();
	runner.repeat_push (RepeatElement (posrepeat) );
	return false;
}

bool RunnerLine::do_until ()
{
	if (runner.repeat_empty () )
		throw ErrUntilWithoutRepeat;
	BlResult br;
	expect (br);
	bool cond= br.tobool ();
        require_endsentence ();
	if (cond)
        {
		runner.repeat_pop ();
		return false;
	}
	RepeatElement re= runner.repeat_top ();
	runner.jump_to (re.getpos () );
	return true;
}

bool RunnerLine::do_while ()
{
	//ProgramPos poswhile (runner.getposactual () );
	ProgramPos poswhile (getposactual () );
	BlResult br;
	expect (br);
	bool cond= br.tobool ();
	require_endsentence ();
	if (cond)
	{
		if (! runner.in_wend () )
			runner.while_push (WhileElement (poswhile) );
		else
			runner.in_wend (false);
                return false;
        }
	if (runner.in_wend () )
	{
		runner.while_pop ();
		runner.in_wend (false);
	}
        bool sameline= true;
	for (size_t level= 1; level > 0; )
	{
                getnextchunk ();
                if (token.code == keyENDLINE)
                {
                        //if (line.number () == 0)
                        if (pline->number () == 0)
                                throw ErrWhileWithoutWend;
                        //line= program.getnextline (line);
			//program.getnextline (line);
			program.getnextline (* pline);
                        //if (line.number () == 0)
                        if (pline->number () == 0)
                                throw ErrWhileWithoutWend;
			sameline= false;
                        gettoken ();
                }
		switch (token.code)
		{
		case keyWHILE: ++level; break;
		case keyWEND: --level; break;
		default: ;
		}
        }
        gettoken ();
	require_endsentence ();
	if (sameline)
		return false;
	//ProgramPos pos (line.number (), line.chunk () );
	ProgramPos pos (pline->number (), pline->chunk () );
	if (token.code == keyENDLINE)
	{
		// Line can't be 0.
		pos.nextline ();
	}
	runner.jump_to (pos);
	return true;
}

bool RunnerLine::do_wend ()
{
        errorifparam ();
	if (runner.while_empty () )
		throw ErrWendWithoutWhile;
	WhileElement w= runner.while_top ();
	runner.jump_to (w.getpos () );
	runner.in_wend (true);
        return true;
}

bool RunnerLine::do_plot ()
{
	BlInteger x, y;
	std::vector <graphics::Point> points;
	gettoken ();
	if (token.code == keyTO)
	{
		points.push_back (graphics::getlast () );
	}
	else
	{
		x= evalinteger ();
		requiretoken (',');
		y= expectinteger ();
		if (token.code != keyTO)
		{
			getinkparams ();
			graphics::plot (x, y);
			return false;
		}
		points.push_back (graphics::Point (int (x), int (y) ) );
	}
	for (;;)
	{
		x= expectinteger ();
		requiretoken (',');
		y= expectinteger ();
		points.push_back (graphics::Point (x, y) );
		if (endsentence () )
			break;
		requiretoken (keyTO);
	}
	graphics::plot (points);
	return false;
}

bool RunnerLine::do_resume ()
{
	ProgramPos posresume= runner.geterrpos ();
	if (posresume.getnum () == 0)
		throw ErrCannotResume;
        gettoken ();
        if (token.code == keyNEXT)
	{
		posresume.nextchunk ();
                gettoken ();
        }
        else if (! endsentence () )
	{
		BlLineNumber l= evallinenumber ();
		posresume= l;
	}
        require_endsentence ();
	//if (posresume.getnum () == 0)
        //        throw ErrCannotResume;
        runner.jump_to (posresume);
	runner.clearerror ();
        return true;
}

bool RunnerLine::do_delete ()
{
	gettoken ();
	BlLineNumber iniline, endline;
	evallinerange (iniline, endline);
	require_endsentence ();
	program.deletelines (iniline, endline);
	runner.setstatus (ProgramEnded);
	return true;
}

bool RunnerLine::do_local ()
{
	do {
		expecttoken (keyIDENTIFIER);
		runner.gosub_addlocal (token.str);
		gettoken ();
	} while (token.code == ',');
	require_endsentence ();
	return false;
}

bool RunnerLine::do_put ()
{
	gettoken ();
	BlChannel channel;
	if (token.code == '#')
		gettoken ();
	channel= evalchannel ();
	size_t pos= 0;
	if (token.code == ',')
        {
		BlNumber bnPos= expectnum ();
		pos= size_t (bnPos);
	}
	require_endsentence ();

	BlFile & out= getfile (channel);
	out.put (pos);

	return false;
}

bool RunnerLine::do_field ()
{
	gettoken ();
	BlChannel channel;
	if (token.code == '#')
		gettoken ();
	channel= evalchannel ();
	BlFile & out= getfile (channel);
        std::vector <BlFile::field_element> fe;
        do {
                requiretoken (',');
                BlNumber bnSize= expectnum ();
                size_t size= size_t (bnSize);
                requiretoken (keyAS);
                expecttoken (keyIDENTIFIER);
                fe.push_back ( BlFile::field_element (size, token.str) );
                gettoken ();
        } while (! endsentence () );
        out.field (fe);
        return false;
}

bool RunnerLine::do_lset ()
{
        BlFile::Align align;
        switch (token.code)
        {
        case keyLSET: align= BlFile::AlignLeft; break;
        case keyRSET: align= BlFile::AlignRight; break;
        default: throw ErrBlassicInternal;
        }
        gettoken ();
        if (token.code != keyIDENTIFIER)
                throw ErrSyntax;
        std::string var= token.str;
        if (typeofvar (var) != VarString)
                throw ErrMismatch;
        expecttoken ('=');
        std::string value= expectstring ();
	require_endsentence ();
	runner.assign_channel_var (var, value, align);
        return false;
}

bool RunnerLine::do_socket ()
{
	std::string host= expectstring ();
	requiretoken (',');
	BlNumber bn= expectnum ();
	requiretoken (keyAS);
	gettoken ();
	if (token.code == '#')
		gettoken ();
	BlChannel channel= evalchannel ();
	require_endsentence ();
	BlFile * pbf= new BlFileSocket (host, short (bn) );
	runner.setfile (channel, pbf);
	return false;
}

bool RunnerLine::do_mid_s ()
{
	expecttoken ('(');
	expecttoken (keyIDENTIFIER);
	std::string varname= token.str;
	if (typeofvar (varname) != VarString)
		throw ErrMismatch;
	expecttoken (',');
	size_t inipos;
	{
		BlNumber bnInipos= expectnum ();
		inipos= util::checked_cast <size_t> (bnInipos, ErrMismatch);
	}
	if (inipos == 0)
		throw ErrMismatch;
	--inipos;
	size_t len= 0; // Initialized to avoid warning.
	bool fLen= false;
	if (token.code == ',')
	{
		BlNumber bnLen= expectnum ();
		len= util::checked_cast <size_t> (bnLen, ErrMismatch);
		fLen= true;
	}
	requiretoken (')');
	expecttoken ('=');
	std::string value= expectstring ();
	require_endsentence ();
	size_t l= value.size ();
	if (! fLen || len > l)
		len= l;

	std::string * presult= addrvarstring (varname);
	l= presult->size ();
	if (inipos >= l)
		return false;
	if (inipos + len > l)
		len= l - inipos;
	std::copy (value.begin (), value.begin () + len,
		presult->begin () + inipos);
	return false;
}

bool RunnerLine::do_draw ()
{
	//BlNumber x= expectnum ();
	BlResult r;
	expect (r);
	if (r.type () == VarString)
		graphics::draw (r.str () );
	else
	{
		BlInteger x= r.integer ();
		#if 0
		requiretoken (',');
		//BlNumber y= expectnum ();
		expect (r);
		BlInteger y= r.integer ();

		require_endsentence ();
		#else
		BlInteger y;
		getdrawargs (y);
		#endif
		//graphics::line (int (x), int (y) );
		graphics::line (x, y);
	}
	return false;
}

namespace {

std::string quoteescape (const std::string & str)
{
        std::string result;
        for (std::string::size_type i= 0, l= str.size (); i < l; ++i)
        {
                char c= str [i];
                if (c == '"')
                        result+= "\"\"";
                else
                        result+= c;
        }
        return result;
}

} // namespace

bool RunnerLine::do_def_fn ()
{
	expecttoken (keyIDENTIFIER);
	std::string fnname= token.str;
	ParameterList param;
	gettoken ();
	switch (token.code)
	{
	case '=':
		// Function without parameters
		break;
	case '(':
		do {
			expecttoken (keyIDENTIFIER);
			param.push_back (token.str);
			gettoken ();
		} while (token.code == ',');
		requiretoken (')');
		gettoken ();
		if (token.code != '=' && ! endsentence () )
			throw ErrSyntax;
		break;
	default:
		if (endsentence () )
			break; // Multi sentence body without parameters
		throw ErrSyntax;
	}

	bool retval= false;
	Function * pf;

	if (endsentence () )
	{
		// Multi sentence body.
		//ProgramPos posfn (line.number (), line.chunk () );
		ProgramPos posfn (pline->number (), pline->chunk () );
		if (token.code == keyENDLINE)
			posfn.nextline ();
		bool sameline= true;
		for (bool finding= true; finding;)
		{
			getnextchunk ();
			if (token.code == keyENDLINE)
			{
				//if (line.number () == 0)
				if (pline->number () == 0)
					throw ErrInvalidDirect;
				//line= program.getnextline (line);
				//program.getnextline (line);
				program.getnextline (* pline);
				//if (line.number () == 0)
				if (pline->number () == 0)
					throw ErrIncompleteDef;
				sameline= false;
				gettoken ();
			}
			switch (token.code)
                        {
			case keyFN:
				gettoken ();
				if (token.code == keyEND)
					finding= false;
				else
					throw ErrSyntax;
				break;
			}
		}
		gettoken ();
		require_endsentence ();
		if (! sameline)
                {
			//ProgramPos pos (line.number (), line.chunk () );
			ProgramPos pos (pline->number (), pline->chunk () );
			if (token.code == keyENDLINE)
			{
				// Line can't be 0.
				pos.nextline ();
			}
			runner.jump_to (pos);
			retval= true;
		}
		pf= new Function (posfn, param);
	}
	else
	{
		// Single sentence body.
		gettoken ();
		std::string fndef ("0 ");
		// The "0 " is the fake line number.
		for ( ; ! endsentence (); gettoken () )
		{
			if (token.code < 256)
				fndef+= token.code;
			else
				switch (token.code)
                                {
				case keyIDENTIFIER:
				case keyNUMBER:
					fndef+= token.str;
	                                break;
                                case keyINTEGER:
                                        fndef+= util::to_string
                                        	(token.integer () );
                                        break;
	                        case keySTRING:
	                                fndef+= '"';
	                                fndef+= quoteescape (token.str);
	                                fndef+= '"';
	                                break;
	                        default:
	                                fndef+= decodekeyword (token.code);
	                        }
	                fndef+= ' ';
	        }
		pf= new Function (fndef, param);
	}
	pf->insert (fnname);
	delete pf;

        return retval;
}

namespace {

inline char get_letter (const std::string & str)
{
	if (str.size () != 1)
		throw ErrSyntax;
	char c= str [0];
	if (! isalpha (c) )
		throw ErrSyntax;
	return c;
}

}

void RunnerLine::definevars (VarType type)
{
	do {
		expecttoken (keyIDENTIFIER);
		char c= get_letter (token.str);
		gettoken ();
		if (token.code == '-')
		{
			expecttoken (keyIDENTIFIER);
			char c2= get_letter (token.str);
			definevar (type, c, c2);
			gettoken ();
		}
		else
			definevar (type, c);
	} while (token.code == ',');
	require_endsentence ();
}

bool RunnerLine::do_def ()
{
        gettoken ();
        VarType type;
	switch (token.code)
	{
        case keyFN:
		return do_def_fn ();
	case keySTR:
	case keyINT:
	case keyREAL:
		type= token.code == keySTR ? VarString :
			token.code == keyINT ? VarInteger : VarNumber;
		#if 0
		do {
			expecttoken (keyIDENTIFIER);
			char c= get_letter (token.str);
			gettoken ();
			if (token.code == '-')
			{
				expecttoken (keyIDENTIFIER);
				char c2= get_letter (token.str);
				definevar (type, c, c2);
				gettoken ();
			}
			else
				definevar (type, c);
		} while (token.code == ',');
		require_endsentence ();
		#else
		definevars (type);
		#endif
		return false;
	default:
		throw ErrSyntax;
	}
}

bool RunnerLine::do_fn ()
{
	expecttoken (keyEND);
	errorifparam ();
	runner.setstatus (ProgramEnded);
	return true;
}

bool RunnerLine::do_programarg_s ()
{
	std::vector <std::string> args;
	gettoken ();
	if (! endsentence () )
	{
		for (;;)
		{
			std::string par= evalstring ();
			args.push_back (par);
			if (endsentence () )
				break;
			requiretoken (',');
			gettoken ();
		}
	}
	setprogramargs (args);
	return false;
}

bool RunnerLine::do_erase ()
{
        for (;;)
        {
                expecttoken (keyIDENTIFIER);
                std::string str (token.str);
                switch (typeofvar (str) )
                {
                case VarNumber:
                        erasevarnumber (str);
                        break;
		case VarInteger:
			erasevarinteger (str);
			break;
                case VarString:
                        erasevarstring (str);
                        break;
                default:
                        throw ErrBlassicInternal;
                }
                gettoken ();
                if (endsentence () )
                        break;
                requiretoken (',');
        }
        return false;
}

bool RunnerLine::do_swap ()
{
	expecttoken (keyIDENTIFIER);
	std::string strvar1= token.str;
	gettoken ();
	Dimension dim1;
	bool isarray1= false;
	if (token.code == '(')
	{
		dim1= expectdims ();
		isarray1= true;
	}
	requiretoken (',');
	gettoken ();
	requiretoken (keyIDENTIFIER);
	std::string strvar2= token.str;
	gettoken ();
	Dimension dim2;
	bool isarray2= false;
	if (token.code == '(')
	{
		dim2= expectdims ();
		isarray2= true;
	}
	require_endsentence ();
	VarType type= typeofvar (strvar1);
	if (type != typeofvar (strvar2) )
		throw ErrMismatch;
	switch (type)
	{
	case VarNumber:
		{
			BlNumber * pbn1= isarray1 ?
				addrdimnumber (strvar1, dim1) :
				addrvarnumber (strvar1);
			BlNumber * pbn2= isarray2 ?
				addrdimnumber (strvar2, dim2) :
				addrvarnumber (strvar2);
			std::swap (* pbn1, * pbn2);
		}
		break;
        case VarInteger:
                {
                        BlInteger * pbi1= isarray1 ?
                                addrdiminteger (strvar1, dim1) :
                                addrvarinteger (strvar1);
                        BlInteger * pbi2= isarray2 ?
                                addrdiminteger (strvar2, dim2) :
                                addrvarinteger (strvar2);
                        std::swap ( * pbi1, * pbi2);
                }
                break;
	case VarString:
		{
			std::string * pstr1= isarray1 ?
				addrdimstring (strvar1, dim1) :
				addrvarstring (strvar1);
			std::string * pstr2= isarray2 ?
				addrdimstring (strvar2, dim2) :
				addrvarstring (strvar2);
			std::swap (* pstr1, * pstr2);
		}
		break;
	default:
		throw ErrBlassicInternal;
	}
	return false;
}

bool RunnerLine::do_symbol ()
{
	gettoken ();
	if (token.code == keyAFTER)
	{
		BlInteger n= expectinteger ();
		require_endsentence ();
		graphics::symbolafter (static_cast <int> (n) );
		return false;
	}
	BlNumber bnSymbol= evalnum ();

	unsigned char byte [8];
	for (int i= 0; i < 8; ++i)
		byte [i]= 0;

	for (int i= 0; i < 8; ++i)
	{
		if (token.code != ',')
		{
			// Parameters can be omitted
			break;
		}
		BlNumber bnByte= expectnum ();
		byte [i]= static_cast <unsigned char> (bnByte);
	}
	require_endsentence ();
	graphics::definesymbol ( int (bnSymbol), byte);
	return false;
}

bool RunnerLine::do_zone ()
{
	throw ErrNotImplemented;
}

bool RunnerLine::do_pop ()
{
	errorifparam ();
	ProgramPos notused;
	runner.gosub_pop (notused);
	return false;
}

bool RunnerLine::do_name ()
{
	std::string strOrig= expectstring ();
	requiretoken (keyAS);
	std::string strDest= expectstring ();
	require_endsentence ();
	if (std::rename (strOrig.c_str (), strDest.c_str () ) != 0)
	{
		switch (errno)
		{
		case ENOENT:
			throw ErrFileNotFound;
		default:
			throw ErrRenameFile;
		}
	}
	return false;
}

bool RunnerLine::do_kill ()
{
	std::string strFile= expectstring ();
	require_endsentence ();
	if (unlink (strFile.c_str () ) != 0)
	{
		switch (errno)
		{
		case ENOENT:
			throw ErrFileNotFound;
		default:
			throw ErrOperatingSystem;
		}
	}
	return false;
}

bool RunnerLine::do_files ()
{
        std::string param;
        gettoken ();
        if (! endsentence () )
                param= evalstring ();
	require_endsentence ();
        if (param.empty () )
                param= "*";

        std::vector <std::string> file;

	// Populate the vector with the files searched.
	Directory d;
	for (std::string r= d.findfirst (param.c_str () ); ! r.empty ();
			r= d.findnext () )
		file.push_back (r);

        const size_t l= file.size ();
        size_t maxlength= 0;
        for (size_t i= 0; i < l; ++i)
                maxlength= std::max (maxlength, file [i].size () );
        ++maxlength;
        size_t width= getwidth ();
	size_t cols= width / maxlength;
	BlFile & bf= getfile (0);
        if (cols <= 1)
        {
                for (size_t i= 0; i < l; ++i)
                        bf << file [i] << '\n';
        }
        else
        {
                const size_t widthcol= width / cols;
                for (size_t i= 0; i < l; ++i)
                {
                        const std::string & str= file [i];
                        bf << file [i];
                        if (i % cols == cols - 1)
                                bf << '\n';
                        else
                                bf << std::string
                                	(widthcol - str.size (), ' ');
                }
                if ( l > 0 && l % cols != 0)
                        bf << '\n';
        }
	return false;
}

bool RunnerLine::do_paper ()
{
	gettoken ();
	BlChannel ch= 0;
	if (token.code == '#')
	{
		ch= expectchannel ();
		requiretoken (',');
		gettoken ();
	}
	BlNumber color= evalnum ();
	require_endsentence ();
	#if 0
        if (graphics::ingraphicsmode () )
	        graphics::setbackground (int (color) );
        else
                textbackground (int (color) );
	#else
	BlFile & out= getfile (ch);
	out.setbackground (int (color) );
	#endif
	return false;
}

bool RunnerLine::do_pen ()
{
	gettoken ();
	BlChannel ch= 0;
	if (token.code == '#')
	{
		ch= expectchannel ();
		requiretoken (',');
		gettoken ();
	}
	BlFile & out= getfile (ch);
	if (token.code != ',')
	{
		// All parameters can't be omitted
		BlNumber color= evalnum ();
		#if 0
                if (graphics::ingraphicsmode () )
		        graphics::setcolor (int (color) );
                else
                        textcolor (int (color) );
		#else
		out.setcolor (int (color) );
		#endif
		if (token.code != ',')
		{
			require_endsentence ();
			return false;
		}
	}
	gettoken ();
        if (token.code != ',')
        {
	        BlNumber bgmode= evalnum ();
	        graphics::settransparent (int (bgmode) );
                if (token.code != ',')
                {
                        require_endsentence ();
                        return false;
                }
        }
        BlNumber mode= expectnum ();
        graphics::setdrawmode (int (mode) );
	require_endsentence ();
	return false;
}

bool RunnerLine::do_shell ()
{
	gettoken ();
	if (endsentence () )
		throw ErrNotImplemented;
	std::string command= evalstring ();
	require_endsentence ();
	int r= system (command.c_str () );
	std::cerr << "Result: " << r << endl;
	if (r == -1)
		throw ErrOperatingSystem;
	sysvar::set (sysvar::ShellResult, char (r >> 8) );
	return false;
}

bool RunnerLine::do_merge ()
{
	std::string progname= expectstring ();
	require_endsentence ();
	program.merge (progname);
	runner.setstatus (ProgramEnded);
	return true;
}

bool RunnerLine::do_chdir ()
{
	std::string dirname= expectstring ();
	require_endsentence ();
	if (chdir (dirname.c_str () ) != 0)
		throw ErrOperatingSystem;
	return false;
}

bool RunnerLine::do_mkdir ()
{
	std::string dirname= expectstring ();
	require_endsentence ();
        #ifdef _Windows
        int r= mkdir (dirname.c_str () );
        #else
	int r= mkdir (dirname.c_str (), 0777);
        #endif
        if (r != 0)
		throw ErrOperatingSystem;
	return false;
}

bool RunnerLine::do_rmdir ()
{
	std::string dirname= expectstring ();
	require_endsentence ();
	if (rmdir (dirname.c_str () ) != 0)
		throw ErrOperatingSystem;
	return false;
}

bool RunnerLine::do_synchronize ()
{
	gettoken ();
	if (endsentence () )
		graphics::synchronize ();
	else
	{
		BlNumber n= evalnum ();
		graphics::synchronize (n != 0);
	}
	return false;
}

bool RunnerLine::do_pause ()
{
	BlNumber bln= expectnum ();
	require_endsentence ();
	unsigned long n= static_cast <unsigned long> (bln);
	#ifdef __WIN32__
	Sleep (static_cast <DWORD> (n) );
	#else
	n*= 1000;
	unsigned int sec= n / 1000000;
	n%= 1000000;
	if (sec != 0)
		sleep (sec);
	usleep (n);
	#endif
	return false;
}

bool RunnerLine::do_chain ()
{
	gettoken ();
	bool merging= false;
	if (token.code == keyMERGE)
	{
		merging= true;
		gettoken ();
	}
	std::string progname= evalstring ();
	BlLineNumber iniline= 0;
	if (! endsentence () )
	{
		requiretoken (',');
		gettoken ();
		if (token.code != keyDELETE)
		{
			iniline= evallinenumber ();
			if (! endsentence () )
			{
				requiretoken (',');
				expecttoken (keyDELETE);
			}
		}
		if (token.code == keyDELETE)
		{
			if (! merging)
				throw ErrSyntax;
			gettoken ();
			BlLineNumber ini, end;
			evallinerange (ini, end);
			program.deletelines (ini, end);
		}
		require_endsentence ();
	}
	if (merging)
		program.merge (progname);
	else
		program.load (progname);
	runner.run_to (iniline);
	return true;
}

bool RunnerLine::do_environ ()
{
        BlResult result;
        expect (result);
	const std::string & str= result.str ();
	size_t l= str.size ();
	// We use an auto alloc, then in case of error the memory is freed.
	util::auto_alloc <char> envstr (l + 1);
	memcpy (envstr, str.data (), l);
	envstr [l]= 0;
	if (putenv (envstr) != 0)
		throw ErrFunctionCall;
	// Do not free the string now, is part of the environment.
	envstr.release ();
	return false;
}

bool RunnerLine::do_edit ()
{
	gettoken ();
	if (token.code != keyNUMBER && token.code != keyINTEGER)
		throw ErrSyntax;
	BlLineNumber dest= evallinenumber ();
	require_endsentence ();

	#if 0

	std::string buffer;
	{
		BlFileOutString bfos;
		program.list (dest, dest, bfos);
		buffer= bfos.str ();
		if (buffer.empty () )
		{
			bfos << dest << " \n";
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

	if (editline (getfile (0), buffer, inipos) )
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

	#else

	editline (getfile (0), program, dest);

	#endif

	return false;
}

bool RunnerLine::do_drawr ()
{
	#if 0
	BlNumber x= expectnum ();
	requiretoken (',');
	BlNumber y= expectnum ();
	require_endsentence ();
	graphics::liner (int (x), int (y) );
	#else
	BlInteger x, y;
	getdrawargs (x, y);
	graphics::liner (x, y);
	#endif
	return false;
}

bool RunnerLine::do_plotr ()
{
	#if 0
	BlNumber x= expectnum ();
	requiretoken (',');
	BlNumber y= expectnum ();
	require_endsentence ();
	graphics::plotr (int (x), int (y) );
	#else
	BlInteger x, y;
	getdrawargs (x, y);
	graphics::plotr (x, y);
	#endif
	return false;
}

bool RunnerLine::do_mover ()
{
	#if 0
	BlNumber x= expectnum ();
	requiretoken (',');
	BlNumber y= expectnum ();
	require_endsentence ();
	graphics::mover (int (x), int (y) );
	#else
	BlInteger x, y;
	getdrawargs (x, y);
	graphics::mover (x, y);
	#endif
	return false;
}

bool RunnerLine::do_poke16 ()
{
	BlNumber bnAddr= expectnum ();
	requiretoken (',');
	BlChar * addr= (BlChar *) (unsigned int) bnAddr;
	BlNumber bnValue= expectnum ();
	require_endsentence ();
	unsigned short value= (unsigned short) bnValue;
	poke16 (addr, value);
	return false;
}

bool RunnerLine::do_poke32 ()
{
	BlNumber bnAddr= expectnum ();
	requiretoken (',');
	BlChar * addr= (BlChar *) (unsigned int) bnAddr;
	BlNumber bnValue= expectnum ();
	require_endsentence ();
	BlInteger value= BlInteger (bnValue);
	poke32 (addr, value);
	return false;
}

bool RunnerLine::do_renum ()
{
	BlResult r;
	gettoken ();
	BlLineNumber newnumber= 10, oldnumber= 0, increment= 10;
	if (! endsentence () && token.code != ',')
		newnumber= evallinenumber ();
	if (! endsentence () )
	{
		requiretoken (',');
		gettoken ();
		if (token.code != ',')
			oldnumber= evallinenumber ();
		if (! endsentence () )
		{
			requiretoken (',');
			gettoken ();
			increment= evallinenumber ();
		}
	}
	require_endsentence ();
	program.renum (newnumber, oldnumber, increment);
	runner.setstatus (ProgramEnded);
	return true;
}

bool RunnerLine::do_circle ()
{
	gettoken ();
	requiretoken ('(');
	BlResult r;
	expect (r);
	BlInteger x= r.integer ();
	requiretoken (',');
	expect (r);
	BlInteger y= r.integer ();
	requiretoken (')');
	expecttoken (',');
	expect (r);
	BlInteger radius= r.integer ();
	BlNumber arcbeg= 0, arcend= 0;
	bool fArc= false;
	bool fElliptic= false;
	BlNumber elliptic= 0; // initialized to avoid a warning.
	if (endsentence () )
		goto do_it;
	requiretoken (',');
	gettoken ();
	if (token.code != ',')
	{
		BlInteger color= evalinteger ();
		graphics::setcolor (color);
		if (endsentence () )
			goto do_it;
		requiretoken (',');
	}
	gettoken ();
	if (token.code != ',')
	{
		arcbeg= evalnum ();
		fArc= true;
		if (endsentence () )
			goto do_it;
		requiretoken (',');
	}
	gettoken ();
	if (token.code != ',')
	{
		arcend= evalnum ();
		fArc= true;
		if (endsentence () )
			goto do_it;
		requiretoken (',');
	}
	gettoken ();
	elliptic= evalnum ();
	fElliptic= true;
	require_endsentence ();
do_it:
	if (! fElliptic)
	{
		if (! fArc)
			graphics::circle (x, y, radius);
		else
			graphics::arccircle (x, y, radius, arcbeg, arcend);
	}
	else
	{
		int rx, ry;
		if (elliptic > 1)
		{
			rx= static_cast <int> (radius / elliptic);
			ry= radius;
		}
		else
		{
			rx= radius;
			ry= static_cast <int> (radius * elliptic);
		}
		if (! fArc)
			graphics::ellipse (x, y, rx, ry);
		else
			graphics::arcellipse (x, y, rx, ry, arcbeg, arcend);
	}
	return false;
}

bool RunnerLine::do_mask ()
{
	BlResult r;
	gettoken ();
	if (token.code != ',')
	{
		eval (r);
		graphics::mask (r.integer () );
	}
	if (! endsentence () )
	{
		requiretoken (',');
		expect (r);
		graphics::maskdrawfirstpoint (r.integer () );
	}
	require_endsentence ();
	return false;
}

bool RunnerLine::do_window ()
{
	gettoken ();
	if (token.code == keySWAP)
	{
		BlChannel ch1= expectchannel ();
		requiretoken (',');
		BlChannel ch2= expectchannel ();
		require_endsentence ();
		runner.windowswap (ch1, ch2);
		return false;
	}
	BlChannel ch= 0;
	if (token.code == '#')
	{
		ch= expectchannel ();
		requiretoken (',');
		gettoken ();
	}
	BlInteger x1= evalinteger ();
	requiretoken (',');
	BlInteger x2= expectinteger ();
	requiretoken (',');
	BlInteger y1= expectinteger ();
	requiretoken (',');
	BlInteger y2= expectinteger ();
	require_endsentence ();
	if (runner.isfileopen (ch) )
	{
		BlFile & bf= runner.getfile (ch);
		BlFileWindow * pw= dynamic_cast <BlFileWindow *>
			(& bf);
		if (pw != NULL)
		{
			pw->reset (x1, x2, y1, y2);
			return false;
		}
	}
	#if 0
	// We need to close first because if it is already a window
	// it will be deleted after the creation resulting in the
	// window become destroyed.
	runner.closechannel (ch);
	#endif
	runner.setfile (ch, new BlFileWindow (ch, x1, x2, y1, y2) );
	return false;
}

void RunnerLine::do_graphics_pen ()
{
	gettoken ();
	if (token.code != ',')
	{
		BlInteger ink= evalinteger ();
		graphics::setcolor (ink);
		if (endsentence () )
			return;
		requiretoken (',');
	}
	BlInteger transpmode= expectinteger ();
	graphics::settransparent (transpmode);
	require_endsentence ();
}

void  RunnerLine::do_graphics_paper ()
{
	BlInteger ink= expectinteger ();
	require_endsentence ();
	graphics::setbackground (ink);
}

void  RunnerLine::do_graphics_cls ()
{
	gettoken ();
	require_endsentence ();
	graphics::cls ();
}

bool  RunnerLine::do_graphics ()
{
	gettoken ();
	switch (token.code)
	{
	case keyPEN:
		do_graphics_pen ();
		break;
	case keyPAPER:
		do_graphics_paper ();
		break;
	case keyCLS:
		do_graphics_cls ();
		break;
	default:
		throw ErrSyntax;
	}
	return false;
}

bool RunnerLine::do_beep ()
{
	gettoken ();
	require_endsentence ();
	ring ();
	return false;
}

bool RunnerLine::do_defint ()
{
	VarType type;
	switch (token.code)
	{
	case keyDEFINT:
		type= VarInteger; break;
	case keyDEFSTR:
		type= VarString; break;
	case keyDEFREAL: case keyDEFSNG: case keyDEFDBL:
		type= VarNumber; break;
	default:
		throw ErrBlassicInternal;
	}
	definevars (type);
	return false;
}

bool RunnerLine::do_ink ()
{
	int inknum= expectinteger ();
	requiretoken (',');
	int r= expectinteger ();
	if (endsentence () )
	{
		graphics::ink (inknum, r);
		return false;
	}
	int g= expectinteger ();
	if (endsentence () )
	{
		// Flashing ink in Amstrad CPC,
		// just ignore second parameter.
		graphics::ink (inknum, r);
		return false;
	}
	int b= expectinteger ();
	require_endsentence ();
	graphics::ink (inknum, r, g, b);
	return false;
}

void RunnerLine::execute ()
{
        bool more= true;
	//runner.setposactual (ProgramPos (line.number () ) );
	//runner.setposactual (ProgramPos (pline->number () ) );
        do {
		#if 0
		graphics::idle ();
		#else
		// We avoid calling idle too many times.
		if (graphics::ingraphicsmode () )
		{
			static size_t counter= 0;
			if (counter ++ == 100)
			{
				graphics::idle ();
				counter= 0;
			}
		}
		#endif

		//codprev= line.actualcode ();
		codprev= pline->actualcode ();
		if (codprev == keyELSE)
		{
			if (fInElse)
				fInElse= false;
			else
				break;
		}
		gettoken ();
		//runner.setchunkactual (line.chunk () );
		//runner.setchunkactual (pline->chunk () );
		actualchunk= pline->chunk ();
		// else can be preceded by ":"
		if (token.code == keyELSE)
		{
			if (fInElse)
				fInElse= false;
			else
				break;
		}
		if (fInterrupted)
		{
			if (runner.getbreakstate () != Runner::BreakCont)
				throw BlBreak ();
			fInterrupted= false;
		}
		// Homenaje a Intercal.
		if (token.code == keyPLEASE)
		{
			gettoken ();
			if (endsentence () )
				throw ErrPolite;
		}
		if (token.code == keyENDLINE)
			more= false;
		else
		{
			mapfunc_t::const_iterator it=
				mapfunc.find (token.code);
			//if (it == mapfunc.end () )
			if (it == mapend)
				throw ErrSyntax;
			if ( (this->*it->second) ())
				more= false;
		}
	} while (more);
}

// Fin de runnerline.cpp
