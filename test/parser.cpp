#include <gtest/gtest.h>
#include "src/parser.h"
#include <sstream>

TEST(ParserTest, Simple) {
    std::wstringstream ss(L"**x:X x.**aa.y y.**bb.z x.!cc.z");
    const ParseTree *tree = Parser(ss).parse(), *c, *t;
    ASSERT_NE((const ParseTree *)NULL, tree);

    EXPECT_EQ(PT_PATTERN, tree->type);
    EXPECT_EQ(L"", tree->value);
    EXPECT_EQ((const ParseTree *)NULL, tree->next);
    ASSERT_NE((const ParseTree *)NULL, tree->child);

    t = tree->child;
    EXPECT_EQ(PT_DECORATION, t->type);
    EXPECT_EQ(L"**", t->value);
    ASSERT_NE((const ParseTree *)NULL, t->child);
    ASSERT_NE((const ParseTree *)NULL, t->next);

    c = t->child;
    EXPECT_EQ(PT_VAR_DEF, c->type);
    EXPECT_EQ(L"x", c->value);
    EXPECT_EQ((const ParseTree *)NULL, c->next);
    ASSERT_NE((const ParseTree *)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_CLASS, c->type);
    EXPECT_EQ(L"X", c->value);
    ASSERT_EQ((const ParseTree *)NULL, c->child);
    ASSERT_EQ((const ParseTree *)NULL, c->next);

    t = t->next;
    EXPECT_EQ(PT_DECORATION, t->type);
    EXPECT_EQ(L"**", t->value);
    ASSERT_NE((const ParseTree *)NULL, t->child);
    ASSERT_NE((const ParseTree *)NULL, t->next);

    c = t->child;
    EXPECT_EQ(PT_LINK, c->type);
    EXPECT_EQ(L"aa", c->value);
    EXPECT_EQ((const ParseTree *)NULL, c->next);
    ASSERT_NE((const ParseTree *)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_VAR_REF, c->type);
    EXPECT_EQ(L"x", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->child);
    ASSERT_NE((ParseTree *)NULL, c->next);

    c = c->next;
    EXPECT_EQ(PT_VAR_DEF, c->type);
    EXPECT_EQ(L"y", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    EXPECT_EQ((ParseTree *)NULL, c->child);

    t = t->next;
    EXPECT_EQ(PT_DECORATION, t->type);
    EXPECT_EQ(L"**", t->value);
    ASSERT_NE((const ParseTree *)NULL, t->child);
    ASSERT_NE((const ParseTree *)NULL, t->next);

    c = t->child;
    EXPECT_EQ(PT_LINK, c->type);
    EXPECT_EQ(L"bb", c->value);
    EXPECT_EQ((const ParseTree *)NULL, c->next);
    ASSERT_NE((const ParseTree *)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_VAR_REF, c->type);
    EXPECT_EQ(L"y", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->child);
    ASSERT_NE((ParseTree *)NULL, c->next);

    c = c->next;
    EXPECT_EQ(PT_VAR_DEF, c->type);
    EXPECT_EQ(L"z", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    EXPECT_EQ((ParseTree *)NULL, c->child);

    t = t->next;
    EXPECT_EQ(PT_DECORATION, t->type);
    EXPECT_EQ(L"!", t->value);
    EXPECT_EQ((const ParseTree *)NULL, t->next);
    ASSERT_NE((const ParseTree *)NULL, t->child);

    c = t->child;
    EXPECT_EQ(PT_LINK, c->type);
    EXPECT_EQ(L"cc", c->value);
    EXPECT_EQ((const ParseTree *)NULL, c->next);
    ASSERT_NE((const ParseTree *)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_VAR_REF, c->type);
    EXPECT_EQ(L"x", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->child);
    ASSERT_NE((ParseTree *)NULL, c->next);

    c = c->next;
    EXPECT_EQ(PT_VAR_DEF, c->type);
    EXPECT_EQ(L"z", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    EXPECT_EQ((ParseTree *)NULL, c->child);

    delete tree;
}

TEST(ParserTest, ParseClassDeclaration) {
    std::wstringstream ss(L"class Person { name:string, family:string }");
    const ParseTree* tree = Parser(ss).parse(), *c, *t;
    ASSERT_NE((const ParseTree*)NULL, tree);
    EXPECT_EQ(PT_CLASS_DEF, tree->type);
    EXPECT_EQ(L"Person", tree->value);
    EXPECT_EQ((ParseTree *)NULL, tree->next);
    ASSERT_NE((ParseTree *)NULL, tree->child);

    t = tree->child;
    ASSERT_NE((const ParseTree*)NULL, t);
    EXPECT_EQ(PT_ATTR, t->type);
    EXPECT_EQ(L"name", t->value);

    c = t->child;
    ASSERT_NE((const ParseTree*)NULL, c);
    EXPECT_EQ(PT_CLASS, c->type);
    EXPECT_EQ(L"string", c->value);

    t = t->next;
    ASSERT_NE((const ParseTree*)NULL, t);
    EXPECT_EQ(PT_ATTR, t->type);
    EXPECT_EQ(L"family", t->value);

    c = t->child;
    ASSERT_NE((const ParseTree*)NULL, c);
    EXPECT_EQ(PT_CLASS, c->type);
    EXPECT_EQ(L"string", c->value);
}

TEST(ParserTest, ParseObjectCreation) {
    std::wstringstream ss(L"!p:Person { p.name = \"first\", p.family = \"last\" }");
    const ParseTree *tree = Parser(ss).parse(), *t, *e, *c;
    ASSERT_NE((ParseTree*)NULL, tree);

    EXPECT_EQ(PT_PATTERN, tree->type);
    EXPECT_EQ(L"", tree->value);
    EXPECT_EQ((ParseTree *)NULL, tree->next);
    ASSERT_NE((ParseTree *)NULL, tree->child);

    t = tree->child;
    EXPECT_EQ(PT_DECORATION, t->type);
    EXPECT_EQ(L"!", t->value);
    ASSERT_NE((ParseTree*)NULL, t->next);
    ASSERT_NE((ParseTree*)NULL, t->child);

    t = t->child;
    EXPECT_EQ(PT_VAR_DEF, t->type);
    EXPECT_EQ(L"p", t->value);
    EXPECT_EQ((ParseTree*)NULL, t->next);
    ASSERT_NE((ParseTree*)NULL, t->child);

    t = t->child;
    EXPECT_EQ(PT_CLASS, t->type);
    EXPECT_EQ(L"Person", t->value);
    EXPECT_EQ((ParseTree*)NULL, t->next);
    EXPECT_EQ((ParseTree*)NULL, t->child);

    e = tree->child->next;
    EXPECT_EQ(PT_EXPRESSION, e->type);
    EXPECT_EQ(L"", e->value);
    ASSERT_NE((ParseTree*)NULL, e->next);
    ASSERT_NE((ParseTree*)NULL, e->child);

    t = e->child;
    EXPECT_EQ(PT_BINOP, t->type);
    EXPECT_EQ(L"=", t->value);
    ASSERT_NE((ParseTree*)NULL, t->child); // left
    ASSERT_NE((ParseTree*)NULL, t->next); // right

    c = t->child;
    EXPECT_EQ(PT_ATTR, c->type);
    EXPECT_EQ(L"name", c->value);
    EXPECT_EQ((ParseTree*)NULL, c->next);
    ASSERT_NE((ParseTree*)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_VAR_REF, c->type);
    EXPECT_EQ(L"p", c->value);
    EXPECT_EQ((ParseTree*)NULL, c->child);
    EXPECT_EQ((ParseTree*)NULL, c->next);

    c = t->next;
    EXPECT_EQ(PT_STRING, c->type);
    EXPECT_EQ(L"first", c->value);
    EXPECT_EQ((ParseTree*)NULL, c->next);
    EXPECT_EQ((ParseTree*)NULL, c->child);

    e = e->next;
    EXPECT_EQ(PT_EXPRESSION, e->type);
    EXPECT_EQ(L"", e->value);
    EXPECT_EQ((ParseTree*)NULL, e->next);
    ASSERT_NE((ParseTree*)NULL, e->child);

    t = e->child;
    EXPECT_EQ(PT_BINOP, t->type);
    EXPECT_EQ(L"=", t->value);
    ASSERT_NE((ParseTree*)NULL, t->child); // left
    ASSERT_NE((ParseTree*)NULL, t->next); // right

    c = t->child;
    EXPECT_EQ(PT_ATTR, c->type);
    EXPECT_EQ(L"family", c->value);
    EXPECT_EQ((ParseTree*)NULL, c->next);
    ASSERT_NE((ParseTree*)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_VAR_REF, c->type);
    EXPECT_EQ(L"p", c->value);
    EXPECT_EQ((ParseTree*)NULL, c->next);
    EXPECT_EQ((ParseTree*)NULL, c->child);

    c = t->next;
    EXPECT_EQ(PT_STRING, c->type);
    EXPECT_EQ(L"last", c->value);
    EXPECT_EQ((ParseTree*)NULL, c->next);
    EXPECT_EQ((ParseTree*)NULL, c->child);

    delete tree;
}

TEST(ParserTest, ParseAssociationDefinition) {
    std::wstringstream ss(L"associate members:Person with family:Family");
    const ParseTree *pt = Parser(ss).parse(), *t, *c;
    ASSERT_NE((ParseTree *)NULL, pt);

    EXPECT_EQ(PT_ASSOC_DEF, pt->type);
    EXPECT_EQ(L"", pt->value);
    EXPECT_EQ((ParseTree *)NULL, pt->next);
    ASSERT_NE((ParseTree *)NULL, pt->child);

    t = pt->child;
    EXPECT_EQ(PT_ASSOC, t->type);
    EXPECT_EQ(L"members", t->value);
    ASSERT_NE((ParseTree *)NULL, t->next);
    ASSERT_NE((ParseTree *)NULL, t->child);

    c = t->child;
    EXPECT_EQ(PT_CLASS, c->type);
    EXPECT_EQ(L"Person", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->child);
    EXPECT_EQ((ParseTree *)NULL, c->next);

    t = t->next;
    EXPECT_EQ(PT_ASSOC, t->type);
    EXPECT_EQ(L"family", t->value);
    EXPECT_EQ((ParseTree *)NULL, t->next);
    ASSERT_NE((ParseTree *)NULL, t->child);

    c = t->child;
    EXPECT_EQ(PT_CLASS, c->type);
    EXPECT_EQ(L"Family", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->child);
    EXPECT_EQ((ParseTree *)NULL, c->next);

    delete pt;
}

TEST(ParserTest, ParseLinkCreation) {
    std::wstringstream ss(L"p.!family.f");
    const ParseTree *pt = Parser(ss).parse(), *t, *c;
    ASSERT_NE((ParseTree *)NULL, pt);

    EXPECT_EQ(PT_PATTERN, pt->type);
    EXPECT_EQ(L"", pt->value);
    EXPECT_EQ((ParseTree *)NULL, pt->next);
    ASSERT_NE((ParseTree *)NULL, pt->child);

    t = pt->child;
    EXPECT_EQ(PT_DECORATION, t->type);
    EXPECT_EQ(L"!", t->value);
    EXPECT_EQ((ParseTree *)NULL, t->next);
    ASSERT_NE((ParseTree *)NULL, t->child);

    c = t->child;
    EXPECT_EQ(PT_LINK, c->type);
    EXPECT_EQ(L"family", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    ASSERT_NE((ParseTree *)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_VAR_REF, c->type);
    EXPECT_EQ(L"p", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->child);
    ASSERT_NE((ParseTree *)NULL, c->next);

    c = c->next;
    EXPECT_EQ(PT_VAR_DEF, c->type);
    EXPECT_EQ(L"f", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    EXPECT_EQ((ParseTree *)NULL, c->child);
}

TEST(ParserTest, ParseExpressionBlock) {
    std::wstringstream ss(L"**p:Person {p.name == \"first\"}");
    const ParseTree *tree = Parser(ss).parse(), *t, *c;
    ASSERT_NE((ParseTree *)NULL, tree);
    ASSERT_NE((ParseTree *)NULL, tree->child);

    t = tree->child;
    EXPECT_EQ(PT_DECORATION, t->type);
    EXPECT_EQ(L"**", t->value);
    EXPECT_NE((ParseTree *)NULL, t->child);
    ASSERT_NE((ParseTree *)NULL, t->next);

    t = t->next;
    EXPECT_EQ(PT_EXPRESSION, t->type);
    EXPECT_EQ(L"", t->value);
    EXPECT_EQ((ParseTree *)NULL, t->next);
    ASSERT_NE((ParseTree *)NULL, t->child);

    t = t->child;
    EXPECT_EQ(PT_BINOP, t->type);
    EXPECT_EQ(L"==", t->value);
    ASSERT_NE((ParseTree *)NULL, t->child);
    ASSERT_NE((ParseTree *)NULL, t->next);

    c = t->child;
    EXPECT_EQ(PT_ATTR, c->type);
    EXPECT_EQ(L"name", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    ASSERT_NE((ParseTree *)NULL, c->child);

    c = c->child;
    EXPECT_EQ(PT_VAR_REF, c->type);
    EXPECT_EQ(L"p", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    EXPECT_EQ((ParseTree *)NULL, c->child);

    c = t->next;
    EXPECT_EQ(PT_STRING, c->type);
    EXPECT_EQ(L"first", c->value);
    EXPECT_EQ((ParseTree *)NULL, c->next);
    EXPECT_EQ((ParseTree *)NULL, c->child);

    delete tree;
}

TEST(ParserTest, ParseContextBlock) {
    // std::wstringstream ss(L"**p:Person [*f:Family {f.name = p.family} | !f:Family { name = p.family }] p.!family.f");
    std::wstringstream ss(L"**p:Person [*f:Family | !f:Family { f.name = p.family }] p.!family.f");

    const ParseTree *tree = Parser(ss).parse(), *t, *c;
    ASSERT_NE((ParseTree*)NULL, tree);

    t = tree;
    EXPECT_EQ(PT_PATTERN, t->type);
    EXPECT_EQ(L"", t->value);
    EXPECT_EQ((ParseTree*)NULL, t->next);
    ASSERT_NE((ParseTree*)NULL, t->child);

    t = t->child;
    ASSERT_NE((ParseTree*)NULL, t->next);

    t = t->next;
    EXPECT_EQ(PT_CONTEXT, t->type);
    EXPECT_EQ(L"", t->value);
    ASSERT_NE((ParseTree*)NULL, t->next);
    ASSERT_NE((ParseTree*)NULL, t->child);

    c = t->child;
    EXPECT_EQ(PT_ALTERNATIVE, c->type);
    EXPECT_EQ(L"", c->value);
    EXPECT_NE((ParseTree*)NULL, c->child);
    ASSERT_NE((ParseTree*)NULL, c->next);

    c = c->next;
    EXPECT_EQ(PT_ALTERNATIVE, c->type);
    EXPECT_EQ(L"", c->value);
    EXPECT_NE((ParseTree*)NULL, c->child);
    EXPECT_EQ((ParseTree*)NULL, c->next);

    t = t->next;
    EXPECT_EQ((ParseTree*)NULL, t->next);
    delete tree;
}
