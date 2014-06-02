#include <locale>
#include <string>
#include <sstream>
using namespace std;

#include "context.h"
#include <jr/japier.h>

#include "src/types.cpp"

#if DEBUG
#include <iostream>
#endif // DEBUG

Context::Context() {
    OpenRepository();
}

Context::~Context() {
    // release all resources consumed within variable member
    CloseRepository();
}

bool Context::LoadRepository(const char* repository) {
    return ::LoadRepository(repository) == 0;
}

bool Context::SaveRepository(const char* repository) {
    return ::SaveRepository(repository) == 0;
}

RVariable* Context::CreateVariable(const std::wstring& name, const RClass *c) {
    const Class *cls = reinterpret_cast<const Class *>(c);
    RVariableImpl *var = new RVariableImpl(cls);

    pair< map<wstring, RVariable* >::iterator, bool > res;
    res = this->variables.insert(pair< wstring, RVariable* >(name, var));
    if ( !res.second ) {
        // recreate variable
        this->variables.insert(pair< wstring, RVariable* >(name, res.first->second));
        return NULL;
    }

    return var;
}

RVariable *Context::GetVariable(const std::wstring& name) const {
    map< wstring, RVariable* >::const_iterator it;
    it = this->variables.find(name);
    if ( it == this->variables.end() )
        return NULL;

    return it->second;
}

RClass *Context::FindClass(const std::wstring& name) const {
    string str = WStringToUtf8(name);
    REFERENCE r = ::GetFirstClassByName(str.c_str());
    return new Class(r);
}

RClass *Context::CreateClass(const std::wstring& name) {
    static REFERENCE refNULL = ::NULL_Ref();

    string str = WStringToUtf8(name);
    REFERENCE r = ::CreateClass(str.c_str());
    if ( ::CompareReferences(refNULL, r) == COMP_EQ )
        return NULL;

    return new Class(r);
}

IType *Context::CreateType(const std::wstring& name, const IType *type) {
    // TODO:
    return NULL;
}

RObject *Context::CreateObject() {
    Object *o = new Object(::CreateObject());
    return o;
}

RIterable *Context::GetLinkedObjects(const RObject *obj, const std::wstring &association) {
    static REFERENCE refNULL = ::NULL_Ref();

    const Object *o = reinterpret_cast<const Object *>(obj);
    string a = WStringToUtf8(association);
    char buf[MAX_ID_NAME_LENGTH1];
    memset(buf, 0, MAX_ID_NAME_LENGTH1);

    REFERENCE ritClass = ::CreateIterator(o->reference, GET_CLASS_FROM_OBJECT);
    REFERENCE rClass = ::GetFirst(ritClass);
    ::GetName(rClass, buf);
    while ( ::CompareReferences(refNULL, rClass) != COMP_EQ ) {
        REFERENCE ritAssociation = ::CreateIterator(rClass, GET_ASSOCIATION_FROM_CLASS_AS_DOMAIN);
        REFERENCE rAssociation = ::GetFirst(ritAssociation);
        while ( ::CompareReferences(refNULL, rAssociation) != COMP_EQ ) {
            if ( ::GetName(rAssociation, buf) == 1 && a == buf ) {
                REFERENCE ritLinked = ::CreateIterator2(o->reference, rAssociation, GET_ASSOCLINK_FROM_OBJECT_AS_SUBJECT_AND_ASSOCIATION);
                REFERENCE rLinked = ::GetFirst(ritLinked);
                REFERENCE ritLinkedClass = ::CreateIterator(rAssociation, GET_CLASS_FROM_ASSOCIATION_AS_RANGE);
                REFERENCE rLinkedClass = ::GetFirst(ritLinkedClass);

                if ( ::CompareReferences(rLinked, refNULL) != COMP_EQ &&
                     ::CompareReferences(rLinkedClass, refNULL) != COMP_EQ ) {
                    ::GetName(rLinkedClass, buf);
                    return new Iterable(rLinkedClass, ::CreateIterator(rLinked, GET_OBJECT_FROM_ASSOCLINK_AS_OBJECT), refNULL);
                }
            }
            rAssociation = ::GetNext(ritAssociation);
        }

        ritAssociation = ::CreateIterator(rClass, GET_ASSOCIATION_FROM_CLASS_AS_RANGE);
        rAssociation = ::GetFirst(ritAssociation);
        while ( ::CompareReferences(refNULL, rAssociation) != COMP_EQ ) {
            if ( ::GetInverseName(rAssociation, buf) == 1 && a == buf ) {
                REFERENCE ritLinked = ::CreateIterator2(o->reference, rAssociation, GET_ASSOCLINK_FROM_OBJECT_AS_OBJECT_AND_ASSOCIATION);
                REFERENCE rLinked = ::GetFirst(ritLinked);
                REFERENCE ritLinkedClass = ::CreateIterator(rAssociation, GET_CLASS_FROM_ASSOCIATION_AS_DOMAIN);
                REFERENCE rLinkedClass = ::GetFirst(ritLinkedClass);

                if ( ::CompareReferences(rLinked, refNULL) != COMP_EQ &&
                     ::CompareReferences(rLinkedClass, refNULL) != COMP_EQ ) {
                    ::GetName(rLinkedClass, buf);
                    return new Iterable(rLinkedClass, ::CreateIterator(rLinked, GET_OBJECT_FROM_ASSOCLINK_AS_SUBJECT), refNULL);
                }
            }
            rAssociation = ::GetNext(ritAssociation);
        }
        rClass = ::GetNext(ritClass);
    }

    // TODO:
    return (RIterable*)NULL;
}

bool Context::CreateAssociation(const std::wstring& domain, const RClass *clsDomain, const std::wstring& range, const RClass *clsRange) {
    string d = WStringToUtf8(domain), r = WStringToUtf8(range);
    const Class *cd = reinterpret_cast<const Class *>(clsDomain),
                *cr = reinterpret_cast<const Class *>(clsRange);

    REFERENCE a = ::CreateAssociation(d.c_str(), r.c_str(), cd->reference, cr->reference, 0, 0, 0, 0);
    if (::CompareReferences(::NULL_Ref(), a) == COMP_EQ) {
#if DEBUG
        char buf[2048];
        memset(buf, 0, 2048);
        ::GetErrorInformation(buf);
        std::cout << buf;
#endif // DEBUG
        return false;
    }
    return true;
}

const BaseType& Context::getIntegerType() {
    static BaseType type(L"INTEGER", INTEGER);
    return type;
}

const BaseType& Context::getStringType() {
    static BaseType type(L"STRING", STRING);
    return type;
}

const BaseType& Context::getBooleanType() {
    static BaseType type(L"BOOLEAN", BOOLEAN);
    return type;
}

const BaseType& Context::getRealType() {
    static BaseType type(L"REAL", REAL);
    return type;
}
