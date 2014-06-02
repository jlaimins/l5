#include <src/interpreter.h>
#include <string>
#include <vector>
#include <stdexcept>

#if DEBUG
#include <iostream>
#endif // DEBUG
using namespace ::std;

class ClassDefinition : public Definition {
public:
    static ClassDefinition* Create(const ParseTree *pt) {
        // class Person { name:string, family:string }
        if ( pt->type != PT_CLASS_DEF )
            return NULL;

        ClassDefinition *expr = new ClassDefinition(pt->value);

        if ( pt->child == NULL)
            return expr;

        for ( pt = pt->child;
              pt != NULL;
              pt = pt->next ) {
            if ( pt->type != PT_ATTR || pt->child == NULL || pt->child->type != PT_CLASS )
                throw runtime_error("Invalid class definition attribute");

            expr->members.push_back(make_pair< wstring, wstring >(pt->value, pt->child->value));
        }

        return expr;
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG
        RClass *cls = context.CreateClass(this->name);
        if ( cls == NULL ) {
#if DEBUG
            std::wcout << L"FAILED: ";
            this->writeTo(std::wcout);
#endif // DEBUG
            return false;
        }

        bool rv = true;
        for ( vector< pair<wstring, wstring> >::const_iterator it = this->members.begin();
              rv && it != this->members.end();
              it++ ) {

            if ( !cls->AddAttribute(it->first, it->second) ) {
                rv = false;
            }
        }

        delete cls;
#if DEBUG
            std::wcout << (rv ? L"SUCCESS: " : L"FAILED: ");
            this->writeTo(std::wcout);
#endif // DEBUG

        // call base class
        return Definition::Execute(context);
    }

    void writeTo(wostream &out) const {
        out << L"class " << this->name << L"{";
        bool first = true;
        for ( vector< pair<wstring, wstring> >::const_iterator it = this->members.begin();
              it != this->members.end();
              it++ ) {
            if ( first )
                first = false;
            else
                out << L",";

            out << it->first << L":" << it->second;
        }

        out << L"}" << endl;
    }
protected:
    ClassDefinition(const wstring &name)
        : name(name) {
    }

    wstring name;
    vector< pair< wstring, wstring > > members;
};

class AssociationDefinition : public Definition {
public:
    static AssociationDefinition *Create(const ParseTree *tree) {
        // associate members:Person with family:Family
        if ( tree->type != PT_ASSOC_DEF )
            return NULL;

        if ( tree->child == NULL ||
             tree->child->type != PT_ASSOC ||
             tree->child->child == NULL ||
             tree->child->child->type != PT_CLASS ||
             tree->child->next == NULL ||
             tree->child->next->type != PT_ASSOC ||
             tree->child->next->child == NULL ||
             tree->child->next->child->type != PT_CLASS ) {
            throw runtime_error("Invalid association definition!");
        }

        return new AssociationDefinition(
            tree->child->next->value, // memberRange
            tree->child->child->value, // classDomain
            tree->child->value, // memberDomain
            tree->child->next->child->value); // classRange
    }

    AssociationDefinition(wstring memberRange, wstring classDomain, wstring memberDomain, wstring classRange)
        : memberRange(memberRange)
        , classDomain(classDomain)
        , memberDomain(memberDomain)
        , classRange(classRange) {
    }

    bool Execute(RContext &context) const {
#if DEBUG
            std::wcout << L"EXEC: ";
            this->writeTo(std::wcout);
#endif // DEBUG

        const RClass *classDomain = context.FindClass(this->classDomain);
        const RClass *classRange = context.FindClass(this->classRange);

        bool rv = context.CreateAssociation(this->memberRange, classDomain, this->memberDomain, classRange);

        delete classDomain;
        delete classRange;

#if DEBUG
            std::wcout << (rv ? L"SUCCESS: " : L"FAILED: ");
            this->writeTo(std::wcout);
#endif // DEBUG

        return rv && Definition::Execute(context);
    }

    void writeTo(wostream &out) const {
        out << L"associate "
            << this->memberDomain
            << L":"
            << this->classDomain
            << L" with "
            << this->memberRange
            << L":"
            << this->classRange
            << endl;
    }
protected:
    wstring memberRange;
    wstring classDomain;
    wstring memberDomain;
    wstring classRange;
};

class PatternDefinition : public Definition {
public:
    static PatternDefinition *Create(const ParseTree *tree) {
        // pattern
        if ( tree->type != PT_PATTERN || tree->child == NULL )
            return NULL;

        Pattern *p = Pattern::Create(tree->child);
        if ( p == NULL ) return NULL;
        return new PatternDefinition(p);
    }

    bool Execute(RContext &context) const {
        return this->pattern->Execute(context) && Definition::Execute(context);
    }

    void writeTo(wostream &out) const {
        this->pattern->writeTo(out);
    }

protected:
    PatternDefinition(const Pattern* p)
        : pattern(p) {
    }

    const Pattern * pattern;
};

Definition *Definition::Create(const ParseTree *tree) {
    if ( tree == NULL )
        return NULL;

    Definition *definition = NULL, *p;
    const ParseTree *t = tree;
    while ( t != NULL ) {
        Definition *c = NULL;
        switch(t->type) {
        case PT_CLASS_DEF:
            c = ClassDefinition::Create(t);
            break;
        case PT_ASSOC_DEF:
            c = AssociationDefinition::Create(t);
            break;
        case PT_PATTERN:
            c = PatternDefinition::Create(t);
            break;
        default:
            // throw runtime_error("Invalid expression!");
            break;
        }

        if ( definition == NULL ) {
            definition = p = c;
        }
        else {
            p->next = c;
            p = c;
        }

        t = t->next;
    }


    return definition;
}
