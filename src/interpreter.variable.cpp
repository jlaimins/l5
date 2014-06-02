
#include "src/interpreter.h"

using namespace ::std;
class Variable : public TVariable {
public:
    static Variable *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_VAR_REF ) {
            return NULL;
        }

        return new Variable(tree->value);
    }

    RVariable *Execute(RContext &context) const {
        return context.GetVariable(this->name);
    }

    void writeTo(wostream &out, wstring decoration) const {
        out << decoration << this->name;
    }
protected:
    wstring name;

    Variable(const wstring &name)
        : name(name) {
    }
};

class VariableDeclaration : public TVariable {
public:
    static VariableDeclaration *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_VAR_DEF ||
            tree->child == NULL ||
            tree->child->type != PT_CLASS ) {
            return NULL;
        }

        return new VariableDeclaration(tree->value, tree->child->value);
    }

    RVariable *Execute(RContext &context) const {
        RClass *cls = context.FindClass(this->className);
        if ( cls == NULL )
            return NULL;

        RVariable *var = context.GetVariable(this->variableName);
        if ( var == NULL ) {
            var = context.CreateVariable(this->variableName, cls);
        }
        else {
            var->IsSubclassOf(cls);
        }

        delete cls; // TODO: auto_ptr
        if ( var->MoveFirst() )
            return var;

        return NULL; // FIXME: Some other way to notify parent about var->MoveFirst() value?
    }

    void writeTo(wostream &out, wstring decoration) const {
        out << decoration << this->variableName << L":" << this->className;
    }
protected:
    wstring variableName;
    wstring className;

    VariableDeclaration(const wstring &variableName, const wstring &className)
        : variableName(variableName)
        , className(className) {
    }
};

class LinkVariable : public TVariable {
public:
    static LinkVariable *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_LINK ||
            tree->child == NULL ||
            tree->child->type != PT_VAR_REF ||
            tree->child->next == NULL ||
            tree->child->next->type != PT_VAR_DEF ) {
            return NULL;
        }

        return new LinkVariable(TVariable::Create(tree->child), tree->value, tree->child->next->value);
    }

    RVariable *Execute(RContext &context) const {
        RVariable *source = this->source->Execute(context);
        if ( source == NULL )
            return NULL;

        RObject *cur = source->getCurrent();
        RIterable * it = context.GetLinkedObjects(cur, this->linkName);
        if ( it == NULL ) {
            return NULL;
        }

        RClass *linkedClass = it->getClass();
        if ( linkedClass == NULL ) {
            delete cur;
            delete it;
            return NULL;
        }

        RVariable * var = context.GetVariable(this->variableName);
        if ( var != NULL ) {
            if( !var->IsSubclassOf(linkedClass) ) {
                delete cur;
                delete linkedClass;
                delete it;
                return NULL;
            }

            var->setSource(it);
        }
        else {
            var = context.CreateVariable(this->variableName, linkedClass);
            var->setSource(it);
        }

        delete linkedClass;
        if ( var->MoveFirst() )
            return var;
        return NULL; // FIXME: Some better way to notify parent about MoveFirst value?
    }

    void writeTo(wostream &out, wstring decoration) const {
        this->source->writeTo(out, L"");
        out << L"." << decoration << this->linkName << L"." << this->variableName;
    }
protected:
    const TVariable *source;
    wstring variableName;
    wstring linkName;

    LinkVariable(const TVariable *source, const wstring &link, const wstring &name)
        : source(source)
        , variableName(name)
        , linkName(link) {
    }
};

TVariable *TVariable::Create(const ParseTree *tree) {
    if ( tree == NULL )
        return NULL;

    switch(tree->type) {
    case PT_LINK:
        return LinkVariable::Create(tree);
    case PT_VAR_DEF:
        return VariableDeclaration::Create(tree);
    case PT_VAR_REF:
        return Variable::Create(tree);
    }

    return NULL;
}
