#include "interpreter.h"

#include <stdexcept>
#include <utility>
#include <vector>

using namespace ::std;

bool _Execute(RContext& context, const ParseTree *tree);

Interpreter::Interpreter() {
}

Interpreter::~Interpreter() {
}

/*
class Expression {
public:
    static Expression *Create(const ParseTree *tree);

    virtual ~Expression() { }

    virtual bool Execute(IContext &context) const {
        return this->next == NULL ? true : this->next->Execute(context);
    };

protected:
    Expression() : next(NULL) { }

    const Expression *next;
};

class ValueExpression {
public:
    static ValueExpression * Create(const ParseTree *);

    virtual wstring getValue(IContext &context) const = 0;
private:
};

class AttributeExpression : public ValueExpression {
public:
    static AttributeExpression *Create(const ParseTree *tree) {
        if ( tree->type != PT_ATTR || tree->child == NULL || tree->child->type != PT_VAR_REF )
            return NULL;

        return new AttributeExpression(tree->value, tree->child->value);
    }

    wstring getValue(IContext &context) const {
        IVariable *var = context.GetVariable(this->variable);
        if ( var == NULL )
            return L"";

        IObject *obj = var->getCurrent();
        if ( obj == NULL )
            return L"";

        return obj->GetAttribute(this->attribute);
    }

    bool setValue(IContext &context, const wstring& value) const {
        IVariable *var = context.GetVariable(this->variable);
        if ( var == NULL )
            return false;

        IObject *obj = var->getCurrent();
        if ( obj == NULL )
            return false;

        return obj->SetAttribute(this->attribute, value);
    }

protected:
    AttributeExpression(const wstring& attribute, const wstring& variable)
        : attribute(attribute)
        , variable(variable) {
    }

    const wstring attribute;
    const wstring variable;
};

class AssignmentExpression : public ValueExpression {
public:
    static AssignmentExpression *Create(const ParseTree *tree) {
        if ( tree->type != PT_BINOP || tree->value != L"=" )
            return NULL;

        return new AssignmentExpression(
            AttributeExpression::Create(tree->child),
            ValueExpression::Create(tree->next));
    }

    wstring getValue(IContext &context) const {
        wstring v = this->value->getValue(context);
        this->attribute->setValue(context, v);
        return v;
    }
protected:
    AssignmentExpression(const AttributeExpression *attribute, const ValueExpression *value)
        : attribute(attribute)
        , value(value) {
    }

    const AttributeExpression *attribute;
    const ValueExpression *value;
};

class BinaryExpression : public ValueExpression {
public:
    static BinaryExpression *Create(const ParseTree *tree) {
        if ( tree->type != PT_BINOP )
            return NULL;

        return new BinaryExpression(
            ValueExpression::Create(tree->child),
            ValueExpression::Create(tree->next));
    }

    wstring getValue(IContext &context) const {
        return lhs->getValue(context);
    }
protected:
    BinaryExpression(const ValueExpression *lhs, const ValueExpression *rhs)
        : lhs(lhs)
        , rhs(rhs) {
    }

    const ValueExpression *lhs;
    const ValueExpression *rhs;
};

class PatternExpression : public Expression {
public:
    static PatternExpression *Create(const ParseTree *tree);

    virtual ~PatternExpression() {
        if ( this->next != NULL )
            delete this->next;

        this->next = NULL;
    }

    virtual bool Execute(IContext &context) const {
        return this->next == NULL ? Expression::Execute(context) : this->next->Execute(context);
    }
protected:
    PatternExpression() : next(NULL) { }

    const PatternExpression *next;
};

class VariableExpression : public PatternExpression {
public:
    static VariableExpression *Create(const ParseTree *);

    virtual IVariable *getVariable(IContext& context) const {
        return context.GetVariable(this->name);
    };

    bool Execute(IContext &context) const {
        return (context.GetVariable(this->name) != NULL) && PatternExpression::Execute(context);
    }

protected:
    VariableExpression(const wstring &name)
        : name(name) {
    }

    const wstring name;
};

class VariableDeclarationExpression : public VariableExpression {
public:
    static VariableDeclarationExpression *Create(const ParseTree *tree) {
        if(tree->type != PT_VAR_DEF)
            return NULL;

        if (tree->child == NULL ||
            tree->child->type != PT_CLASS )
            throw std::runtime_error("Expected class specifier!");

        return new VariableDeclarationExpression(tree->value, tree->child->value);
    }

    VariableDeclarationExpression(const wstring &name, const wstring &className)
        : VariableExpression(name)
        , className(className) {
    }

    bool Execute(IContext &context) const {
        bool created;
        IVariable *var = this->getOrCreateVariable(context, created);
        if ( !var )
            return false;

        return (created ? var->MoveFirst() : var->MoveNext()) && PatternExpression::Execute(context);
    }

    IVariable * getVariable(IContext &context) const {
        bool created;
        return getOrCreateVariable(context, created);
    }

private:

    IVariable *getOrCreateVariable(IContext &context, bool &created) const {
        IClass* c = context.FindClass(this->className);
        if ( c == NULL )
            return NULL;

        IVariable *var = context.GetVariable(this->name);
        if ( var != NULL ) {
            created = false;
            if ( var->IsSubclassOf(c) ) { // FIXME: Move sublass validation to class
            }
            else {
                return NULL;
            }
        }
        else {
            created = true;
            var = context.CreateVariable(this->name, c);
        }

        delete c;
        return var;
    }

    const wstring className;
};

class LinkVariableExpression : public VariableExpression {
public:
    static LinkVariableExpression *Create(const ParseTree *tree) {
        if ( tree == NULL ) return NULL;
        if ( tree->type != PT_LINK )
            throw runtime_error("Link expression required!");

        return new LinkVariableExpression(VariableExpression::Create(tree->child), tree->value, tree->child->next->value);
    }

    bool Execute(IContext &context) const {
        bool created;
        IVariable * var = this->getOrCreateVariable(context, created);

        return created ? var->MoveFirst() : var->MoveNext();
    }

    IVariable *getVariable(IContext &context) const {
        bool created;
        return this->getOrCreateVariable(context, created);
    }
private:
    const VariableExpression *sourceVariable;
    const wstring linkName;

    LinkVariableExpression(const VariableExpression *sourceVariable, const wstring& linkName, const wstring& name)
        : VariableExpression(name)
        , sourceVariable(sourceVariable)
        , linkName(linkName) {
    }

    IVariable *getOrCreateVariable(IContext &context, bool &created) const {
        IIterable * it = context.GetLinkedObjects(this->sourceVariable->getVariable(context)->getCurrent(), this->linkName);
        IClass *linkedClass = it->getClass();

        IVariable * var = context.GetVariable(this->name);
        if ( var != NULL ) {
            created = false;
            if( !var->IsSubclassOf(linkedClass) ) {
                throw runtime_error("Variable with incompatible type!");
            }

            var->setSource(it);
        }
        else {
            created = true;
            var = context.CreateVariable(this->name, linkedClass);
            var->setSource(it);
        }

        delete linkedClass;
        return var;
    }
};

VariableExpression * VariableExpression::Create(const ParseTree *tree) {
    if ( tree == NULL ) return NULL;

    switch ( tree->type ) {
    case PT_VAR_DEF:
        return VariableDeclarationExpression::Create(tree);
    case PT_LINK:
        return LinkVariableExpression::Create(tree);
    case PT_VAR_REF:
        return new VariableExpression(tree->value);
    }

    throw runtime_error("Variable expression required!");
    return NULL;
}

class CreationExpression : public PatternExpression {
public:
    static CreationExpression *Create(const ParseTree *tree) {
        if ( tree->type != PT_VAR_DEF ||
            tree->child == NULL ||
            tree->child->type != PT_CLASS)
            return NULL;

        CreationExpression *expr = new CreationExpression(tree->value, tree->child->value);

        // TODO: Move initializer to tree->child->next
        if ( tree->child->child == NULL )
            return expr;

        const ParseTree *pt = tree->child->child;
        if ( pt->type != PT_INITIALIZER )
            throw runtime_error("Invalid member initializer!");

        for ( pt = pt->child;
              pt != NULL;
              pt = pt->next ) {
            ValueExpression *vexpr = ValueExpression::Create(pt->child);
            expr->members.push_back(make_pair<wstring, const ValueExpression*>(pt->value, vexpr));
        }

        return expr;
    }

    CreationExpression(wstring variableName, wstring className)
        : variableName(variableName)
        , className(className) {
    }

    virtual ~CreationExpression() {
    }

    bool Execute(IContext &context) const {
        IClass *cls = context.FindClass(this->className);
        if ( cls == NULL )
            return false;

        IVariable *var = context.GetVariable(this->variableName);
        if ( var == NULL ) {
            var = context.CreateVariable(this->variableName, cls);
        }
        else if ( !var->IsSubclassOf(cls) ) {
            return false;
        }

        IObject *obj = context.CreateObject();
        if ( obj == NULL )
            return false;

        obj->SpecifyType(cls);

        var->setCurrent(obj);

        for ( vector< pair< wstring, const ValueExpression *> >::const_iterator it = this->members.begin();
              it != this->members.end();
              it++ ) {

            obj->AddAttribute(it->first, it->second->getValue(context));
        }

        return PatternExpression::Execute(context);
    }
private:
    wstring variableName;
    wstring className;
    vector< pair< wstring, const ValueExpression *> > members;
};

class CreateLinkExpression : public PatternExpression {
public:
    static CreateLinkExpression *Create(const ParseTree *tree) {
        if ( tree->type != PT_LINK )
            return NULL;

        return new CreateLinkExpression(tree->child->value, tree->value, tree->child->next->value);
    }

    CreateLinkExpression(const wstring &sourceVariable, const wstring &association, const wstring &variable)
        : sourceVariable(sourceVariable)
        , association(association)
        , variable(variable) {
    }

    virtual ~CreateLinkExpression() {
    }

    bool Execute(IContext &context) const {
        IVariable *source = context.GetVariable(this->sourceVariable);
        IVariable *target = context.GetVariable(this->variable);
        if ( source == NULL || target == NULL )
            return false;

        if(!source->getCurrent()->AddLink(this->association, target->getCurrent()))
            return false;

        return PatternExpression::Execute(context);
    }

private:
    wstring sourceVariable;
    wstring association;
    wstring variable;
};

class IteratorExpression : public PatternExpression {
public:
    static IteratorExpression* Create(const ParseTree *tree) {
        if ( tree->type != PT_DECORATION || tree->value != L"**" )
            return NULL;

        VariableExpression *over = VariableExpression::Create(tree->child);
        if ( over == NULL )
            return NULL;

        return new IteratorExpression(over);
    }

    IteratorExpression(const VariableExpression *variable)
        : variable(variable) {
    }

    virtual ~IteratorExpression() {
        if ( this->variable != NULL )
            delete this->variable;

        this->variable = NULL;
    }

    bool Execute(IContext &context) const {
        IVariable *var = this->variable->getVariable(context);
        if(var == NULL || !var->MoveFirst())
            return false;

        // returns true if at least one iteration has been completed
        bool rv = false;
        do {
            rv = this->next->Execute(context) || rv;
        } while ( var->MoveNext() );

        return rv;
    }

private:
    const VariableExpression *variable;
    const Expression *body;
};

class ValuePatternExpression : public PatternExpression {
public:
    ValuePatternExpression(const ValueExpression *expr)
        : expression(expr) {
    }

    bool Execute(IContext &context) const {
        return this->expression->getValue(context) != L"false";
    }

protected:
    const ValueExpression *expression;
};

PatternExpression * PatternExpression::Create(const ParseTree *tree) {
    if ( tree == NULL )
        return NULL;

    PatternExpression *expr = NULL;
    switch(tree->type) {
    case PT_VAR_DEF:
        expr = VariableDeclarationExpression::Create(tree);
        break;
    case PT_DECORATION:
        {
            if ( tree->child != NULL ) {
                if ( tree->value == L"!") {
                    if ( tree->child->type == PT_LINK )
                        expr = CreateLinkExpression::Create(tree->child);
                    else if ( tree->child->type == PT_VAR_DEF )
                        expr = CreationExpression::Create(tree->child);
                }
                else if ( tree->value == L"**" || tree->value == L"**+" ) {
                    expr = IteratorExpression::Create(tree);
                }
            }
        }
        break;
    case PT_EXPRESSION:
        expr = new ValuePatternExpression(ValueExpression::Create(tree->child));
        break;
    }

    if ( expr == NULL )
        throw runtime_error("Invalid pattern expression!");

    expr->next = PatternExpression::Create(tree->next);
    return expr;
}
*/
bool Interpreter::Execute(RContext& context, const ParseTree *tree) {
    Definition *expr = Definition::Create(tree);
    bool rv = expr->Execute(context);
    delete expr;
    return rv;
}
