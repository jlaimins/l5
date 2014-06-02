#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <string>
#include <map>

template<class TValue>
class IIterator {
public:
    virtual ~IIterator() { }
    virtual bool MoveNext() = 0;
    virtual bool MoveFirst() = 0;
    virtual TValue* Current() const = 0;
};


enum ValueType {
    VT_UNKNOWN,
    VT_BOOL,
    VT_INTEGER,
    VT_REAL,
    VT_STRING,
    VT_ITERABLE, // Objects
};

class RValue {
public:
    RValue(ValueType type, const std::wstring &value)
        : type(type)
        , value(value) {
    }

    ValueType getValueType() const {return this->type; }
    const std::wstring &getValue() const { return this->value; }
private:
    ValueType type;
    std::wstring value;
};

class RObject;
class RClass;
class RIterable;
class RVariable {
public:
    virtual ~RVariable() { }

    virtual bool MoveFirst() = 0;
    virtual bool MoveNext() = 0;

    virtual bool IsSubclassOf(const RClass *c) const = 0;
    virtual RClass * getClass() const = 0;

    virtual void setCurrent(const RObject* o) = 0;
    virtual RObject * getCurrent() const = 0;

    virtual void setSource(RIterable *source) = 0;
    virtual RIterable *getSource() const = 0;
};

class RClass;
class RObject {
public:
    virtual ~RObject() { }

    virtual bool SpecifyType(const RClass *cls) = 0;
    virtual bool AddAttribute(const std::wstring& attribute, const RValue *value) = 0;
    virtual bool AddLink(const std::wstring &name, const RObject *other) = 0;

    virtual const RValue *GetAttribute(const std::wstring& attribute) = 0;
    virtual bool SetAttribute(const std::wstring& attribute, const RValue *value) = 0;
    typedef IIterator<RObject> Iterator;
};

class IType {
public:
    virtual ~IType() { }

    typedef IIterator<IType> Iterator;
};

class RClass {
public:
    virtual ~RClass() { }
    virtual bool AddAttribute(const std::wstring &name, const std::wstring &type) = 0;

    // virtual IVariable* CreateVariable() const = 0;
    // virtual void Print() const = 0;

    typedef IIterator<RClass> Iterator;
};

class RIterable {
public:
    virtual ~RIterable() { }

    virtual RClass * getClass() const = 0;
    virtual RObject *getCurrent() const = 0;

    virtual bool MoveFirst() = 0;
    virtual bool MoveNext() = 0;
};
/*
class IRepository
{
public:
    virtual ~IRepository() { }
    virtual IClass::Iterator* FindClass(std::wstring name) const = 0;
};
*/
class RContext
{
public:
    virtual ~RContext() { }

    virtual RVariable* CreateVariable(const std::wstring& name, const RClass *c) = 0;
    virtual RVariable* GetVariable(const std::wstring& name) const = 0;

    virtual RClass* CreateClass(const std::wstring& name) = 0;
    virtual RObject* CreateObject() = 0;
    virtual RIterable *GetLinkedObjects(const RObject *object, const std::wstring &linkName) = 0;

    virtual bool CreateAssociation(const std::wstring &domain, const RClass *clsDomain, const std::wstring &range, const RClass *clsRange) = 0;

    virtual RClass* FindClass(const std::wstring& name) const = 0;
};
#endif // IREPOSITORY_H
