#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <istream>

enum TokenType {
    TOK_ERR = -1,
    TOK_EOF = 0,
    TOK_WS  = 1,
    TOK_SYM,
    TOK_ID,
    TOK_NUM,
    TOK_STR,
    TOK_CLASS,
    TOK_ASSOC,
    TOK_BOOL
};
struct Token {
    Token(int type, std::wstring value);
    int type;
    std::wstring value;
};

class TokenizerImpl;

class Tokenizer
{
    public:
        /** Creates a new Tokenizer instance
         */
        Tokenizer(std::wistream &);

        /** Cleanup
         */
        virtual ~Tokenizer();

        /** Peek @ref n tokens ahead
         * @param ws: Whether to skip whitespace tokens
         * @param n: Tokens ahead to peek
         */
        const Token peek(bool ws, int n = 1);

        const Token peek(int n = 1) {
            return this->peek(false, n);
        }

        /** Consume @ref n tokens
         * @param ws: Whether to skip whitespace tokens
         * @param n: Number of tokens to consume
         * @return the next token
         */
        const Token eat(bool ws, int n = 1);

        const Token eat(int n = 1) {
            return this->eat(false, n);
        }

        int lineAt() const;
        int charAt() const;
    protected:
    private:
        TokenizerImpl* m_impl;
};

#endif // TOKENIZER_H
