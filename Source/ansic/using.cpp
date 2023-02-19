// using.cpp
// Revision 2-jun-2003

#include "using.h"
#include "util.h"
#include "trace.h"

#include <sstream>
#include <iomanip>
#include <memory>
using std::auto_ptr;
#include <cstdlib>
#include <cmath>

#include <cassert>
#define ASSERT assert

//**************************************
//	Auxiliary functions
//**************************************

namespace {

const char poundsign= '\xA3';
#ifdef _Windows
const char eurosign= '\x80';
#else
const char eurosign= '\xA4';
#endif

bool charformat [256]= { false };

bool initformat ()
{
	charformat [static_cast <unsigned char> ('#') ]= true;
	charformat [static_cast <unsigned char> ('.') ]= true;
	charformat [static_cast <unsigned char> ('+') ]= true;
	charformat [static_cast <unsigned char> ('*') ]= true;
	charformat [static_cast <unsigned char> ('$') ]= true;
	charformat [static_cast <unsigned char> (poundsign) ]= true;
	charformat [static_cast <unsigned char> (eurosign) ]= true;
	charformat [static_cast <unsigned char> ('\\') ]= true;
	charformat [static_cast <unsigned char> ('&') ]= true;
	charformat [static_cast <unsigned char> ('!') ]= true;
	return true;
}

bool initedformat= initformat ();

bool ischarformat (char c)
{
	return charformat [static_cast <unsigned char> (c) ];
}

}

//**************************************
//		Using
//**************************************

Using::~Using ()
{ }

bool Using::isliteral () const
{ return false; }

void Using::putliteral (BlFile &) const
{ throw ErrBlassicInternal; }

void Using::putnumeric (BlFile &, BlNumber) const
{ throw ErrBlassicInternal; }

void Using::putstring (BlFile &, const std::string &) const
{ throw ErrBlassicInternal; }

//**************************************
//		UsingLiteral
//**************************************

UsingLiteral::UsingLiteral (std::istream & is,
	const std::string & strinit)
{
	s= strinit;
	char c;
	while (is >> c && ! ischarformat (c) )
	{
		s+= c;
	}
	ASSERT (! s.empty () );
	if (is)
		is.unget ();
}

bool UsingLiteral::isliteral () const
{ return true; }

void UsingLiteral::addstr (const std::string & stradd)
{
	s+= stradd;
}

void UsingLiteral::putliteral (BlFile & out) const
{
	out << s;
}

//**************************************
//		UsingNumeric
//**************************************

namespace {

class NoNumeric {
public:
	NoNumeric (const std::string & s) :
		s (s)
	{ }
	NoNumeric (char c) :
		s (1, c)
	{ }
	const std::string & str () const { return s; }
private:
	std::string s;
};

}

UsingNumeric::UsingNumeric (std::istream & is, std::string & tail) :
	digit (0),
	decimal (0),
	scientific (0),
	milliards (false),
	putsign (false),
	signatend (false),
	blankpositive (false),
	asterisk (false),
	dollar (false),
	pound (false),
	euro (false)
{
	TraceFunc tr ("UsingNumeric::UsingNumeric");

	char c;
	is >> c;
	ASSERT (is);
	if (c == '+')
	{
		putsign= true;
		is >> c;
	}

	if (is)
	{
		switch (c)
		{
		case '*':
			if (! (is >> c) )
				throw NoNumeric ("*");
			if (c != '*')
			{
				is.unget ();
				throw NoNumeric ("*");
			}
			asterisk= true;
			digit+= 2;
			if (is >> c)
			{
				switch (c)
				{
				case '$':
					dollar= true;
					is >> c;
					break;
				case poundsign:
					pound= true;
					is >> c;
					break;
				case eurosign:
					euro= true;
					is >> c;
					break;
				}
			}
			break;
		case '$':
		case poundsign:
		case eurosign:
			{
				char sign= c;
				if (! (is >> c) )
					throw NoNumeric (sign);
				if (c != sign)
				{
					is.unget ();
					throw NoNumeric (sign);
				}
				++digit;
				switch (sign)
				{
				case '$': dollar= true; break;
				case poundsign: pound= true; break;
				case eurosign: euro= true; break;
				}
			}
			is >> c;
			break;
		}
	}
	
	while (is && c == '#')
	{
		++digit;
		is >> c;
	}
	if (is)
	{
		if (c == '.')
		{
			while (is >> c && c == '#')
				++decimal;
		}
	}
	if (is)
	{
		if (c == '^')
		{
			++scientific;
			while (scientific < 5 && is >> c && c == '^')
				++scientific;
			if (scientific == 5)
				is >> c;
			if (scientific < 4)
			{
				tail= std::string (scientific, '^');
				scientific= 0;
			}
			else
				scientific-= 2;
		}
	}
	if (is && (c == '+' || c == '-') )
	{
		putsign= true;
		signatend= true;
		if (c == '-')
			blankpositive= true;
		is >> c;
	}
	if (is)
		is.unget ();
}

namespace {

class UsingOverflow { };

}

