#ifndef PARSER_H
#define PARSER_H

#include <istream>
#include "tokenizer.h"

enum ParseTreeType {
    PT_PATTERN,
    PT_DECORATION,
    PT_VAR_DEF,
    PT_CLASS,
    PT_LINK,
    PT_VAR_REF, // 5
    PT_ATTR,
    PT_CLASS_DEF,
    PT_INITIALIZER,
    PT_STRING,
    PT_ASSOC, // 10
    PT_ASSOC_DEF,
    PT_CONTEXT,
    PT_ALTERNATIVE,
    PT_EXPRESSION,
    PT_BINOP,  //15
    PT_NUMBER,
    PT_BOOL,
    PT_UNOP
};

struct ParseTree {
    ParseTree(ParseTreeType t, std::wstring v, const ParseTree *c = NULL);
    ~ParseTree();

    ParseTreeType type;
    std::wstring value;
    const ParseTree *child;
    const ParseTree *next;
};

class ParserImpl;
class Parser
{
    public:
        /** Create a parser instance
        */
        Parser(std::wistream &stream);

        /** Cleanup
         */
        virtual ~Parser();

        /** Creates a parse tree from given source stream
         */
        const ParseTree * parse();
    protected:
    private:
        ParserImpl *_impl;
};

#endif // PARSER_H
