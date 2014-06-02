#include <src/interpreter.h>
#include <iostream>
#include <vector>

using namespace ::std;

class VariableDeclarationPattern : public Pattern {
public:
    static VariableDeclarationPattern *Create(const ParseTree *tree) {
        if ( tree == NULL || tree->type != PT_VAR_DEF || tree->child == NULL || tree->child->type != PT_CLASS)
            return NULL;

        return new VariableDeclarationPattern(tree->value, tree->child->value);
    }

    virtual ~VariableDeclarationPattern() {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG

        RClass *cls = context.FindClass(this->className);
        if ( cls == NULL ) {
#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG
            return false;
        }


        bool rv = false;
        RVariable *var = context.GetVariable(this->variableName);
        if ( var != NULL ) {
            rv = var->IsSubclassOf(cls);
            if ( rv )
                rv = var->MoveNext();
        }
        else { // if ( var == NULL )
            var = context.CreateVariable(this->variableName, cls);
            rv = var->MoveFirst();
        }

        delete cls;
        #if DEBUG
        std::wcout << (rv ? L"FAILED: " : L"SUCCESS: ");
        this->writeTo(std::wcout);
        #endif // DEBUG

        return rv && Pattern::Execute(context);
    }

    void writeTo(wostream &out) const {
        out << this->variableName << L":" << this->className << endl;
    }
protected:
    wstring variableName;
    wstring className;

    VariableDeclarationPattern(const wstring &variableName, const wstring &className)
        : variableName(variableName)
        , className(className) {
    }
};

class LinkVariableDeclarationPattern : public Pattern {
public:
    static LinkVariableDeclarationPattern *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_LINK ||
            tree->child == NULL ||
            tree->child->type != PT_VAR_REF ||
            tree->child->next == NULL ||
            tree->child->next->type != PT_VAR_DEF ) {
            return NULL;
        }

        return new LinkVariableDeclarationPattern(tree->child->value, tree->value, tree->child->next->value);
    }

    virtual ~LinkVariableDeclarationPattern() {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        RVariable *source = context.GetVariable(this->sourceVariableName);
        RIterable * it = NULL;
        RClass *linkedClass = NULL;

        if ( source == NULL ||
            (it = context.GetLinkedObjects(source->getCurrent(), this->linkName)) == NULL ||
            (linkedClass = it->getClass()) == NULL ) {

#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG
            if ( it != NULL ) delete it;
            return false;
        }

        bool rv = false;
        RVariable * var = context.GetVariable(this->linkVariableName);
        if ( var != NULL ) {
            rv = var->IsSubclassOf(linkedClass);

            if(rv)
                var->setSource(it);
            // else
            //     throw runtime_error("Variable with incompatible type!");
        }
        else {
            var = context.CreateVariable(this->linkVariableName, linkedClass);
            var->setSource(it);
            rv = true;
        }

#if DEBUG
            std::wcout << (rv ? L"SUCCESS: " : L"FAILED: ");
            this->writeTo(std::wcout);
#endif // DEBUG

        if ( !rv )
            delete it;
        delete linkedClass;

        return rv && Pattern::Execute(context);
    }

    void writeTo(wostream& out) const {
        out << this->sourceVariableName
            << L"."
            << this->linkName
            << L"."
            << this->linkVariableName
            << endl;
    }

protected:
    wstring sourceVariableName;
    wstring linkVariableName;
    wstring linkName;

    LinkVariableDeclarationPattern(const wstring &sourceVariableName, const wstring &linkName, const wstring &linkVariableName)
        : sourceVariableName(sourceVariableName)
        , linkVariableName(linkVariableName)
        , linkName(linkName) {
    }
};

class CreateObjectPattern : public Pattern {
public:
    static CreateObjectPattern *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_VAR_DEF ||
            tree->child == NULL ||
            tree->child->type != PT_CLASS ) {
            return NULL;
        }

        return new CreateObjectPattern(tree->value, tree->child->value);
    }

    virtual ~CreateObjectPattern() {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        RClass *cls = context.FindClass(this->className);
        if ( cls == NULL ) {

#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG

            return false;
        }

        RVariable *var = context.GetVariable(this->variableName);
        if ( var == NULL ) {
            var = context.CreateVariable(this->variableName, cls);
        }
        else {
            if( !var->IsSubclassOf(cls) ) {
                delete cls;

#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG

                return false;
            }
        }

        RObject *obj = context.CreateObject();
        bool rv = false;
        if ( obj != NULL ) {
            obj->SpecifyType(cls);
            var->setCurrent(obj);
            rv = true;
        }

#if DEBUG
            std::wcout << (rv ? L"SUCCESS: " : L"FAILED: ");
            this->writeTo(std::wcout);
#endif // DEBUG

        delete cls;
        return rv && Pattern::Execute(context);
    }

    void writeTo(wostream &out) const {
        out << L"!"
            << this->variableName
            << L":"
            << this->className
            << endl;
    }
