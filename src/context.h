#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>
#include <string>
#include "src/irepository.h"

using namespace std;

class BaseType;
class Context : public RContext {
public:
    Context();
    virtual ~Context();

    bool LoadRepository(const char* repository);
    bool SaveRepository(const char* repository);

    RVariable* CreateVariable(const std::wstring& name, const RClass *c);
    RVariable* GetVariable(const std::wstring& name) const;

    RClass* CreateClass(const std::wstring& name);
    IType* CreateType(const std::wstring& name, const IType * base = NULL);

    bool CreateAssociation(const std::wstring& domain, const RClass *clsDomain, const std::wstring& range, const RClass *clsRange);

    RObject* CreateObject();
    RIterable *GetLinkedObjects(const RObject *obj, const std::wstring &association);

    RClass* FindClass(const std::wstring& name) const;

    static const BaseType& getIntegerType();
    static const BaseType& getStringType();
    static const BaseType& getBooleanType();
    static const BaseType& getRealType();
protected:
private:
    map<wstring, RVariable*> variables;
};

#endif // CONTEXT_H