void UsingNumeric::putnumeric (BlFile & out, BlNumber n) const
{
	try
	{
		int negative= 0;
		if (scientific > 0)
		{
			size_t d= digit;
			if (n < 0 && ! putsign)
				--d;
			int prec= d + decimal;
			int dec;
			char * aux= ecvt (n, prec, & dec, & negative);
			int e= dec - static_cast <int> (d);
			if (scientific == 0 && e != 0)
				throw UsingOverflow ();
			std::string stre= util::to_string (abs (e) );
			int ezeroes= scientific - stre.size ();
			if (ezeroes < 0)
				throw UsingOverflow ();

			if (putsign && ! signatend)
			{
				out << (negative ? '-' : '+');
			}
			if (! putsign && negative)
				out << '-';
			out << (std::string (aux, d) + '.' +
				std::string (aux + d) ) <<
				'E' << (e < 0 ? '-' : '+') <<
				std::string (ezeroes, '0') <<
				stre;
		}
		else
		{
			int numdig= 0;
			negative= n < 0;
			if (negative)
				n= -n;
			if (n != 0)
				numdig= int (std::log10 (n * 10 ) );
			if (numdig < 0)
				numdig= 0;
			size_t w= numdig;
			if (digit > w)
				w= digit;
			size_t maxw= digit;
			if (decimal > 0)
			{
				w+= decimal + 1;
				maxw+= decimal + 1;
			}
			if (negative && ! putsign)
			{
				--w;
				--maxw;
			}
			std::ostringstream oss;
			if (decimal > 0)
				oss.setf (std::ios::showpoint);

			// Some implementations lack the fixed manipulator.
			oss.setf (std::ios::fixed, std::ios::floatfield);
			oss << std::setw (w) <<
				//std::setprecision (decimal + numdig) <<
				std::setprecision (decimal) <<
				//std::fixed <<
				n;
			std::string strn= oss.str ();
			if (digit == 0 && strn [0] == '0')
				strn.erase (0, 1);
			if (strn.size () > maxw)
				throw UsingOverflow ();
			if (dollar || pound || euro)
			{
				char sign= dollar ? '$' :
					pound ? poundsign : eurosign;
				strn.insert (strn.find_first_not_of (" "),
					1, sign);
			}
			if ( (putsign && ! signatend) ||
				(! putsign && negative) )
			{
				strn.insert (strn.find_first_not_of (" "),
					1, negative ? '-' : '+');
			}
			if (asterisk)
			{
				for (size_t i= 0,
					l= strn.find_first_not_of (" ");
					i < l; ++i)
				{
					strn [i]= '*';
				}
			}
			out << strn;
		}
		if (signatend)
		{
			ASSERT (putsign);
			out << (negative ? '-' :
				blankpositive ? ' ' : '+' );
		}
	}
	catch (UsingOverflow &)
	{
		out << '%' << n;
	}
}

//**************************************
//		UsingString
//**************************************

UsingString::UsingString (std::istream & is) :
	n (0)
{
	char c;
	is >> c;
	ASSERT (is);
	switch (c)
	{
	case '\\':
		n= 1;
		while (is >> c && c == ' ')
			++n;
		if (is)
		{
			if (c == '\\')
				++n;
			else
				is.unget ();
		}
		break;
	case '&':
		// Nothing in particular
		break;
	case '!':
		n= 1;
		break;
	default:
		throw ErrBlassicInternal;
	}
}

void UsingString::putstring (BlFile & out, const std::string & str) const
{
	if (n > 0)
		out << str.substr (0, n);
	else
		out << str;
}

//**************************************
//		VectorUsing
//**************************************

VectorUsing::VectorUsing ()
{
}

VectorUsing::~VectorUsing ()
{
}

void parseusing (const std::string & str, VectorUsing & vu)
{
	TraceFunc tr ("parseusing");

	std::istringstream is (str);
	is.unsetf (std::ios::skipws);
	int c;
	while ( (c= is.peek () ) != EOF)
	{
		switch (static_cast <char> (c) )
		{
		case '#':
		case '.':
		case '+':
		case '*':
		case '$':
		case poundsign:
		case eurosign:
			try
			{
				std::string tail;
				auto_ptr <UsingNumeric>
					pun (new UsingNumeric (is, tail) );
				vu.push_back (& * pun);
				pun.release ();
				if (! tail.empty () )
				{
					auto_ptr <UsingLiteral>
						pul (new UsingLiteral
							(is, tail) );
					vu.push_back (& * pul);
					pul.release ();
				}
			}
			catch (NoNumeric & e)
			{
				tr.message (std::string ("Catched: ") +
					e.str () );
				UsingLiteral * prev;
				if (! vu.empty () && (prev=
					dynamic_cast <UsingLiteral *>
						(vu.back () ) ) != NULL)
				{
					prev->addstr (e.str () );
				}
				else
				{
					auto_ptr <UsingLiteral>
						pul (new UsingLiteral
							(is, e.str () ) );
					vu.push_back (& * pul);
					pul.release ();
				}
			}
			break;
		case '\\':
		case '&':
		case '!':
			{
				auto_ptr <UsingString>
					pus (new UsingString (is) );
				vu.push_back (& * pus);
				pus.release ();
			}
			break;
		default:
			{
				auto_ptr <UsingLiteral>
					pul (new UsingLiteral (is) );
				vu.push_back (& * pul);
				pul.release ();
			}
		}
	}
}

// End of using.cpp
