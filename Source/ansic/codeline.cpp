//	codeline.cpp

//#include "codeline.h"
#include "program.h"
#include "keyword.h"
#include "token.h"
#include "var.h"
#include "error.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <cctype>
using std::isxdigit;

#include <iostream>
using std::cerr;
using std::endl;

namespace {

inline bool isbindigit (char c)
{
        return c == '0' || c == '1';
}

inline bool isoctdigit (char c)
{
        return c >= '0' && c <= '7';
}

inline bool isbeginidentifier (char c)
{
	return isalpha (c) /* || c == '_' */;
}

inline bool isidentifier (char c)
{
	return isalnum (c) || c == '_';
}

} // namespace

BlNumber CodeLine::Token::number (const std::string & str)
{
	size_t i= 0, l= str.size ();
	if (i >= l) return 0;
	BlNumber n= 0;
	unsigned char c;
	if (str [0] == '&')
	{
                if (l > 1)
                {
			long num= 0;
                        c= str [1];
                        if (c == 'X' || c == 'x')
                        {
			// Binary
                                i= 1;
                                while (++i < l && isbindigit (c= str [i]) )
                                {
                                        num*= 2;
                                        num+= c- '0';
                                }
                        }
                        else if (c == 'O' || c == 'o')
                        {
			// Octal
                                i= 1;
                                while (++i < l && isoctdigit (c= str [i]) )
                                {
                                        num*= 8;
                                        num+= c- '0';
                                }
                        }
                        else
                        {
                        // Hexadecimal
                                if (c == 'H' || c == 'h')
                                        i= 1;
                                while (++i < l && isxdigit (c= str [i] ) )
                                {
                                        num*= 16;
                                        if (c >= 'a' && c <= 'f')
                                                c= (unsigned char) (c + '9' - 'a' + 1);
                                        else if (c >= 'A' && c <= 'F')
                                                c= (unsigned char) (c + '9' - 'A' + 1);
                                        num+= c - '0';
                                }
                        }
			n= num;
                }
	}
	else {
		// Decimal
		#if 0
		while (i < l && isdigit (c= str [i]) )
		{
			n*= 10;
			n+= c - '0';
			++i;
		}
		if (i < l && str [i] == '.')
		{
			++i;
			BlNumber mult= 0.1;
			while (i < l && isdigit (c= str [i] ) )
			{
				n+= (c - '0') * mult;
				mult/= 10;
				++i;
			}
		}
		if (i < l && (str [i] == 'E' || str [i] == 'e') )
		{
			++i;
			BlNumber e= 0;
			bool neg= false;
			if (str [i] == '-')
			{
				neg= true;
				++i;
			}
			while (i < l && (isdigit (c= str [i] ) ) )
			{
				e*= 10;
				e+= c - '0';
				++i;
			}
			if (neg) e= -e;
			n*= pow (10, e);
		}
		#else
		std::istringstream iss (str);
		iss >> n;
		#endif
	}
	return n;
}

BlNumber CodeLine::Token::number () const
{
        switch (code)
        {
        case keyNUMBER:
        case keySTRING:
                return number (str);
        case keyINTEGER:
                return valueint;
        default:
        	cerr << "Codeline::Token::number called but code= " <<
        		code << " is not valid." << endl;
                throw ErrBlassicInternal;
        }
}

CodeLine::CodeLine () :
	strcontent (0),
	owner (false),
	pos (0),
	chk (0),
	lastcode (0)
{
}

CodeLine::CodeLine (BlChar * str, BlLineNumber number, BlLineLength length) :
	strcontent (str),
	linenumber (number),
	len (length),
	owner (false),
	pos (0),
	chk (0),
	lastcode (0)
{
}

CodeLine::CodeLine (const CodeLine & old) :
	strcontent (old.strcontent),
	linenumber (old.linenumber),
	len (old.len),
	owner (false),
	pos (0),
	chk (0),
	lastcode (0)
{
}

CodeLine::~CodeLine ()
{
	if (owner && strcontent)
		delete [] strcontent;
}

void CodeLine::assign (BlChar * str, BlLineNumber number, BlLineLength length)
{
	if (owner)
		delete strcontent;
	strcontent= str;
	linenumber= number;
	len= length;
	owner= false;
	pos= 0;
	chk= 0;
	lastcode= 0;
}

