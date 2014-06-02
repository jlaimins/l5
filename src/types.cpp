#include <jr/japier.h>
#include "src/irepository.h"

#include <stdexcept>
using std::runtime_error;

// FIXME
std::string WStringToUtf8(const std::wstring& str) {
    char wc[str.length() + 1];
    wcstombs(wc, str.c_str(), str.length());
    wc[str.length()] = 0;
    return std::string(wc);
}

// FIXME
std::wstring Utf8ToWString(const std::string& str) {
    wchar_t wc[str.length() + 1];
    mbstowcs(wc, str.c_str(), str.length());
    wc[str.length()] = 0;
    return std::wstring(wc);
}

class BaseType : public IType {
public:
    BaseType(const wstring& name, INT4 baseType) {
        string s = WStringToUtf8(name);
        this->reference = ::CreateDataType(s.c_str(), baseType);
    }

    ~BaseType() {
    }

    REFERENCE reference;
};

class Class : public RClass {
public:
    Class(REFERENCE r)
        : reference(r) {
    }

    virtual ~Class() {
    }

    virtual bool AddAttribute(const std::wstring &name, const std::wstring &type) {
        REFERENCE rtype;
        if ( type == L"string" )
            rtype = Context::getStringType().reference;
        else if ( type == L"integer" )
            rtype = Context::getIntegerType().reference;
        else if ( type == L"real" )
            rtype = Context::getRealType().reference;
        else if ( type == L"boolean" )
            rtype = Context::getBooleanType().reference;

        // name, class, dataType
        string s = WStringToUtf8(name);
        return CompareReferences(
            ::CreateAttribute(s.c_str(), this->reference, rtype),
            ::NULL_Ref()) != 0;
    }

    virtual void Print() const {
        ::PrintReference("", this->reference, 0);
    }

    virtual RVariable* CreateVariable() const {
        // TODO:
        return (RVariable *)NULL;
    }

    REFERENCE reference;
};

class ClassIterator : public RClass::Iterator {
public:
    ClassIterator(REFERENCE r)
        : _reference(r) {
    }

    virtual ~ClassIterator() {
    }

    virtual bool MoveNext() {
        // TODO:
        this->_reference = ::GetNext(this->_reference);
        INT4 cmp = CompareReferences(this->_reference, ::NULL_Ref());
        return cmp != 2;
    }

    virtual bool MoveFirst() {
        this->_reference = ::GetFirst(this->_reference);
        return CompareReferences(this->_reference, ::NULL_Ref()) != 0;
    }

    virtual RClass* Current() const {
        // TODO:
        return new Class(this->_reference);
    }
private:
    REFERENCE _reference;
};

class Object : public RObject {
public:
    Object(REFERENCE r)
        : reference(r) {
    }

    virtual ~Object() { }

    bool SpecifyType(const RClass *cls) {
        const Class *c = reinterpret_cast<const Class *>(cls);
        if ( c == NULL )
            throw runtime_error("Argument type error: Argument of Class type expected!");

        ::SpecifyType(this->reference, c->reference);
        return false;
    }

    bool AddAttribute(const std::wstring& attribute, const RValue *value) {
        string attrName = WStringToUtf8(attribute);
        string attrValue = WStringToUtf8(value->getValue());

        char buf[MAX_ID_NAME_LENGTH1];
        memset(buf, 0, MAX_ID_NAME_LENGTH1);

        REFERENCE rNULL = ::NULL_Ref();
        REFERENCE ritClass = ::CreateIterator(this->reference, GET_CLASS_FROM_OBJECT);
        REFERENCE rClass = ::GetFirst(ritClass);

        while (::CompareReferences(rNULL, rClass) != COMP_EQ) {
            REFERENCE ritAttr = ::CreateIterator(rClass, GET_ATTRIBUTE_FROM_CLASS);
            REFERENCE rAttr = ::GetFirst(ritAttr);
            while (::CompareReferences(rNULL, rAttr) != COMP_EQ) {
                if ( ::GetName(rAttr, buf) == TRUE && attrName == string(buf) ) {
                    // TODO: get value type
                    REFERENCE ritType = ::CreateIterator(rAttr, GET_DATATYPE_FROM_ATTRIBUTE);
                    REFERENCE rType = ::GetFirst(ritType);
                    if ( ::CompareReferences(rNULL, rType) != COMP_EQ) {
                        // TODO: create value
                        REFERENCE rValue = ::CreateDataValue(rType, attrValue.c_str());
                        if ( ::CompareReferences(rNULL, rValue) != COMP_EQ ) {
                            REFERENCE rAttrLink = ::CreateAttrLink(rAttr, this->reference, rValue);
                            return ::CompareReferences(rNULL, rAttrLink) != COMP_EQ;
                        }
                    }
                }

                rAttr = ::GetNext(ritAttr);
            }

            rClass = ::GetNext(ritClass);
        }

        return false;
    }

