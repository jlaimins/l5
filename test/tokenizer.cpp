#include <gtest/gtest.h>
#include "src/tokenizer.h"

#include <sstream>

TEST(TokenizerTest, Simple) {
    std::wstringstream ss(L"\nid  == 12+4\"abc\"(*) *+** **+");
    Tokenizer *t = new Tokenizer(ss);
    EXPECT_EQ(TOK_WS, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_ID, t->peek().type);
    EXPECT_EQ(L"id", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_WS, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L"==", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_WS, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_NUM, t->peek().type);
    EXPECT_EQ(L"12", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L"+", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_NUM, t->peek().type);
    EXPECT_EQ(L"4", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_STR, t->peek().type);
    EXPECT_EQ(L"abc", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L"(", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L"*", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L")", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_WS, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L"*+", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L"**", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_WS, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_SYM, t->peek().type);
    EXPECT_EQ(L"**+", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_EOF, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);

    t->eat();
    EXPECT_EQ(TOK_EOF, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);
    delete t;
}

TEST(TokenizerTest, Whitespace) {
    std::wstringstream ss(L"1\n2\n3\n4\n");

    Tokenizer *t = new Tokenizer(ss);
    EXPECT_EQ(TOK_NUM, t->peek().type);
    EXPECT_EQ(L"1", t->peek().value);
    t->eat(true);

    EXPECT_EQ(TOK_NUM, t->peek().type);
    EXPECT_EQ(L"2", t->peek().value);
    t->eat(true);

    EXPECT_EQ(TOK_NUM, t->peek().type);
    EXPECT_EQ(L"3", t->peek().value);
    t->eat(true);

    EXPECT_EQ(TOK_NUM, t->peek().type);
    EXPECT_EQ(L"4", t->peek().value);
    t->eat(true);

    EXPECT_EQ(TOK_EOF, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);

    delete t;
}



TEST(TokenizerTest, Lookahead) {
    std::wstringstream ss(L"1\n2\n3\n4\n");
    Tokenizer *t = new Tokenizer(ss);
    EXPECT_EQ(TOK_EOF, t->peek(9).type);
    EXPECT_EQ(L"", t->peek(9).value);
    EXPECT_EQ(TOK_NUM, t->peek(7).type);
    EXPECT_EQ(L"4", t->peek(7).value);
    t->eat(2);
    EXPECT_EQ(TOK_NUM, t->peek().type);
    EXPECT_EQ(L"2", t->peek().value);
    EXPECT_EQ(TOK_NUM, t->peek(5).type);
    EXPECT_EQ(L"4", t->peek(5).value);
    EXPECT_EQ(TOK_EOF, t->peek(7).type);
    EXPECT_EQ(L"", t->peek(7).value);
    t->eat(7);
    EXPECT_EQ(TOK_EOF, t->peek().type);
    EXPECT_EQ(L"", t->peek().value);
    EXPECT_EQ(TOK_EOF, t->peek(7).type);
    EXPECT_EQ(L"", t->peek(7).value);
    delete t;
}