CodeLine & CodeLine::operator= (const CodeLine & old)
{
	if (& old != this)
	{
		if (owner && strcontent)
			delete [] strcontent;
		strcontent= old.strcontent;
		linenumber= old.linenumber;
		len= old.len;
		owner= false;
		pos= 0;
		chk= 0;
		lastcode= 0;
	}
	return * this;
}

CodeLine::Token CodeLine::getdata ()
{
        while (pos < len && isspace (strcontent [pos] ) )
                ++pos;
        Token r;
        if (pos >= len)
        {
                r.code= lastcode= keyENDLINE;
                return r;
        }
        char c= strcontent [pos];
        if (c == '"')
        {
		++pos;
		while ( (c= strcontent [pos++]) != '\0')
			r.str+= c;
        }
        else if (c == INTEGER_PREFIX)
        {
                //BlInteger n;
                //n= * (BlInteger *) (strcontent + pos + 1);
                //r.valueint= n;
		r.valueint= peek32 (strcontent + pos + 1);
                r.code= keyINTEGER;
                pos+= 5;
                return r;
        }
        else
	{
                while (pos < len && (c= strcontent [pos]) != ',' && c != ':')
                {
                        r.str+= c;
                        ++pos;
                }
	}
        r.code= keySTRING;
        return r;
}

CodeLine::Token CodeLine::gettoken ()
{
	while (pos < len && isspace (strcontent [pos] ) )
		++pos;
	Token r;
	if (pos >= len)
	{
		r.code= lastcode= keyENDLINE;
		//Prueba
		++chk;
		return r;
	}
	BlChar c= strcontent [pos];
	if (iskey (c) )
	{
		r.code= strcontent [pos++];
		r.code<<= 8;
		r.code|= strcontent [pos++];
	}
	else if (isbeginidentifier (c) )
	{
		r.code= keyIDENTIFIER;
		do {
			r.str+= char (toupper (c) );
			++pos;
		} while (pos < len && isidentifier (c= strcontent [pos]) );
		if (pos < len && c == '$' || c == '%' || c == '!')
		{
			++pos;
			r.str+= c;
		}
	}
	else if (isdigit (c) || c == '.')
	{
		r.code= keyNUMBER;
		std::string strnum;
		while (pos < len && (isdigit (c= strcontent [pos]) ) )
		{
			strnum+= c;
			++pos;
		}
		if (pos < len && (c= strcontent [pos] ) == '.')
		{
			strnum+= '.';
			++pos;
			while (pos < len && (isdigit (c= strcontent [pos]) ) )
			{
				strnum+= c;
				++pos;
			}
		}
		if (pos < len && (c == 'E' || c == 'e') )
		{
			strnum+= c;
			++pos;
			if ( (c= strcontent [pos]) == '-' || c == '+')
			{
				strnum+= c;
				++pos;
			}
			while (pos < len && (isdigit (c= strcontent [pos] ) ) )
			{
				strnum+= c;
				++pos;
			}
		}
		r.str+= strnum;
	}
	else if (c == '&')
	{
		// Hexadecimal, octal or binary number.
		r.code= keyNUMBER;
		r.str+= '&';
                ++pos;
                if (pos < len)
                {
                        c= strcontent [pos];
                        if (c == 'X' || c == 'x')
                        {
				// Binary
                                ++pos;
                                r.str+= c;
                                while (pos < len && isbindigit (strcontent [pos] ) )
                                {
                			r.str+= strcontent [pos];
                                        ++pos;
		                }
                        }
                        else if (c == 'O' || c == 'o')
                        {
				// Octal
                                ++pos;
                                r.str+= c;
                                while (pos < len && isoctdigit (strcontent [pos] ) )
                                {
                                        r.str+= strcontent [pos];
                                        ++pos;
                                }
                        }
                        else
                        {
				// Hexadecimal
                                if (c == 'H' || c == 'h')
                                {
                                        ++pos;
                                        r.str+= c;
                                }
        	        	while (pos < len && isxdigit (strcontent [pos] ) )
        	        	{
                			r.str+= strcontent [pos];
                                        ++pos;
		                }
                        }
                }
	}
	else
	{
		++pos;
		switch (c)
		{
		case INTEGER_PREFIX:
			r.code= keyINTEGER;
			{
				//r.valueint= * (BlInteger *) (strcontent + pos);
				r.valueint= peek32 (strcontent + pos);
				pos+= 4;
			}
			break;
		case '"':
			r.code= keySTRING;
			while ( (c= strcontent [pos++]) != '\0')
				r.str+= c;
			break;
		case '<':
			if (pos < len)
			{
				switch (strcontent [pos] )
				{
				case '>':
					r.code= keyDISTINCT;
					++pos;
					break;
				case '=':
					r.code= keyMINOREQUAL;
					++pos;
					break;
				default:
					r.code= '<';
				}
			}
			else r.code= c;
			break;
		case '>':
			if (pos < len)
			{
				switch (strcontent [pos] )
				{
				case '=':
					r.code= keyGREATEREQUAL;
					++pos;
					break;
				default:
					r.code= '>';
				}
			}
			else r.code= c;
			break;
		case '\'':
			r.code= lastcode= keyENDLINE;
			++chk;
			pos= len;
			break;
		default:
			r.code= c;
		}
	}
	if (r.code == ':' || r.code == keyTHEN || r.code == keyELSE)
		++chk;
	lastcode= r.code;
	return r;
}