    const RValue *GetAttribute(const std::wstring& name) {
        static REFERENCE refNULL = ::NULL_Ref();
        std::string n = WStringToUtf8(name);
        char buf[MAX_ID_NAME_LENGTH1];
        memset(buf, 0, MAX_ID_NAME_LENGTH1);

        // 1. Get class reference
        // 2. Get attribute reference
        // 3. Get value
        REFERENCE ritClass = ::CreateIterator(this->reference, GET_CLASS_FROM_OBJECT);
        REFERENCE rClass = ::GetFirst(ritClass);
        while ( ::CompareReferences(refNULL, rClass) != COMP_EQ ) {
            REFERENCE ritAttribute = ::CreateIterator(rClass, GET_ATTRIBUTE_FROM_CLASS);
            REFERENCE rAttribute = ::GetFirst(ritAttribute);
            while ( ::CompareReferences(refNULL, rAttribute) != COMP_EQ) {
                if ( ::GetName(rAttribute, buf) == 1 && n == buf ) {
                    REFERENCE ritValue = ::CreateIterator2(this->reference, rAttribute, GET_DATAVALUE_FROM_OBJECT_AND_ATTRIBUTE);
                    REFERENCE ritType = ::CreateIterator(rAttribute, GET_DATATYPE_FROM_ATTRIBUTE);
                    REFERENCE rType = ::GetFirst(ritType);
                    REFERENCE rValue = ::GetFirst(ritValue);
                    wstring value, type;

                    if ( ::GetValue(rValue, buf) == 1 )
                        value = Utf8ToWString(string(buf));

                    if ( ::GetName(rType, buf) == 1 )
                        type = Utf8ToWString(string(buf));

                    if ( type == L"STRING" ) {
                        return new RValue(VT_STRING, value);
                    }
                    else if ( type == L"INT" ) {
                        return new RValue(VT_INTEGER, value);
                    }
                    else if ( type == L"BOOL" ) {
                        return new RValue(VT_BOOL, value);
                    }
                    else if ( type == L"REAL" ) {
                        return new RValue(VT_REAL, value);
                    }
                    else {
                        return new RValue(VT_UNKNOWN, value);
                    }
                }

                rAttribute = ::GetNext(ritAttribute);
            }

            rClass = ::GetNext(ritClass);
        }

        return NULL;
    }

    bool SetAttribute(const std::wstring& name, const RValue *value) {
        return this->AddAttribute(name, value);
    }

