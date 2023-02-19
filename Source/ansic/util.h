// util.h

#ifndef UTIL_H_
#define UTIL_H_

#include <sstream>
#include <stdexcept>
#include <iostream>

namespace util {

template <class DEST, class ORG, class EX>
DEST checked_cast (ORG org, EX ex)
{
	DEST dest= static_cast <DEST> (org);
	if (static_cast <ORG> (dest) != org)
		throw ex;
	return dest;
}

template <class C, size_t N>
size_t dim_array (C (&) [N])
{ return N; }

template <class C, size_t N>
size_t dim_array (const C (&) [N])
{ return N; }

template <class C>
class auto_buffer
{
public:
	auto_buffer (size_t ns) :
		p (new C [ns]),
		s (ns)
	{ }
	auto_buffer () : p (0) { }
	~auto_buffer ()
	{ delete [] p; }
	void release () { p= 0; }
	void alloc (size_t ns)
	{
		delete [] p;
		p= new C [ns];
		s= ns;
	}
	operator C * () { return p; }
	typedef C * iterator;
	iterator begin () { return p; }
	iterator end () { return p + s; }
	// We define data for commodity, and better legibility
	// than begin where not used as iterator.
	C * data () { return p; }
private:
	auto_buffer (const auto_buffer &); // Forbidden
	auto_buffer & operator = (const auto_buffer &); // Forbidden
	C * p;
	size_t s;
};

template <class C>
class auto_alloc
{
public:
	auto_alloc (size_t size) :
		p (static_cast <C *> (malloc (size * sizeof (C) ) ) )
	{
		if (p == 0)
			throw ErrOutMemory;
	}
	~auto_alloc ()
	{ free (p); }
	void release () { p= 0; }
	operator C * () { return p; }
	C * data () { return p; } // Commodity when cast is needed
private:
	auto_alloc (const auto_alloc &); // Forbidden
	auto_alloc & operator = (const auto_alloc &); // Forbidden
	C * p;
};

template <class C>
std::string to_string (const C & c)
{
        std::ostringstream oss;
        oss << c;
        return oss.str ();
}

template <bool assertion>
inline void COMPILE_ASSERTION ()
{ if_this_is_instantiated_then_an_assertion_has_failed (); }

template <>
inline void COMPILE_ASSERTION <true> () { }

#ifndef NDEBUG
inline void assertion_has_failed
	(const char * assertion, const char * file, int line)
{
	std::cerr << std::endl << "Assertion failed: " << assertion <<
		" on " << file << " line " << line << std::endl;
	throw std::logic_error ("Assertion failed.");
}
#endif

} // namespace util

#if ! defined __GNUC__ || __GNUC__ > 2 || \
	(__GNUC__ == 2 && __GNUC_MINOR__ > 95)

#define COMPILE_ASSERT(a) util::COMPILE_ASSERTION <a> ()

#else

#ifndef NDEBUG

#define COMPILE_ASSERT(a) if (a) ; else \
	util::assertion_has_failed (#a, __FILE__, __LINE__)
#else
#define COMPILE_ASSERT(a)
#endif

#endif

#endif // UTIL_H_

// End of util.h
