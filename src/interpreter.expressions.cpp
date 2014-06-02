#include "src/interpreter.h"
#include <string>

using namespace ::std;

class EVariable {
public:
    static EVariable *Create(const ParseTree *tree);

    virtual ~EVariable() { }

    virtual const IValue *GetValue(RContext &context) const = 0;
    virtual bool SetValue(RContext &context, const IValue * value) const = 0;
};

class ExpressionVariable : public EVariable {
public:
    static ExpressionVariable *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_VAR_REF ) {
            return NULL;
        }

        return new ExpressionVariable(tree->value);
    }

    virtual ~ExpressionVariable() { }

    const IValue *GetValue(RContext &context) const {
        // IVariable *var = context.GetVariable(this->variableName);
        // IObject *obj = var->getCurrent();

        // TODO:
        return NULL;
    }

    bool SetValue(RContext &context, const IValue *value) const {
        RVariable *var = context.GetVariable(this->variableName);

        // TODO:
        var ->setCurrent(NULL);

        return false;
    }

protected:
    wstring variableName;

    ExpressionVariable(const wstring &variableName)
        : variableName(variableName) {
    }
};

class ExpressionAttributeVariable : public EVariable {
public:
    static ExpressionAttributeVariable *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_ATTR ||
            tree->child == NULL ||
            tree->child->type != PT_VAR_REF ) {
            return NULL;
        }

        return new ExpressionAttributeVariable(tree->child->value, tree->value);
    }

    virtual ~ExpressionAttributeVariable() { }

    const IValue *GetValue(RContext &context) const {
        RVariable *var = context.GetVariable(this->variableName);
        if ( var == NULL )
            return NULL;

        RObject *obj = var->getCurrent();
        if ( obj == NULL )
            return NULL;

        const RValue *value = obj->GetAttribute(this->attributeName);
        IValue *val = IValue::Create(value->getValueType(), value->getValue());

        delete value; // FIXME: auto_ptr
        return val;
    }

    bool SetValue(RContext &context, const IValue *value) const {
        RVariable *var = context.GetVariable(this->variableName);
        if ( var == NULL )
            return false;

        RObject *obj = var->getCurrent();
        if ( obj == NULL )
            return false;

        RValue val(value->getValueType(), value->toString());
        return obj->SetAttribute(this->attributeName, &val);
    }

protected:
    wstring variableName;
    wstring attributeName;

    ExpressionAttributeVariable(const wstring &variableName, const wstring &attributeName)
        : variableName(variableName)
        , attributeName(attributeName) {
    }
};

EVariable *EVariable::Create(const ParseTree *tree) {
    if ( tree == NULL )
        return NULL;

    switch(tree->type) {
    case PT_ATTR:
        return ExpressionAttributeVariable::Create(tree);
    case PT_VAR_REF:
        return ExpressionVariable::Create(tree);
    }
    return NULL;
}

class VariableExpression : public TExpression {
public:
    static VariableExpression *Create(const ParseTree *tree) {
        if ( tree == NULL || tree->type != PT_VAR_REF )
            return NULL;

        return new VariableExpression(tree->value);
    }

    virtual ~VariableExpression() {
        if ( this->expr != NULL ) {
            delete this->expr;
            this->expr = NULL;
        }
    }

    IValue *Execute(RContext &context) const {
        // TODO:
        return IValue::Create(VT_UNKNOWN, L"");
        // if ( this->expr == NULL ) {
        //     IVariable *var = context.GetVariable(name);
        //     return
        // }
        // else
        //     return IValue::Create(VT_UNKNOWN, L"");
    }

    ValueType getValueType(RContext &context) const {
        // TODO:
        return VT_UNKNOWN;
        // IVariable *var = context.GetVariable(name);
        // if ( var == NULL )
        //    return IValue::Create(VT_UNKNOWN, L"");

        // if ( this->expr == NULL ) {
        //     IVariable *var = context.GetVariable(this->variableName);
        // }
    }
protected:
    std::wstring variableName;
    const TExpression *expr; // optional value to assign

    VariableExpression(const std::wstring &variableName)
        : variableName(variableName)
        , expr(NULL) {
    }

    VariableExpression(const std::wstring &variableName, const TExpression *expr)
        : variableName(variableName)
        , expr(NULL) {
    }
};

class AttributeExpression : public TExpression {
public:
    static AttributeExpression *Create(const ParseTree *tree) {
        if ( tree->type != PT_ATTR || tree->child == NULL || tree->child->type != PT_VAR_REF )
            return NULL;

        return new AttributeExpression(tree->value, tree->child->value);
    }

    virtual ~AttributeExpression() {
    }

    const IValue *Execute(RContext &context) const {
        RVariable *var = context.GetVariable(this->variableName);
        RObject *obj;

        if ( var == NULL || (obj = var->getCurrent()) == NULL )
            return IValue::Create(VT_UNKNOWN, L"");

        const RValue *value = obj->GetAttribute(this->attributeName);
        if ( value == NULL )
            return NULL;

        IValue *val = IValue::Create(value->getValueType(), value->getValue());
        delete value; // FIXME: auto_ptr

        return val;
    }