    bool AddLink(const std::wstring& name, const RObject *obj) {
        static REFERENCE refNULL = ::NULL_Ref();

        string n = WStringToUtf8(name);
        const Object *o = reinterpret_cast<const Object *>(obj);
        char buf[MAX_ID_NAME_LENGTH1];
        memset(buf, 0, MAX_ID_NAME_LENGTH1);

        REFERENCE ritClass = ::CreateIterator(this->reference, GET_CLASS_FROM_OBJECT);
        for (REFERENCE rClass = ::GetFirst(ritClass);
             ::CompareReferences(refNULL, rClass) != COMP_EQ;
             rClass = ::GetNext(ritClass))
        {
            REFERENCE ritAssoc = ::CreateIterator(rClass, GET_ASSOCIATION_FROM_CLASS_AS_DOMAIN);
            REFERENCE rAssoc;
            for (rAssoc = ::GetFirst(ritAssoc);
                ::CompareReferences(refNULL, rAssoc) != COMP_EQ;
                rAssoc = ::GetNext(ritAssoc))
            {
                if ( ::GetName(rAssoc, buf) == 1 && n == buf ) {
                    REFERENCE rLink = ::CreateAssocLink(rAssoc, this->reference, o->reference);
                    if ( ::CompareReferences(refNULL, rLink )!= COMP_EQ )
                        return true;
                }
            }

            ritAssoc = ::CreateIterator(rClass, GET_ASSOCIATION_FROM_CLASS_AS_RANGE);
            for (rAssoc = ::GetFirst(ritAssoc);
                ::CompareReferences(refNULL, rAssoc) != COMP_EQ;
                rAssoc = ::GetNext(ritAssoc))
            {
                if ( ::GetInverseName(rAssoc, buf) == 1 && n == buf ) {
                    REFERENCE rLink = ::CreateAssocLink(rAssoc, o->reference, this->reference);
                    if ( ::CompareReferences(refNULL, rLink )!= COMP_EQ )
                        return true;
                }
            }
        }

        return false;
    }

    REFERENCE reference;
};

class Iterable : public RIterable {
public:
    Iterable(REFERENCE cls, REFERENCE it, REFERENCE cur)
        : it(it)
        , cls(cls)
        , current(cur) {
    }

    virtual ~Iterable() { }

    bool MoveFirst() {
        static REFERENCE rNULL = ::NULL_Ref();
        this->current = ::GetFirst(this->it);
        return ::CompareReferences(rNULL, this->current) != COMP_EQ;
    }

    bool MoveNext() {
        static REFERENCE rNULL = ::NULL_Ref();
        this->current = ::GetNext(this->it);
        return ::CompareReferences(rNULL, this->current) != COMP_EQ;
    }

    bool IsSubclassOf(const RClass *cls) const {
        const Class *c = reinterpret_cast<const Class *>(cls);
        if ( ::CompareReferences(c->reference, this->cls) == COMP_EQ )
            return true;

        return ::TypeSubclassOfPathExists(this->cls, c->reference) == OK;
    }

    void setCurrent(const RObject *o) {
        const Object *obj = reinterpret_cast<const Object *>(o);
        this->current = obj->reference;
    }

    RObject * getCurrent() const {
        static REFERENCE rNULL = ::NULL_Ref();
        if ( ::CompareReferences(rNULL, this->current) == COMP_EQ )
            return NULL;

        return new Object(this->current);
    }

    RClass * getClass() const {
        return new Class(this->cls);
    }

    REFERENCE it;
    REFERENCE cls;
    REFERENCE current;
};

class RVariableImpl : public RVariable {
public:
    RVariableImpl(const Class *c)
        : it(new Iterable(c->reference, ::CreateIterator(c->reference, GET_OBJECT_FROM_CLASS), ::NULL_Ref())) {
    }

    virtual ~RVariableImpl() {
        if ( this->it != NULL )
            delete this->it;

        this->it = NULL;
    }

    bool MoveFirst() {
        if ( this->it == NULL )
            return false;
        return this->it->MoveFirst();
    }

    bool MoveNext() {
        if (this->it == NULL ) return false;
        return this->it->MoveNext();
    }

    bool IsSubclassOf(const RClass *cls) const {
        const Class *c = reinterpret_cast<const Class *>(cls);
        if ( ::CompareReferences(c->reference, this->it->cls) == COMP_EQ )
            return true;

        return ::TypeSubclassOfPathExists(this->it->cls, c->reference) == OK;
    }

    void setCurrent(const RObject *o) {
        if ( this->it != NULL )
            this->it->setCurrent(o);
    }

    RObject * getCurrent() const {
        if ( this->it == NULL )
            return NULL;

        return this->it->getCurrent();
    }

    RClass * getClass() const {
        return this->it->getClass();
    }

    RIterable * getSource() const {
        // TODO
        return this->it;
    }

    void setSource(RIterable *iterable) {
        // TODO:
        if ( this->it != NULL )
            delete this->it;

        Iterable *it = reinterpret_cast<Iterable *>(iterable);
        this->it = it;
    }

    Iterable *it;
};