protected:
    wstring variableName;
    wstring className;

    CreateObjectPattern(wstring variableName, wstring className)
        : variableName(variableName)
        , className(className) {
    }
};

class CreateLinkPattern : public Pattern {
public:
    static CreateLinkPattern *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_LINK ||
            tree->child == NULL ||
            tree->child->type != PT_VAR_REF ||
            tree->child->next == NULL ||
            tree->child->next->type != PT_VAR_DEF ) {
            return NULL;
        }

        return new CreateLinkPattern(tree->child->value, tree->value, tree->child->next->value);
    }

    virtual ~CreateLinkPattern() {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        RVariable *source = context.GetVariable(this->sourceVariableName);
        RVariable *target = context.GetVariable(this->linkVariableName);
        if ( source == NULL || target == NULL ) {

#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG
            return false;
        }

        if(!source->getCurrent()->AddLink(this->linkName, target->getCurrent())){

#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG
            return false;
        }

#if DEBUG
            std::wcout << L"SUCCESS: ";
            this->writeTo(std::wcout);
#endif // DEBUG

        return Pattern::Execute(context);
    }

    void writeTo(wostream &out) const {
        out << this->sourceVariableName
            << L".!"
            << this->linkName
            << L"."
            << this->linkVariableName
            << endl;
    }
protected:
    wstring sourceVariableName;
    wstring linkVariableName;
    wstring linkName;

    CreateLinkPattern(const wstring &sourceVariableName, const wstring &linkName, const wstring &linkVariableName)
        : sourceVariableName(sourceVariableName)
        , linkVariableName(linkVariableName)
        , linkName(linkName) {
    }
};

class DeleteObjectPattern : public Pattern {
public:
    static DeleteObjectPattern *Create(const ParseTree *tree) {
        if (tree == NULL || tree->type != PT_VAR_REF ) {
            return NULL;
        }

        return new DeleteObjectPattern(tree->value);
    }

    virtual ~DeleteObjectPattern() {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        return false;
    }

    void writeTo(wostream &out) const {
        out << "~" << this->variableName << endl;
    }
protected:
    wstring variableName;

    DeleteObjectPattern(const wstring &variableName)
        : variableName(variableName) {
    }
};

class DeleteLinkPattern : public Pattern {
public:
    static DeleteLinkPattern *Create(const ParseTree *tree) {
        if ( tree == NULL ||
             tree->type != PT_LINK ||
             tree->child == NULL ||
             tree->child->type != PT_VAR_REF ||
             tree->child->next == NULL ||
             tree->child->next->type != PT_VAR_DEF ) {
             return NULL;
        }

        return new DeleteLinkPattern(tree->child->value, tree->value, tree->child->next->value);
    }

    virtual ~DeleteLinkPattern() {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        return false;
    }

    void writeTo(wostream &out) const {
        out << this->sourceVariableName
            << L".~"
            << this->linkName
            << L"."
            << this->linkVariableName
            << endl;
    }
protected:
    wstring sourceVariableName;
    wstring linkVariableName;
    wstring linkName;

    DeleteLinkPattern(const wstring &sourceVariableName, const wstring &linkName, const wstring &linkVariableName)
        : sourceVariableName(sourceVariableName)
        , linkVariableName(linkVariableName)
        , linkName(linkName) {
    }
};

class IterablePattern : public Pattern {
public:
    static IterablePattern *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_DECORATION ||
            tree->child == NULL) {
            return NULL;
        }

        TVariable *variable = TVariable::Create(tree->child);
        return new IterablePattern(variable, tree->value);
    }

    virtual ~IterablePattern() {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        RVariable *var = this->variable->Execute(context);
        if ( var == NULL ){
#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG

            return false;
        }

        bool rv = false;
        if ( this->iterationType == L"**" ) {
            do {
                rv = Pattern::Execute(context) || rv;
            } while ( var->MoveNext() );
        }
        else if ( this->iterationType == L"*" ) {
            do {
                rv = Pattern::Execute(context);
            } while ( !rv && var->MoveNext() );
        }

#if DEBUG
            std::wcout << (rv ? L"SUCCESS: " : L"FAILED: ");
            this->writeTo(std::wcout);
#endif // DEBUG

        return rv;
    }

    void writeTo(wostream &out) const {
        this->variable->writeTo(out, this->iterationType);
        out << endl;
    }