    ValueType getValueType(RContext &context) const {
        return VT_UNKNOWN;
        //IVariable *var = context.GetVariable(this->variableName);
        //IClass *cls = var->getClass();
        //IAttribute *attr = cls->GetAttribute(this->attributeName);
        //ValueType t = attr->getValueType();
        //delete attr;
        //delete cls;
        // // delete var - cleaned up within context
        //return t;
    }
protected:
    std::wstring variableName;
    std::wstring attributeName;

    AttributeExpression(const wstring &attributeName, const wstring &variableName)
        : variableName(variableName)
        , attributeName(attributeName) {
    }
};

class BinaryExpression : public TExpression {
public:
    static BinaryExpression *Create(const ParseTree *tree) {
        if ( tree->type != PT_BINOP )
            return NULL;

        return new BinaryExpression(
            TExpression::Create(tree->child),
            TExpression::Create(tree->next),
            tree->value);
    }

    const IValue *Execute(RContext &context) const {
        const IValue *lv, *rv;
        if ( (lv = this->left->Execute(context)) == NULL )
            return NULL;
        if ( (rv = this->right->Execute(context)) == NULL )
        {
            delete lv;
            return NULL;
        }

        const IValue *value = NULL;
        if ( lv->getValueType() != VT_UNKNOWN && rv->getValueType() != VT_UNKNOWN )
            value = lv->HandleBinary(this->op, rv);

        delete lv; // FIXME: auto_ptr
        delete rv;
        return value;
    }

    ValueType getValueType(RContext &context) const {
        return VT_UNKNOWN;
    }
protected:
    const TExpression *left;
    const TExpression *right;
    std::wstring op;

    BinaryExpression(const TExpression *left, const TExpression *right, wstring op)
        : left(left)
        , right(right)
        , op(op) {

    }
};

class UnaryExpression : public TExpression {
public:
    static UnaryExpression *Create(const ParseTree *tree) {
        return NULL;
    }

    const IValue *Execute(RContext &context) const {
        return NULL;
    }

    ValueType getValueType(RContext &context) const {
        return VT_UNKNOWN;
    }
protected:
    const TExpression *expr;
    std::wstring op;
};

class ConstantExpression : public TExpression {
public:
    static ConstantExpression *Create(const ParseTree *tree) {
        if ( tree == NULL )
            return NULL;

        IValue *val;
        if ( tree->type == PT_STRING ) {
            val = IValue::Create(VT_STRING, tree->value);
        }
        else if ( tree->type == PT_NUMBER ) {
            val = IValue::Create(VT_REAL, tree->value);
        }
        else if ( tree->type == PT_BOOL ) {
            val = IValue::Create(VT_BOOL, tree->value);
        }

        return new ConstantExpression(val);
    }

    const IValue *Execute(RContext &context) const {
        return this->value; // TODO: Return a copy
    }

    ValueType getValueType(RContext &context) const {
        return this->value->getValueType();
    }
protected:
    const IValue *value;

    ConstantExpression(const IValue *value)
        :value(value) {
    }
};

class AssignmentExpression : public TExpression {
public:
    static AssignmentExpression *Create(const ParseTree *tree) {
        if ( tree == NULL ||
             tree->type != PT_BINOP ||
             tree->value != L"=" ) {
            return NULL;
        }

        EVariable *variable = EVariable::Create(tree->child);
        if ( variable == NULL ) return NULL;

        TExpression *expr = TExpression::Create(tree->next);
        if ( expr == NULL ) {
            delete variable; // FIXME: auto_ptr
            return NULL;
        }

        return new AssignmentExpression(variable, expr);
    }

    virtual ~AssignmentExpression() {
    }

    const IValue *Execute(RContext &context) const {
        const IValue *val = this->expr->Execute(context);
        this->variable->SetValue(context, val); // TODO: return null if false
        return val;
    }

    ValueType getValueType(RContext &context) const {
        return VT_UNKNOWN;
    }
protected:
    const EVariable *variable;
    const TExpression *expr;

    AssignmentExpression(const EVariable *variable, const TExpression *expr)
        : variable(variable)
        , expr(expr) {
    }
};

TExpression *TExpression::Create(const ParseTree *tree) {
    if ( tree == NULL ) return NULL;

    switch(tree->type) {
        case PT_BINOP:
            if ( tree->value == L"=" )
                return AssignmentExpression::Create(tree);

            return BinaryExpression::Create(tree);
        case PT_UNOP:
            return UnaryExpression::Create(tree);

        case PT_VAR_REF:
            return VariableExpression::Create(tree);

        case PT_ATTR:
            return AttributeExpression::Create(tree);

        case PT_STRING:
        case PT_BOOL:
        case PT_NUMBER:
            return ConstantExpression::Create(tree);
    }

    return NULL;
}
