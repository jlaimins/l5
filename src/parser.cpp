#include "parser.h"
#include <stdexcept>
#include <sstream>

using std::runtime_error;

class ParserImpl {
public:
    ParserImpl(std::wistream &stream)
        : m_tokenizer(Tokenizer(stream)) {
    }

    ~ParserImpl() {
    }

    const ParseTree * parse() {
        return this->program();
    }
protected:
    ParseTree *program() {
        ParseTree *pt = NULL, *t, *c;

        Token tok = this->m_tokenizer.peek(true);
        while ( tok.type != TOK_EOF ) {
            if ( tok.type == TOK_CLASS )
                c = this->cdecl();
            else if ( tok.type == TOK_ASSOC )
                c = this->adef();
            else {
                c = new ParseTree(PT_PATTERN, L"", this->pattern());
            }

            if ( pt == NULL ) {
                pt = t = c;
            }
            else {
                t->next = c;
                t = c;
            }

            tok = this->m_tokenizer.peek(true);
        }

        return pt;
    }

    ParseTree *pattern() {
        Token tok = this->m_tokenizer.peek(true);
        ParseTree *pt = NULL, *t, *c;
        while ( true ) {
            if(tok.type == TOK_SYM) {
                if ( tok.value == L"**" || tok.value == L"*" || tok.value == L"**+" || tok.value == L"*+") {
                    this->m_tokenizer.eat(true);
                    c = new ParseTree(PT_DECORATION, tok.value, this->var());
                }
                else if ( tok.value == L"!") {
                    this->m_tokenizer.eat(true);
                    c = new ParseTree(PT_DECORATION, tok.value, this->creation());
                }
                else if ( tok.value == L"{" ) {
                    tok = this->m_tokenizer.eat(true); // {
                    if ( tok.type != TOK_SYM || tok.value != L"}" ) { // handle empty expression block
                        ParseTree *x = NULL, *cx;
                        do {
                            cx = new ParseTree(PT_EXPRESSION, L"", this->expr());
                            if ( x == NULL ) {
                                c = x = cx;
                            }
                            else {
                                x->next = cx;
                                x = cx;
                            }
                            tok = this->m_tokenizer.peek(true);
                        } while ( tok.type == TOK_SYM && tok.value == L"," && this->m_tokenizer.eat(true).type != TOK_EOF);
                    }

                    if ( tok.type != TOK_SYM || tok.value != L"}" )
                        throw runtime_error("Expected '}'. ");
                    this->m_tokenizer.eat(true); // }
                }
                else if ( tok.value == L"[" ) {
                    c = new ParseTree(PT_CONTEXT, L"", this->context());
                }
                else if ( tok.value == L"|" || tok.value == L"]" ) {
                    c = NULL;
                    break;
                }
                else {
                    throw std::runtime_error("Unexpected symbol!");
                }
            }
            else if ( tok.type == TOK_ID ) {
                c = this->var();
            }
            else {
                throw std::runtime_error("Syntax error!");
            }

            if ( pt == NULL ) {
                pt = t = c;
            }
            else {
                t->next = c;
                t = c;
                while ( t->next != NULL ) t = (ParseTree *)t->next; // FIXME: skip to the end
            }

            tok = this->m_tokenizer.peek(true);
            if ( tok.type == TOK_EOF || tok.type == TOK_CLASS || tok.type == TOK_ASSOC )
                return pt;
        }

        return pt;
    }

    ParseTree * context() {
        // "[" is already removed, read until "]"
        ParseTree *pt = NULL, *c, *p;
        Token t = Token(TOK_EOF, L"");
        do
        {
            t = this->m_tokenizer.eat(true);
            p = new ParseTree(PT_ALTERNATIVE, L"", this->pattern());

            if ( pt == NULL ) {
                c = pt = p;
            }
            else {
                c->next = p;
                c = p;
            }

            t = this->m_tokenizer.peek(true);
        } while ( t.type == TOK_SYM && t.value == L"|" );

        if ( t.type != TOK_SYM && t.value != L"]" )
            throw runtime_error("Expected ']'");

        this->m_tokenizer.eat(true);
        return pt;
    }

