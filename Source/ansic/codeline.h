//	codeline.h

#ifndef CODELINE_H_
#define CODELINE_H_

#include "blassic.h"
#include "keyword.h"
#include "var.h"

class Program;

class CodeLine {
	BlChar * strcontent;
	BlLineNumber linenumber;
	BlLineLength len;
	bool owner;
	BlLineLength pos;
	BlChunk chk;
	BlCode lastcode;
public:
	class Token {
	public:
		BlCode code;
		std::string str;
		BlInteger valueint;
                static BlNumber number (const std::string & str);
		BlNumber number () const;
		BlInteger integer () const { return valueint; }
		inline bool isnumeric ()
		{
			return isnumberfunction (code) || code == keyNUMBER ||
				code == keyINTEGER ||
				(code == keyIDENTIFIER &&
					typeofvar (str) == VarNumber);
		}
		inline bool isendsentence () const
		{
			return code == ':' ||
				code == keyENDLINE || 
				code == keyELSE;
		}
	};

	CodeLine ();
	CodeLine (BlChar * str, BlLineNumber number, BlLineLength length);
	CodeLine (const CodeLine & old);
	~CodeLine ();
	void assign (BlChar * str, BlLineNumber number, BlLineLength length);
	CodeLine & operator= (const CodeLine & old);
	bool empty () const { return len == 0; }
	BlLineNumber number () const { return linenumber; }
	void setnumber (BlLineNumber n) { linenumber= n; }
	BlLineLength length () const { return len; }
	BlChunk chunk () const { return chk; }
	BlChar * content () { return strcontent; }
	const BlChar * content () const { return strcontent; }
	BlCode actualcode () const { return lastcode; }
        Token getdata ();
	//Token gettoken ();
	void gettoken (Token & r);
	void gotochunk (BlChunk chknew);
	void scan (const std::string & line);
	void execute (Program & program);
};

#endif

// Fin de codeline.h
