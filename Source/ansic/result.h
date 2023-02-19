// result.h
// Revision 18-aug-2003

#ifndef RESULT_H_
#define RESULT_H_

#ifdef __BORLANDC__
#pragma warn -inl
#endif

#include "var.h"

class BlResult {
public:
	BlResult () :
		vartype (VarUndef)
	{ }
	BlResult (BlNumber num) :
		vartype (VarNumber),
		varnumber (num)
	{ }
	BlResult (BlInteger inum) :
		vartype (VarInteger),
		varinteger (inum)
	{ }
	BlResult (const BlResult & br) :
		vartype (br.vartype)
	{
		switch (vartype)
		{
		case VarString:
			varstr= br.varstr;
			break;
		case VarNumber:
			varnumber= br.varnumber;
			break;
		case VarInteger:
			varinteger= br.varinteger;
			break;
		default:
			;
		}
	}
	void operator = (const BlResult & br)
	{
		vartype= br.vartype;
		switch (vartype)
		{
		case VarString:
			varstr= br.varstr;
			break;
		case VarNumber:
			varnumber= br.varnumber;
			break;
		case VarInteger:
			varinteger= br.varinteger;
			break;
		default:
			;
		}
	}
	VarType type () const { return vartype; }
	bool is_numeric () const { return is_numeric_type (vartype); }
	const std::string & str () const
	{
        	if (vartype != VarString)
        		throw ErrMismatch;
		return varstr;
	}
        std::string & str ()
        {
        	if (vartype != VarString)
        		throw ErrMismatch;
                return varstr;
        }
	BlNumber number () const
	{
		switch (vartype)
		{
		case VarNumber:
			return varnumber;
		case VarInteger:
			return varinteger;
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
	}
	BlInteger integer () const
	{
		switch (vartype)
		{
		case VarNumber:
			return BlInteger (varnumber);
		case VarInteger:
			return varinteger;
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
	}
	bool tobool () const
	{
		switch (vartype)
		{
		case VarNumber:
			return varnumber != 0.0;
		case VarInteger:
			return varinteger != 0;
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
	}
	void integertonumber ()
	{
		vartype= VarNumber;
		varnumber= varinteger;
	}
	void numbertointeger ()
	{
		vartype= VarInteger;
		varinteger= BlInteger (varnumber);
	}
	/*BlResult & */ void operator = (const std::string & nstr)
	{
		vartype= VarString;
		varstr= nstr;
		//return * this;
	}
	/*BlResult & */ void operator = (BlNumber num)
	{
		vartype= VarNumber;
		varnumber= num;
		//varstr.erase ();
		//return * this;
	}
	// Do not define operator = for BlInteger,
	// it will be a redefinition for one of
	// long, int or short.
	/*BlResult & */ void operator = (long inum)
	{
		vartype= VarInteger;
		varinteger= inum;
		//varstr.erase ();
		//return * this;
	}
	/*BlResult & */ void operator = (int inum)
	{
		vartype= VarInteger;
		varinteger= inum;
		//varstr.erase ();
		//return * this;
	}
	/*BlResult & */ void operator = (short inum)
	{
		vartype= VarInteger;
		varinteger= inum;
	}
	/*BlResult & */ void operator = (size_t inum)
	{
		vartype= VarInteger;
		varinteger= inum;
		//varstr.erase ();
		//return * this;
	}
	/*BlResult & */ void operator += (const BlResult & br)
	{
		switch (vartype)
		{
		case VarString:
			varstr+= br.str ();
			break;
		case VarNumber:
			varnumber+= br.number ();
			break;
		case VarInteger:
			switch (br.vartype)
			{
			case VarInteger:
				varinteger+= br.integer ();
				break;
			default:
				integertonumber ();
				varnumber+= br.number ();
			}
			break;
		default:
			throw ErrBlassicInternal;
		}
		//return * this;
	}
	/*BlResult & */ void operator -= (const BlResult & br)
	{
		switch (vartype)
		{
		case VarNumber:
			varnumber-= br.number ();
			break;
		case VarInteger:
			switch (br.vartype)
			{
			case VarInteger:
				varinteger-= br.integer ();
				break;
			default:
				vartype= VarNumber;
				varnumber= varinteger;
				varnumber-= br.number ();
			}
			break;
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
		//return * this;
	}
	/*BlResult & */ void operator *= (const BlResult & br)
	{
		switch (vartype)
		{
		case VarNumber:
			varnumber*= br.number ();
			break;
		case VarInteger:
			switch (br.vartype)
			{
			case VarInteger:
				varinteger*= br.integer ();
				break;
			default:
				integertonumber ();
				varnumber*= br.number ();
			}
			break;
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
		//return * this;
	}
	/*BlResult & */ void operator /= (const BlResult & br)
	{
		switch (vartype)
		{
		case VarInteger:
			integertonumber ();
		case VarNumber:
			{
				BlNumber n= br.number ();
				if (n == 0)
					throw ErrDivZero;
				varnumber/= n;
			}
			break;
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
		//return * this;
	}
	BlResult operator - ()
	{
		switch (vartype)
		{
		case VarNumber:
			return BlResult (-varnumber);
		case VarInteger:
			return BlResult (-varinteger);
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
	}
	/*BlResult & */ void operator %= (const BlResult & br)
	{
		switch (vartype)
		{
		case VarInteger:
			break;
		case VarNumber:
			numbertointeger ();
			break;
		case VarString:
			throw ErrMismatch;
		default:
			throw ErrBlassicInternal;
		}
		BlInteger r= br.integer ();
		if (r == 0)
			throw ErrDivZero;
		varinteger= varinteger %  r;
		//return * this;
	}
	bool operator == (const BlResult & br)
	{
		switch (vartype)
		{
		case VarString:
			return varstr == br.str ();
		case VarNumber:
			return varnumber == br.number ();
		case VarInteger:
			switch (br.vartype)
			{
			case VarInteger:
				return varinteger == br.varinteger;
			case VarNumber:
				return varinteger == br.varnumber;
			case VarString:
				throw ErrMismatch;
			default:
				throw ErrBlassicInternal;
			}
		default:
			throw ErrBlassicInternal;
		}
	}
	bool operator != (const BlResult & br)
	{
		return ! operator == (br);
	}
	bool operator < (const BlResult & br)
	{
		switch (vartype)
		{
		case VarString:
			return varstr < br.str ();
		case VarNumber:
			return varnumber < br.number ();
		case VarInteger:
			return number () < br.number ();
		default:
			throw ErrBlassicInternal;
		}
	}
	bool operator <= (const BlResult & br)
	{
		switch (vartype)
		{
		case VarString:
			return varstr <= br.str ();
		case VarNumber:
			return varnumber <= br.number ();
		case VarInteger:
			return number () <= br.number ();
		default:
			throw ErrBlassicInternal;
		}
	}
	bool operator > (const BlResult & br)
	{
		return ! operator <= (br);
	}
	bool operator >= (const BlResult & br)
	{
		return ! operator < (br);
	}
private:
	VarType vartype;
	std::string varstr;
	union {
		BlNumber varnumber;
		BlInteger varinteger;
	};
};

#endif

// Fin de result.h
