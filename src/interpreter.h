#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "src/parser.h"
#include "src/icontext.h"

class RContext;
class ParseTree;
class Interpreter
{
public:
    Interpreter();
    virtual ~Interpreter();

    bool Execute(RContext& context, const ParseTree *tree);
};

class IValue {
public:
    static IValue *Create(ValueType type, std::wstring val);

    virtual ~IValue() { }

    ValueType getValueType() const {
        return this->_valueType;
    }

    virtual std::wstring toString() const = 0;

    virtual const IValue * HandleBinary(const std::wstring& op, const IValue *right) const = 0;
protected:
    ValueType _valueType;

    IValue(ValueType vt)
        : _valueType(vt) {
    }
};

class Definition {
public:
    static Definition *Create(const ParseTree *tree);

    virtual ~Definition() { }

    virtual bool Execute(RContext &context) const {
        if ( this->next != NULL )
            return this->next->Execute(context);

        return true;
    }

    virtual void writeTo(std::wostream &out) const = 0;
protected:
    Definition() : next((Definition *)0) { }

    const Definition *next;
};

class Pattern {
public:
    static Pattern *Create(const ParseTree *tree);

    virtual ~Pattern() { }

    virtual bool Execute(RContext &context) const {
        if ( this->next != NULL )
            return this->next->Execute(context);
        return true;
    }

    virtual void writeTo(std::wostream &out) const = 0;
protected:
    Pattern() : next((Pattern*)0) { }

    const Pattern *next;
};

class TExpression {
public:
    static TExpression *Create(const ParseTree *tree);

    virtual ~TExpression() { }

    virtual const IValue *Execute(RContext &context) const = 0;

    virtual ValueType getValueType(RContext &context) const = 0;
};

class TVariable {
public:
    static TVariable *Create(const ParseTree *tree);

    virtual ~TVariable() { }

    virtual RVariable *Execute(RContext &context) const = 0;

    virtual void writeTo(std::wostream &out, std::wstring decoration) const = 0;
};
#endif // INTERPRETER_H