void CodeLine::gotochunk (BlChunk chknew)
{
	pos= 0; chk= 0;
	if (chknew == 0)
	{
		lastcode= 0;
		return;
	}
	while (chk < chknew)
	{
		if (pos >= len)
		{
			lastcode= keyENDLINE;
			return;
		}
		char c= strcontent [pos];
		if (iskey (c) )
		{
			BlCode code;
			code= strcontent [pos++];
			code<<= 8;
			code|= strcontent [pos++];
			if (code == keyTHEN || code == keyELSE)
			{
				lastcode= code;
				++chk;
			}
		}
		else
			if (c == '"')
			{
				++pos;
				while (strcontent [pos] != '\0')
					++pos;
				++pos;
			}
			else
			{
				if (c == ':')
				{
					lastcode= ':';
					++chk;
				}
				++pos;
			}
	}
}

void CodeLine::scan (const std::string & line)
{
	linenumber= 0;
	len= 0;
	pos= 0;
	if (line.empty () )
		return;
	int i= 0, l= line.size ();
	while (i < l && isspace (line [i] ) )
		++i;
	if (i < l && isdigit (line [i] ) )
	{
		while (i < l && isdigit (line [i] ) )
		{
			linenumber*= 10;
			linenumber+= line [i] - '0';
			++i;
		}
		if (i < l && line [i] == ' ') ++i;
	}
	else i= 0;

	std::string newcontent;
	Tokenizer t (line.substr (i) );
	//std::string str;
        Tokenizer::Token token;
        BlCode code;
	bool incomment= false;
        bool addspace;
	while ( ! (token= t.get () ).empty () )
	{
		//std::cerr << '(' << str << ')' << std::flush;
                switch (token.type)
                {
                case Tokenizer::Literal:
                        newcontent+= '"';
			newcontent+= token.str;
			newcontent+= '\0';
                        break;
                case Tokenizer::Integer:
                        newcontent+= INTEGER_PREFIX;
                        //newcontent+= std::string ( (char *) & token.n, 4);
			newcontent+= makestring (token.n);
                        break;
                case Tokenizer::Plain:
			code= 0;
			addspace= false;
			if (! incomment)
			{
				if (token.str == "?")
				{
					code= keyPRINT;
					char c= t.peek ();
					if (c != '\0' && ! isspace (c) )
						addspace= true;
				}
				else
					code= keyword (token.str);
			}
			if (code == 0)
			{
				newcontent+= token.str;
				if (token.str == "'")
					incomment= true;
			}
			else
			{
				newcontent+= char (code >> 8);
				newcontent+= char (code & 0xFF);
				if (code == keyREM)
					incomment= true;
			}
			if (addspace)
				newcontent+= ' ';
                        break;
                default:
                        throw ErrBlassicInternal;
		}
	}
	if (owner && strcontent)
		delete [] strcontent;
	len= newcontent.size ();
	if (len > 0)
	{
		strcontent= new unsigned char [len];
		owner= true;
		memcpy (strcontent, newcontent.data (), len);
	}
	else
	{
		owner= false;
		strcontent= NULL;
	}
}

// Fin de codeline.cpp
