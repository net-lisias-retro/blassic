// using.h
// Revision 1-jun-2003

#ifndef INCLUDE_USING_H
#define INCLUDE_USING_H

#include "error.h"
#include "file.h"

#include <iostream>
#include <string>

class Using {
public:
	Using () { }
	virtual ~Using ();
	virtual bool isliteral () const;
	virtual void putliteral (BlFile &) const;
	virtual void putnumeric (BlFile &, BlNumber) const;
	virtual void putstring (BlFile &, const std::string &) const;
};

class UsingLiteral : public Using {
public:
	UsingLiteral (std::istream & is,
		const std::string & strinit= std::string () );
	bool isliteral () const;
	void addstr (const std::string & stradd);
	void putliteral (BlFile & out) const;
private:
	std::string s;
};

class UsingNumeric : public Using {
public:
	UsingNumeric (std::istream & is, std::string & tail);
	void putnumeric (BlFile & out, BlNumber n) const;
private:
	size_t digit, decimal, scientific;
	bool milliards, putsign, signatend, blankpositive;
	bool asterisk, dollar, pound, euro;
};

class UsingString : public Using {
public:
	UsingString (std::istream & is);
	void putstring (BlFile & out, const std::string & str) const;
private:
	size_t n;
};

class VectorUsing : public std::vector <Using *>
{
public:
	VectorUsing ();
	~VectorUsing ();
private:
	VectorUsing (const VectorUsing &); // Forbidden
	VectorUsing & operator = (const VectorUsing &); // Forbidden
	static void delete_it (const Using * u) { delete u; }
};

void parseusing (const std::string & str, VectorUsing & vu);

#endif

// End of using.h
