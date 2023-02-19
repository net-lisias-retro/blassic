// function.h

#include <string>

#include "codeline.h"

class ParameterList {
public:
        ParameterList ();
        ParameterList (const ParameterList & pl);
        ~ParameterList ();
        ParameterList & operator= (const ParameterList & pl);
        void push_back (const std::string & name);
        size_t size () const;
        //const std::string & operator [] (size_t n) const;
        std::string operator [] (size_t n) const;
private:
        class Internal;
        Internal * pin;
};

class Function {
public:
	enum DefType { DefSingle, DefMulti };
        Function (const std::string & strdef, const ParameterList & param);
        Function (ProgramPos posfn, const ParameterList & param);
        Function (const Function & f);
        ~Function ();
        Function & operator= (const Function &);
	DefType getdeftype () const;
        CodeLine & getcode ();
        ProgramPos getpos () const;
        const ParameterList & getparam ();
        static void clear ();
        void insert (const std::string & name);
        static Function & get (const std::string & name);
private:
        class Internal;
        Internal * pin;
};

// Fin de function.h
