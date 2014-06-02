#include <src/interpreter.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace ::std;

class IBoolValue : public IValue {
public:
    IBoolValue(bool v)
        : IValue(VT_BOOL)
        , value(v) {
    }

    IBoolValue(const wstring& v)
        : IValue(VT_BOOL)
        , value(v == L"true") {
    }

    IBoolValue(const IBoolValue &v)
        : IValue(VT_BOOL)
        , value(v.value) {
    }

    virtual ~IBoolValue() { }

    wstring toString() const {
        return this->value ? L"true" : L"false";
    }

    const IValue * HandleBinary(const wstring& op, const IValue *other) const {
        return NULL;
    }
protected:
    bool value;
};

class IIntegerValue : public IValue {
public:
    IIntegerValue(int v)
        : IValue(VT_INTEGER)
        , value(v){
    }

    IIntegerValue(const std::wstring &v)
        : IValue(VT_INTEGER)
        , value(_wtoi(v.c_str())) {
    }

    IIntegerValue(const IIntegerValue &v)
        : IValue(VT_INTEGER)
        , value(v.value) {
    }

    virtual ~IIntegerValue() { }

    wstring toString() const {
        wstringstream ss;
        ss << this->value;
        return ss.str();
    }

    const IValue * HandleBinary(const wstring& op, const IValue *other) const {
        return NULL;
    }
protected:
    int value;
};

class IRealValue : public IValue {
public:
    IRealValue(double v)
        : IValue(VT_REAL)
        , value(v) {
    }

    IRealValue(const std::wstring &val)
        : IValue(VT_REAL)
        , value(_wtof(val.c_str())) {
    }

    IRealValue(const IRealValue &v)
        : IValue(VT_REAL)
        , value(v.value) {
    }

    virtual ~IRealValue() { }

    wstring toString() const {
        wstringstream ss;
        ss << this->value;
        return ss.str();
    }

    const IValue * HandleBinary(const wstring& op, const IValue *other) const {
        return NULL;
    }
protected:
    double value;
};

class IStringValue : public IValue {
public:
    IStringValue(std::wstring v)
        : IValue(VT_STRING)
        , value(v) {
    }

    IStringValue(const IStringValue &v)
        : IValue(VT_STRING)
        , value(v.value) {
    }

    virtual ~IStringValue() { }

    wstring toString() const {
        return this->value;
    }

    const IValue * HandleBinary(const wstring& op, const IValue *other) const {
        if ( other->getValueType() != VT_STRING ) {
            return NULL;
        }

        const IStringValue *o = reinterpret_cast<const IStringValue *>(other);
        if ( o == NULL )
            return NULL;

        if ( op == L"==" ) {
            return new IBoolValue(this->value == o->value);
        }
        else if ( op == L"!=" ) {
            return new IBoolValue(this->value != o->value);
        }
        else if ( op == L"+" ) {
            return new IStringValue(this->value + o->value);
        }

        return NULL;
    }
protected:
    std::wstring value;
};

class UnknownValue : public IValue {
public:
    UnknownValue()
        : IValue(VT_UNKNOWN) {
    }

    UnknownValue(const UnknownValue &v)
        : IValue(VT_UNKNOWN) {
    }

    virtual ~UnknownValue() { }

    wstring toString() const {
        return L"";
    }

    const IValue * HandleBinary(const wstring& op, const IValue *other) const {
        return NULL;
    }
};

IValue *IValue::Create(ValueType vt, wstring val) {
    switch(vt) {
    case VT_BOOL:
        return new IBoolValue(val);
    case VT_INTEGER:
        return new IIntegerValue(val);
    case VT_REAL:
        return new IRealValue(val);
    case VT_STRING:
        return new IStringValue(val);
    case VT_UNKNOWN:
    case VT_ITERABLE:
        break;
    }

    return new UnknownValue();
}