    /** Parses a variable pattern
     * var ::= ID (':' ID | '.' DECORATION? ID '.' 'ID' )
     */
    ParseTree * var() {
        Token tok = this->m_tokenizer.peek(true);
        if(tok.type != TOK_ID)
            throw std::runtime_error("Identifier expected!");

        Token next = this->m_tokenizer.eat(true);
        if(next.value == L":") {
            next = this->m_tokenizer.eat(true);
            if(next.type != TOK_ID)
                throw std::runtime_error("Identifier expected!");

            this->m_tokenizer.eat(true);
            return new ParseTree(PT_VAR_DEF, tok.value, new ParseTree(PT_CLASS, next.value));
        }

        if(next.value == L".") {
            std::wstring linkName;
            ParseTree *pd = NULL,
                      *pt = new ParseTree(PT_VAR_REF, tok.value); // variable reference

            next = this->m_tokenizer.eat(true);
            if ( next.type == TOK_SYM ) { // decoration?
                pd = new ParseTree(PT_DECORATION, next.value);

                tok = this->m_tokenizer.eat(true); // ID - assoc name
                if(tok.type != TOK_ID)
                    throw std::runtime_error("Identifier expected!");

                linkName = tok.value;
            }
            else if ( next.type != TOK_ID) { // ID
                throw std::runtime_error("Identifier expected!");
            }
            else { // next.type == TOK_ID
                linkName = next.value;
            }

            next = this->m_tokenizer.eat(true); // '.'
            if ( next.value != L"." ) {
                throw std::runtime_error("Expected '.'");
            }

            next = this->m_tokenizer.eat(true); // ID - var name
            if ( next.type != TOK_ID) {
                throw std::runtime_error("Identifier expected!");
            }

            this->m_tokenizer.eat(true);
            pt->next = new ParseTree(PT_VAR_DEF, next.value);
            if(pd != NULL) {
                pd->child = new ParseTree(PT_LINK, linkName, pt);
                return pd;
            }

            return new ParseTree(PT_LINK, linkName, pt);
        }

        std::stringstream ss;
        ss << this->m_tokenizer.lineAt() << ":" << this->m_tokenizer.charAt() << " - Unexpected token!";
        throw std::runtime_error(ss.str());
    }

    /** Parses a simple class declaration pattern
     * CDECL ::= 'class' ID '{' (ID ':' ID)* '}'
    */
    ParseTree *cdecl() {
        Token tok = this->m_tokenizer.peek(true);
        if ( tok.type != TOK_CLASS )
            throw std::runtime_error("Expected 'class'!");

        Token cname = this->m_tokenizer.eat(true);
        tok = this->m_tokenizer.eat(true);
        if ( tok.type != TOK_SYM || tok.value != L"{" )
            throw std::runtime_error("Expected '{'!");

        ParseTree *tree = new ParseTree(PT_CLASS_DEF, cname.value);
        try {
            // ParseTree: PT_CLASS, name - { child: name, child { type }, next: name, child { type } }
            ParseTree *p = NULL, *c;

            tok = this->m_tokenizer.peek(true, 2);
            if ( tok.type != TOK_SYM || tok.value != L"}") {
                do {
                    tok = this->m_tokenizer.eat(true);

                    if ( tok.type != TOK_ID )
                        throw std::runtime_error("Expected identifier!");

                    if ( this->m_tokenizer.eat(true).value != L":" )
                        throw std::runtime_error("Expected ':'!");

                    if ( this->m_tokenizer.eat(true).type != TOK_ID)
                        throw std::runtime_error("Expected identifier!");

                    c = new ParseTree(PT_ATTR, tok.value);

                    tok = this->m_tokenizer.peek();
                    c->child = new ParseTree(PT_CLASS, tok.value);

                    if ( p == NULL )
                        tree->child = c;
                    else
                        p->next = c;

                    p = c;
                    tok = this->m_tokenizer.eat(true);
                } while (tok.type == TOK_SYM && tok.value == L",");
            }
            else {
                tok = this->m_tokenizer.eat(true);
            }

            if ( tok.value != L"}")
                throw std::runtime_error("Expected '}'!");

            this->m_tokenizer.eat(true);
            return tree;
        }
        catch (...) {
            if ( tree != NULL )
            delete tree;
            throw;
        }

        return NULL;
    }

    /** Parses a simple association definition pattern
     * ADEF ::= 'associate' ID? ':' ID 'with' ID? ':' ID
    */
    ParseTree *adef() {
        Token tok = this->m_tokenizer.peek(true);
        if ( tok.type != TOK_ASSOC )
            throw std::runtime_error("Association definition expected!");

        ParseTree *tree = new ParseTree(PT_ASSOC_DEF, L""), *t, *c;
        tok = this->m_tokenizer.eat(true);

        if ( tok.type == TOK_ID ) {
            t = new ParseTree(PT_ASSOC, tok.value);
            tok = this->m_tokenizer.eat(true);
        }
        else {
            t = new ParseTree(PT_ASSOC, L"");
        }

        tree->child = t;
        if ( tok.type != TOK_SYM || tok.value != L":" )
            throw std::runtime_error("Expected ':'!");

        tok = this->m_tokenizer.eat(true);
        if ( tok.type != TOK_ID )
            throw std::runtime_error("Expected class identifier!");

        c = new ParseTree(PT_CLASS, tok.value);
        t->child = c;

        tok = this->m_tokenizer.eat(true);
        if ( tok.type != TOK_ID || tok.value != L"with" )
            throw std::runtime_error("Expected 'with'!");

        tok = this->m_tokenizer.eat(true);
        if ( tok.type == TOK_ID ) {
            c = new ParseTree(PT_ASSOC, tok.value);
            tok = this->m_tokenizer.eat(true);
        }
        else {
            c = new ParseTree(PT_ASSOC, L"");
        }

        t->next = c;
        t = c;
        if ( tok.type != TOK_SYM || tok.value != L":" )
            throw std::runtime_error("Expected ':'!");

        tok = this->m_tokenizer.eat(true);
        if ( tok.type != TOK_ID )
            throw std::runtime_error("Expected class identifier!");

        c = new ParseTree(PT_CLASS, tok.value);
        t->child = c;

        this->m_tokenizer.eat(true);
        return tree;
    }

