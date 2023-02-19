// keyword.cpp
// Revision 16-may-2003

#include "keyword.h"
#include "util.h"
#include "trace.h"

using util::dim_array;

#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cctype>

// I don't understand why, but with this using the older version
// of C++ Builder fails.
#if __BORLANDC__ >= 0x0560
using std::toupper;
#endif

namespace {

struct keycode {
	std::string key;
	BlCode code;
	keycode (const char * str, BlCode c) :
		key (str), code (c)
	{ }
};

// Can't declare const on Borland or fail at instantiate find_if,
// don't know why.
#ifdef __BORLANDC__
#define const_keycode keycode
#else
#define const_keycode const keycode
#endif

const_keycode table []= {
	keycode ("END",               keyEND),
	keycode ("LIST",              keyLIST),
	keycode ("REM",               keyREM),
	keycode ("LOAD",              keyLOAD),
	keycode ("SAVE",              keySAVE),
        keycode ("EXIT",              keyEXIT),
	keycode ("NEW",               keyNEW),
	keycode ("RUN",               keyRUN),
	keycode ("PRINT",             keyPRINT),
	keycode ("FOR",               keyFOR),
	keycode ("NEXT",              keyNEXT),
	keycode ("TO",                keyTO),
	keycode ("STEP",              keySTEP),
	keycode ("IF",                keyIF),
	keycode ("THEN",              keyTHEN),
	keycode ("ELSE",              keyELSE),
	keycode ("TRON",              keyTRON),
	keycode ("TROFF",             keyTROFF),
	keycode ("LET",               keyLET),
	keycode ("GOTO",              keyGOTO),
	keycode ("STOP",              keySTOP),
	keycode ("CONT",              keyCONT),
	keycode ("CLEAR",             keyCLEAR),
	keycode ("GOSUB",             keyGOSUB),
	keycode ("RETURN",            keyRETURN),
	keycode ("POKE",              keyPOKE),
	keycode ("DATA",              keyDATA),
	keycode ("READ",              keyREAD),
	keycode ("RESTORE",           keyRESTORE),
	keycode ("INPUT",             keyINPUT),
	keycode ("LINE",              keyLINE),
	keycode ("RANDOMIZE",         keyRANDOMIZE),
	keycode ("PLEASE",            keyPLEASE),
	keycode ("AUTO",              keyAUTO),
	keycode ("DIM",               keyDIM),
	keycode ("SYSTEM",            keySYSTEM),
        keycode ("ON",                keyON),
        keycode ("ERROR",             keyERROR),
        keycode ("OPEN",              keyOPEN),
        keycode ("CLOSE",             keyCLOSE),
        keycode ("OUTPUT",            keyOUTPUT),
        keycode ("AS",                keyAS),
	keycode ("LOCATE",            keyLOCATE),
	keycode ("CLS",               keyCLS),
	keycode ("APPEND",            keyAPPEND),
	keycode ("WRITE",             keyWRITE),
	keycode ("MODE",              keyMODE),
	keycode ("MOVE",              keyMOVE),
	keycode ("COLOR",             keyCOLOR),
        keycode ("GET",               keyGET),
	keycode ("LABEL",             keyLABEL),
	keycode ("DELIMITER",         keyDELIMITER),
	keycode ("REPEAT",            keyREPEAT),
	keycode ("UNTIL",             keyUNTIL),
        keycode ("WHILE",             keyWHILE),
        keycode ("WEND",              keyWEND),
	keycode ("PLOT",              keyPLOT),
        keycode ("POPEN",             keyPOPEN),
        keycode ("RESUME",            keyRESUME),
	keycode ("DELETE",            keyDELETE),
	keycode ("LOCAL",             keyLOCAL),
	keycode ("RANDOM",            keyRANDOM),
	keycode ("PUT",               keyPUT),
        keycode ("FIELD",             keyFIELD),
        keycode ("LSET",              keyLSET),
        keycode ("RSET",              keyRSET),
	keycode ("SOCKET",            keySOCKET),
	keycode ("DRAW",              keyDRAW),
	keycode ("DEF",               keyDEF),
	keycode ("FN",                keyFN),
        keycode ("ERASE",             keyERASE),
        keycode ("SWAP",              keySWAP),
        keycode ("SYMBOL",            keySYMBOL),
        keycode ("ZONE",              keyZONE),
        keycode ("POP",               keyPOP),
	keycode ("NAME",              keyNAME),
	keycode ("KILL",              keyKILL),
	keycode ("FILES",             keyFILES),
	keycode ("PAPER",             keyPAPER),
	keycode ("PEN",               keyPEN),
	keycode ("SHELL",             keySHELL),
	keycode ("MERGE",             keyMERGE),
	keycode ("CHDIR",             keyCHDIR),
	keycode ("MKDIR",             keyMKDIR),
	keycode ("RMDIR",             keyRMDIR),
	keycode ("BREAK",             keyBREAK),
	keycode ("SYNCHRONIZE",       keySYNCHRONIZE),
	keycode ("PAUSE",             keyPAUSE),
	keycode ("CHAIN",             keyCHAIN),
	keycode ("STR",               keySTR),
	keycode ("REAL",              keyREAL),
	keycode ("ENVIRON",           keyENVIRON),
	keycode ("EDIT",              keyEDIT),
	keycode ("DRAWR",             keyDRAWR),
	keycode ("PLOTR",             keyPLOTR),
	keycode ("MOVER",             keyMOVER),
	keycode ("POKE16",            keyPOKE16),
	keycode ("POKE32",            keyPOKE32),
	keycode ("RENUM",             keyRENUM),
	keycode ("CIRCLE",            keyCIRCLE),
	keycode ("MASK",              keyMASK),
	keycode ("WINDOW",            keyWINDOW),
	keycode ("GRAPHICS",          keyGRAPHICS),
	keycode ("AFTER",             keyAFTER),

	keycode ("MID$",              keyMID_S),
	keycode ("LEFT$",             keyLEFT_S),
	keycode ("RIGHT$",            keyRIGHT_S),
	keycode ("CHR$",              keyCHR_S),
	keycode ("ENVIRON$",          keyENVIRON_S),
	keycode ("STRING$",           keySTRING_S),
	keycode ("OSFAMILY$",         keyOSFAMILY_S),
	keycode ("HEX$",              keyHEX_S),
        keycode ("SPACE$",            keySPACE_S),
        keycode ("UPPER$",            keyUPPER_S),
        keycode ("LOWER$",            keyLOWER_S),
        keycode ("STR$",              keySTR_S),
        keycode ("OCT$",              keyOCT_S),
        keycode ("BIN$",              keyBIN_S),
        keycode ("INKEY$",            keyINKEY_S),
	keycode ("PROGRAMARG$",       keyPROGRAMARG_S),
	keycode ("DATE$",             keyDATE_S),
	keycode ("TIME$",             keyTIME_S),
	keycode ("INPUT$",            keyINPUT_S),
	keycode ("MKI$",              keyMKI_S),
	keycode ("MKS$",              keyMKS_S),
	keycode ("MKD$",              keyMKD_S),
	keycode ("MKL$",              keyMKL_S),
	keycode ("TRIM$",             keyTRIM_S),
	keycode ("LTRIM$",            keyLTRIM_S),
	keycode ("RTRIM$",            keyRTRIM_S),
	keycode ("OSNAME$",           keyOSNAME_S),

        keycode ("ASC",               keyASC),
	keycode ("LEN",               keyLEN),
	keycode ("PEEK",              keyPEEK),
	keycode ("PROGRAMPTR",        keyPROGRAMPTR),
	keycode ("RND",               keyRND),
	keycode ("INT",               keyINT),
	keycode ("SIN",               keySIN),
	keycode ("COS",               keyCOS),
	keycode ("PI",                keyPI),
	keycode ("TAN",               keyTAN),
	keycode ("SQR",               keySQR),
	keycode ("ASIN",              keyASIN),
	keycode ("ACOS",              keyACOS),
	keycode ("INSTR",             keyINSTR),
	keycode ("ATAN",              keyATAN),
	keycode ("ABS",               keyABS),
	keycode ("USR",               keyUSR),
        keycode ("VAL",               keyVAL),
	keycode ("EOF",               keyEOF),
	keycode ("VARPTR",            keyVARPTR),
	keycode ("SYSVARPTR",         keySYSVARPTR),
	keycode ("SGN",               keySGN),
	keycode ("LOG",               keyLOG),
	keycode ("LOG10",             keyLOG10),
	keycode ("EXP",               keyEXP),
	keycode ("TIME",              keyTIME),
        keycode ("ERR",               keyERR),
        keycode ("ERL",               keyERL),
        keycode ("CVI",               keyCVI),
        keycode ("CVS",               keyCVS),
        keycode ("CVD",               keyCVD),
        keycode ("CVL",               keyCVL),
        keycode ("MIN",               keyMIN),
        keycode ("MAX",               keyMAX),
	keycode ("CINT",              keyCINT),
	keycode ("FIX",               keyFIX),
	keycode ("XMOUSE",            keyXMOUSE),
	keycode ("YMOUSE",            keyYMOUSE),
	keycode ("XPOS",              keyXPOS),
	keycode ("YPOS",              keyYPOS),
	keycode ("PEEK16",            keyPEEK16),
	keycode ("PEEK32",            keyPEEK32),
	keycode ("RINSTR",            keyRINSTR),
	keycode ("FIND_FIRST_OF",     keyFIND_FIRST_OF),
	keycode ("FIND_LAST_OF",      keyFIND_LAST_OF),
	keycode ("FIND_FIRST_NOT_OF", keyFIND_FIRST_NOT_OF),
	keycode ("FIND_LAST_NOT_OF",  keyFIND_LAST_NOT_OF),

	keycode ("NOT",               keyNOT),
	keycode ("OR",                keyOR),
	keycode ("AND",               keyAND),
        keycode ("TAB",               keyTAB),
        keycode ("SPC",               keySPC),
        keycode ("AT",                keyAT),
	keycode ("XOR",               keyXOR),
	keycode ("MOD",               keyMOD),
	keycode ("USING",             keyUSING),

	// table_end points here, then if find_if (table, table_end, ...)
	// fails the result is:
	keycode ("???",         0),
};

const_keycode * table_end= table + dim_array (table) - 1;

class key_is : public std::unary_function <keycode, bool> {
public:
	key_is (const std::string & str) : str (str)
	{ }
	bool operator () (const keycode & k) const
	{ return k.key == str; }
private:
	const std::string & str;
};

class code_is : public std::unary_function <keycode, bool> {
public:
	code_is (BlCode code) : code (code)
	{ }
	bool operator () (const keycode & k) const
	{ return k.code == code; }
private:
	BlCode code;
};

inline std::string stringupper (const std::string & str)
{
	std::string u (str.size (), 0);
	std::transform (str.begin (), str.end (), u.begin (), toupper);
	return u;
}

std::set <std::string> exclude;

} // namespace

void excludekeyword (const std::string & str)
{
	TraceFunc tr ("excludekeyword");

	std::string stru= stringupper (str);
	if (find_if (table, table_end, key_is (stru) ) != table_end)
	{
		exclude.insert (stru);
		tr.message (std::string ("Excluding ") + stru);
	}
}

BlCode keyword (const std::string & str)
{
	#if 0
	#if __GNUC__ == 2 && __GNUC_MINOR__ <= 95
	// Apparently before gcc 2.95.3 there is no push_back in
	// string, but there is no method to evaluate te third number,
	// then we disallow it in all 2.95 versions.
	std::string stru (str.size (), 0);
	std::transform (str.begin (), str.end (), stru.begin (), toupper);
	#else
	std::string stru;
	std::transform (str.begin (), str.end (),
		std::back_inserter (stru), toupper);
	#endif
	#else
	std::string stru= stringupper (str);
	#endif
	BlCode code= std::find_if (table, table_end, key_is (stru) )->code;
	if (code != 0)
		if (exclude.find (stru) != exclude.end () )
			return 0;
	return code;
}

std::string decodekeyword (BlCode s)
{
	return std::find_if (table, table_end, code_is (s) )->key;
}

// Fin de keyword.cpp
