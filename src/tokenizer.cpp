#include "tokenizer.h"
#include <list>
#include <sstream>

class TokenizerImpl {
public:
    TokenizerImpl(std::wistream &stream)
        : m_stream(stream)
        , m_line(1)
        , m_char(1)
        , m_look(0) {
    }

    virtual ~TokenizerImpl() {
    }

    virtual const Token eat(bool ws, unsigned int n) {
        if ( ws ) {
            while ( this->m_list.size() > 0 ) {
                if ( this->m_list.front().type != TOK_WS ) {
                    if ( n == 0 ) {
                        return this->m_list.front();
                    }


                    n -= 1;
                }

                this->m_list.pop_front();
            }

            Token tok = this->next();
            while ( n > 0 || tok.type == TOK_WS ) {
                if ( tok.type != TOK_WS ) {
                    n -= 1;
                }

                tok = this->next();
            }

            this->m_list.push_back(tok);
        }
        else if ( n >= this->m_list.size() ) {
            n -= this->m_list.size();
            this->m_list.clear();

            for(; n > 0; n -= 1) {
                this->next();
            }

            this->m_list.push_back(this->next());
        }
        else {
            for (; n > 0; n -= 1 ) {
                this->m_list.pop_front();
            }
        }

        return this->m_list.front();
    }

    virtual const Token peek(bool ws, unsigned int n) {
        if ( ws ) {
            for ( std::list<Token>::const_iterator it = this->m_list.begin();
                  n > 0 && it != this->m_list.end();
                  it++ ) {
                Token t = *it;
                if ( t.type != TOK_WS )
                    n -= 1;

                if ( n == 0 ) {
                    return t;
                }
            }

            Token tok = this->next();
            while ( (n > 1 || tok.type == TOK_WS) && tok.type != TOK_EOF ) {
                if ( tok.type != TOK_WS ) {
                    n -= 1;
                }

                this->m_list.push_back(tok);
                tok = this->next();
            }

            this->m_list.push_back(tok);
            return tok;
        }
        else if ( n >= this->m_list.size() ) {
            n -= this->m_list.size();
            for(; n > 0; n -= 1) {
                this->m_list.push_back(this->next());
            }

            return this->m_list.back();
        }
        else {
            std::list<Token>::const_iterator it = this->m_list.begin();
            std::advance(it, n);
            return *it;
        }
    }

    /** returns next @see Token from stream
     * Token EOF: { 0, L"" } - end of file token
     * Token WS: { 1, L"" } - whitespace token
     * Token SYM: { 2, L<value> } - symbol token
     * Token ID: { 3, L<value> } - identifier token
     * Token NUM: { 4, L<value } - numeric token
     * Token STR: { 5, L<value> } - string token
    */
    virtual Token next() {
        wchar_t ch;
        if ( this->m_look != 0 ) {
            ch = this->m_look;
            this->m_look = 0;
        }
        else {
            ch = this->getchar();
        }

        if ( this->m_stream.eof() ) {
            return Token(TOK_EOF, L"");
        }

        if (::iswspace(ch)) {
            while ( ::iswspace(ch) ) {
                ch = this->getchar();
            }

            this->m_look = ch;
            return Token(TOK_WS, L"");
        }

        std::wstringstream ss;
        if (!::iswprint(ch)) {
            ss << ch;
            return Token(TOK_ERR, ss.str());
        }

        if (::iswalpha(ch)) {
            while ( ::iswalnum(ch)) {
                ss << ch;
                ch = this->getchar();
            }

            this->m_look = ch;
            std::wstring s = ss.str();
            if ( L"class" == s )
                return Token(TOK_CLASS, s);
            else if ( L"associate" == s )
                return Token(TOK_ASSOC, L"");
            else if ( L"true" == s || L"false" == s )
                return Token(TOK_BOOL, s);

            return Token(TOK_ID, s);
        }

        if (::iswdigit(ch)) {
            while ( ::isdigit(ch)) {
                ss << ch;
                ch = this->getchar();
            }

            this->m_look = ch;
            return Token(TOK_NUM, ss.str());
        }

        if (ch == L'"') {
            ch = this->getchar();
            while ( ch != L'"' ) {
                ss << ch;
                ch = this->getchar();
            }
            this->m_look = this->getchar();
            return Token(TOK_STR, ss.str());
        }

        ss << ch;
        if  (ch == L'*' ) {
            ch = this->getchar();
            if(ch != L'*' && ch != L'+' ) {
                this->m_look = ch;
                return Token(TOK_SYM, ss.str());
            }

            ss << ch;
            ch = this->getchar();
            if (ch != '+') {
                this->m_look = ch;
                return Token(TOK_SYM, ss.str());
            }

            ss << ch;
            return Token(TOK_SYM, ss.str());
        }
        if ( ch == L'=' ) {
            ch = this->getchar();
            if ( ch != L'=' ) {
                this->m_look = ch;
                return Token(TOK_SYM, ss.str());
            }

            ss << ch;
            return Token(TOK_SYM, ss.str());
        }

        return Token(TOK_SYM, ss.str());
    }

    virtual wchar_t getchar() {
        wchar_t ch = (wchar_t)this->m_stream.get();
        if ( this->m_stream.eof() )
            return 0;

        if ( ch == L'\n' ) {
            this->m_char = 0;
            this->m_line += 1;
        }
        else {
            this->m_char += 1;
        }

        return ch;
    }

    int m_line;
    int m_char;

    std::wistream &m_stream;
    std::list<Token> m_list;
    wchar_t m_look;
};

Tokenizer::Tokenizer(std::wistream &stream)
    : m_impl(new TokenizerImpl(stream)) {
}

Tokenizer::~Tokenizer() {
    if ( this->m_impl )
        delete this->m_impl;

    this->m_impl = NULL;
}

const Token Tokenizer::eat(bool ws, int n) {
    return this->m_impl->eat(ws, n);
}

const Token Tokenizer::peek(bool ws, int n) {
    return this->m_impl->peek(ws, n);
}

int Tokenizer::charAt() const {
    return this->m_impl->m_char;
}

int Tokenizer::lineAt() const {
    return this->m_impl->m_line;
}

Token::Token(int t, std::wstring v)
    : type(t)
    , value(v){

}