    /** Parse object creation pattern
     * CRT ::= '!' ID ':' ID '{' ( ID '=' EXPR ( ',' ID '=' EXPR )* )? '}'
     */
    ParseTree * creation() {
        ParseTree *parseTree, *p, *c = NULL;
        Token t = this->m_tokenizer.peek(true);
        if ( t.type != TOK_ID )
            throw runtime_error("Identifier expected!");

        p = parseTree = new ParseTree(PT_VAR_DEF, t.value);

        t = this->m_tokenizer.eat(true);
        if ( t.type != TOK_SYM || t.value != L":" )
            throw runtime_error("Expected ':'!");

        t = this->m_tokenizer.eat(true);
        if ( t.type != TOK_ID )
            throw runtime_error("Class name expected!");

        p->child = c = new ParseTree(PT_CLASS, t.value);
        p = c;

        t = this->m_tokenizer.eat(true);
        /* if ( t.type == TOK_SYM && t.value == L"{" ) {
            p->child = c = new ParseTree(PT_INITIALIZER, L"");
            p = c;

            c = NULL;
            while ( t.type != TOK_EOF && t.value != L"}" ) {
                t = this->m_tokenizer.eat(true); // ID

                Token o = this->m_tokenizer.eat(true); // '='
                if ( o.type != TOK_SYM || o.value != L"=" )
                    throw runtime_error("Expected '='!");

                this->m_tokenizer.eat(true); // 'expr'
                if ( c == NULL ) {
                    c = new ParseTree(PT_ATTR, t.value, this->expr());
                    p->child = c;
                }
                else {
                    c->next = p = new ParseTree(PT_ATTR, t.value, this->expr());
                    c = p;
                }

                t = this->m_tokenizer.peek(true); // either ',' or '}'
            }

            this->m_tokenizer.eat(true); // eat '}'
        } */

        return parseTree;
    }

    /**
     * Tokenizer should already be on first expression symbol!
     */
    ParseTree *expr() {
        ParseTree *pt = NULL;
        Token t = this->m_tokenizer.peek(true);

        if ( t.type == TOK_NUM ) {
            pt = new ParseTree(PT_NUMBER, t.value);
            t = this->m_tokenizer.eat(true);
        }
        else if ( t.type == TOK_STR ) {
            pt = new ParseTree(PT_STRING, t.value);
            t = this->m_tokenizer.eat(true);
        }
        else if ( t.type == TOK_BOOL ) {
            pt = new ParseTree(PT_BOOL, t.value);
            t = this->m_tokenizer.eat(true);
        }
        else if ( t.type == TOK_ID ) {
            Token n = this->m_tokenizer.eat(true); // <id> . <id>
            if ( n.value == L"." ) {
                n = this->m_tokenizer.eat(true);
                pt = new ParseTree(PT_ATTR, n.value, new ParseTree(PT_VAR_REF, t.value));
            }
            else
                throw runtime_error("Expected '.'"); // TODO: '('

            t = this->m_tokenizer.eat(true);
            if ( t.type == TOK_SYM && t.value == L"=" ) {
                this->m_tokenizer.eat(true);
                pt = new ParseTree(PT_BINOP, t.value, pt);
                pt->next = this->expr();
            }
        }

        t = this->m_tokenizer.peek(true);
        if ( t.type == TOK_SYM && (
             t.value == L"==" ||
             t.value == L"+" ||
             t.value == L"-" ||
             t.value == L"*" ||
             t.value == L"/" ||
             t.value == L"&&" ||
             t.value == L"||" ) )
        {
            // TODO: operator precedence
            this->m_tokenizer.eat(true);
            pt = new ParseTree(PT_BINOP, t.value, pt);
            pt->next = this->expr();
        }

        return pt;
    }
private:
    Tokenizer m_tokenizer;
};

Parser::Parser(std::wistream &stream)
    : _impl(new ParserImpl(stream)) {
}

Parser::~Parser() {
    if ( this->_impl != NULL ) {
        delete this->_impl;
    }

    this->_impl = NULL;
}

const ParseTree * Parser::parse() {
    return this->_impl->parse();
}

ParseTree::ParseTree(ParseTreeType t, std::wstring v, const ParseTree *c)
    : type(t)
    , value(v)
    , child(c)
    , next(NULL) {
}

ParseTree::~ParseTree() {
    if ( this->next != NULL ) {
        delete this->next;
    }

    this->next = NULL;
}