protected:
    const TVariable *variable;
    wstring iterationType;

    IterablePattern(const TVariable *variable, const wstring &iterationType)
        : variable(variable)
        , iterationType(iterationType) {
    }
};

class ContextPattern : public Pattern {
public:
    static ContextPattern *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_CONTEXT ||
            tree->child == NULL ||
            tree->child->type != PT_ALTERNATIVE ||
            tree->child->child == NULL ) {
            return NULL;
        }

        ContextPattern *pattern = new ContextPattern();
        for ( const ParseTree *t = tree->child; t != NULL; t = t->next ) {
            Pattern *p = Pattern::Create(t->child);
            if ( p == NULL ) {
                delete pattern;
                return NULL;
            }

            pattern->alternatives.push_back(p);
        }

        return pattern;
    }

    virtual ~ContextPattern() {
        for ( vector<const Pattern *>::const_iterator it = this->alternatives.begin();
              it != this->alternatives.end();
              it++ ) {
            const Pattern *p = *it;
            delete p;
        }
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        bool rv = false;
        for ( vector<const Pattern *>::const_iterator it = this->alternatives.begin();
              !rv && it != this->alternatives.end();
              it++ ) {

            const Pattern *p = *it;
            if ( p->Execute(context) ) {
                rv = true;
            }
        }
#if DEBUG
            std::wcout << (rv ? L"SUCCESS: " : L"FAILED: ");
            this->writeTo(std::wcout);
#endif // DEBUG

        return rv && Pattern::Execute(context);
    }

    void writeTo(wostream &out) const {
        out << L"[";
        bool first = true;
        for ( vector< const Pattern * >::const_iterator it = this->alternatives.begin();
              it != this->alternatives.end();
              it++ ) {
            if ( first )
                first = false;
            else
                out << "|";
            const Pattern *p = *it;
            p->writeTo(out);
        }

        out << L"]" << endl;
    }
protected:
    vector<const Pattern *> alternatives;

    ContextPattern() {
    }
};

class ExpressionPattern : public Pattern {
public:
    static ExpressionPattern *Create(const ParseTree *tree) {
        if ( tree == NULL ||
            tree->type != PT_EXPRESSION ) {
            return NULL;
        }

        return new ExpressionPattern(TExpression::Create(tree->child));
    }

    virtual ~ExpressionPattern() {

    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        bool rv = true;
        if ( this->expr != NULL ) {
            const IValue *val = this->expr->Execute(context);
            rv =
                val != NULL &&
                val->getValueType() != VT_UNKNOWN &&
                (val->getValueType() != VT_BOOL || val->toString() == L"true");
            delete val;
        }

#if DEBUG
        std::wcout << (rv ? L"SUCCESS: " : L"FAILED: ");
        this->writeTo(std::wcout);
#endif // DEBUG

        return rv && Pattern::Execute(context);
    }

    void writeTo(wostream &out) const {
        out << L"{ TODO }" << endl;
    }
protected:
    const TExpression *expr;

    ExpressionPattern(const TExpression *expr)
        : expr(expr) {
    }
};

Pattern *Pattern::Create(const ParseTree *tree) {
    Pattern *pattern = NULL, *p = NULL;
    const ParseTree *t = tree;
    while ( t != NULL ) {
        Pattern *c = NULL;
        switch(t->type) {
        case PT_DECORATION:
            {
                if ( t->child == NULL )
                    break;

                if ( t->child->type == PT_LINK ) {
                    if ( t->value == L"!" ) { // create
                        c = CreateLinkPattern::Create(t->child);
                    }
                    else if ( t->value == L"~" ) {
                        c = DeleteLinkPattern::Create(t->child);
                    }
                    else {
                        c = IterablePattern::Create(t);
                    }
                }
                else {
                    if ( t->value == L"!" ) { // create
                        c = CreateObjectPattern::Create(t->child);
                    }
                    else if ( t->value == L"~" ) {
                        c = DeleteObjectPattern::Create(t->child);
                    }
                    else {
                        c = IterablePattern::Create(t);
                    }
                }
            }
            break;
        case PT_EXPRESSION:
            c = ExpressionPattern::Create(t);
            break;
        case PT_CONTEXT:
            c = ContextPattern::Create(t);
            break;
        case PT_VAR_DEF:
            c = VariableDeclarationPattern::Create(t);
            break;
        case PT_LINK:
            c = LinkVariableDeclarationPattern::Create(t);
            break;
        }

        if ( pattern == NULL ) {
            pattern = p = c;
        }
        else {
            p->next = c;
            p = c;
        }

        t = t->next;
    }

    return pattern;
}
