// using.h
// Revision 31-jul-2003

#ifndef INCLUDE_USING_H
#define INCLUDE_USING_H

#include "error.h"
#include "file.h"

#include <iostream>
#include <string>

class Using {
protected:
	Using () { }
public:
	virtual ~Using ();
	virtual Using * clone () const
	{ throw ErrBlassicInternal; }
	virtual bool isliteral () const;
	virtual void putliteral (BlFile &) const;
	virtual void putnumeric (BlFile &, BlNumber) const;
	virtual void putstring (BlFile &, const std::string &) const;
};

class UsingLiteral : public Using {
public:
	UsingLiteral (std::istream & is);
	UsingLiteral (std::istream & is, const std::string & strinit);
	UsingLiteral (const std::string & strinit);
	UsingLiteral * clone () const
	{ return new UsingLiteral (* this); }
	bool isliteral () const;
	const std::string & str () const { return s; }
	void addstr (const std::string & stradd);
	void putliteral (BlFile & out) const;
private:
	std::string s;
	void init (std::istream & is);
};

class UsingNumeric : public Using {
public:
	UsingNumeric (std::istream & is, std::string & tail);
	UsingNumeric * clone () const
	{ return new UsingNumeric (* this); }
	const std::string & str () const { return s; }
	bool isvalid () const { return ! invalid; }
	void putnumeric (BlFile & out, BlNumber n) const;
private:
	bool invalid;
	std::string s;
	size_t digit, decimal, scientific;
	bool milliards, putsign, signatend, blankpositive;
	bool asterisk, dollar, pound, euro;
};

class UsingString : public Using {
public:
	UsingString (std::istream & is);
	UsingString * clone () const
	{ return new UsingString (* this); }
	void putstring (BlFile & out, const std::string & str) const;
private:
	size_t n;
};

class VectorUsing
{
public:
	VectorUsing ();
	~VectorUsing ();
	size_t size () const { return v.size (); }
	bool empty () const { return v.empty (); }
	Using * & back () { return v.back (); }
	Using * & operator [] (size_t i) { return v [i]; }
	void push_back (const Using & u);
private:
	VectorUsing (const VectorUsing &); // Forbidden
	VectorUsing & operator = (const VectorUsing &); // Forbidden
	static void delete_it (const Using * u) { delete u; }

	std::vector <Using *>  v;
};

void parseusing (const std::string & str, VectorUsing & vu);

#endif

// End of using.h
