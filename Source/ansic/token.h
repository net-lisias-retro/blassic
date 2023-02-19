// token.h

#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

#include "blassic.h"

class Tokenizer {
public:
        enum Type { Empty, Plain, Literal, Integer };
	Tokenizer (const std::string & source);
        class Token {
        public:
                Token () :
                        type (Empty)
                { }
                Token (Type t, const std::string & str) :
                        type (t), str (str)
                { }
                Token (BlInteger n) :
                        type (Integer), n (n)
                { }
                bool empty () { return type == Plain && str.empty (); }
                Type type;
                std::string str;
                BlInteger n;
        };
	Token get ();
	char peek ();
private:
	char nextchar ();
	void ungetchar ();
	std::string str;
	size_t pos, limit;
};

#endif

// Fin de token.h
