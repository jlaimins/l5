#include <sstream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "src/icontext.h"
#include "src/irepository.h"

#include "src/parser.h"
#include "src/interpreter.h"

using namespace ::std;
using namespace ::testing;

class MockClass : public RClass {
public:
    MOCK_CONST_METHOD0(CreateVariable, RVariable*());
    MOCK_CONST_METHOD0(Print, void());

    MOCK_METHOD2(AddAttribute, bool(const wstring&, const wstring&));
};

/*
class MockClassIterator : public IIterator<IClass> {
public:
    MOCK_METHOD0(MoveNext, bool());
    MOCK_METHOD0(MoveFirst, bool());
    MOCK_CONST_METHOD0(Current, IClass*());
};
*/

class MockContext : public RContext {
public:
    MOCK_METHOD2(CreateVariable, RVariable*(const std::wstring&, const RClass *));
    MOCK_METHOD1(CreateClass, RClass*(const std::wstring&));
    MOCK_METHOD0(CreateObject, RObject*());
    MOCK_METHOD4(CreateAssociation, bool(const wstring &, const RClass *, const wstring &, const RClass *));

    MOCK_METHOD2(GetLinkedObjects, RIterable*(const RObject *, const wstring &));

    MOCK_CONST_METHOD1(FindClass, RClass*(const std::wstring&));
    MOCK_CONST_METHOD1(GetVariable, RVariable*(const std::wstring&));
};

class MockVariable : public RVariable {
public:
    MOCK_METHOD0(MoveFirst, bool());
    MOCK_METHOD0(MoveNext, bool());

    MOCK_CONST_METHOD1(IsSubclassOf, bool(const RClass *c));
    MOCK_CONST_METHOD0(getClass, RClass *());

    MOCK_METHOD1(setCurrent, void(const RObject *));
    MOCK_CONST_METHOD0(getCurrent, RObject *());

    MOCK_METHOD1(setSource, void(RIterable *));
    MOCK_CONST_METHOD0(getSource, RIterable *());
};

class MockIterable : public RIterable {
public:
    MOCK_CONST_METHOD0(getClass, RClass *());
    MOCK_CONST_METHOD0(getCurrent, RObject *());

    MOCK_METHOD0(MoveFirst, bool());
    MOCK_METHOD0(MoveNext, bool());
};

class MockObject : public RObject {
public:
    MOCK_METHOD1(SpecifyType, bool(const RClass*));
    MOCK_METHOD2(AddAttribute, bool(const wstring&, const wstring&));
    MOCK_METHOD1(GetAttribute, RValue *(const wstring &));
    MOCK_METHOD2(SetAttribute, bool(const wstring&, const RValue *));
    MOCK_METHOD2(AddLink, bool(const wstring&, const RObject*));
};

TEST(InterpreterTest, Simple) {
    wstringstream ss(L"x:X");
    const ParseTree *pt = Parser(ss).parse();
    MockContext context;
    MockVariable *var = new MockVariable();
    MockClass *cls = new MockClass();

    EXPECT_CALL(context, GetVariable(wstring(L"x"))).WillOnce(Return((RVariable *)NULL));
    EXPECT_CALL(context, FindClass(wstring(L"X"))).WillOnce(Return(cls));
    EXPECT_CALL(context, CreateVariable(wstring(L"x"), _)).WillOnce(Return(var));
    EXPECT_CALL(*var, MoveFirst()).WillOnce(Return(true));

    Interpreter interpreter;
    bool actual = interpreter.Execute(context, pt);
    ASSERT_TRUE(actual);
    delete pt;

    // cls should be collected within method
    delete var;
}

TEST(InterpreterTest, MultiplePatterns) {
    // p and f are already defined
    // p:Person f:Family
    MockContext context;
    MockClass *clsPerson = new MockClass();
    MockClass *clsFamily = new MockClass();
    MockVariable *varPerson = new MockVariable();
    MockVariable *varFamily = new MockVariable();

    EXPECT_CALL(context, FindClass(wstring(L"Person"))).WillOnce(Return(clsPerson));
    EXPECT_CALL(context, FindClass(wstring(L"Family"))).WillOnce(Return(clsFamily));

    EXPECT_CALL(context, GetVariable(wstring(L"p"))).WillOnce(Return(varPerson));
    EXPECT_CALL(context, GetVariable(wstring(L"f"))).WillOnce(Return(varFamily));

    EXPECT_CALL(*varPerson, IsSubclassOf(clsPerson)).WillOnce(Return(true));
    EXPECT_CALL(*varFamily, IsSubclassOf(clsFamily)).WillOnce(Return(true));

    EXPECT_CALL(*varPerson, MoveNext()).WillOnce(Return(true));
    EXPECT_CALL(*varFamily, MoveNext()).WillOnce(Return(true));

    wstringstream ss(L"p:Person f:Family");
    const ParseTree *pt = Parser(ss).parse();

    Interpreter interpreter;
    ASSERT_TRUE(interpreter.Execute(context, pt));

    // Cleanup will happen within real context's destructor
    delete varPerson;
    delete varFamily;
}

TEST(InterpreterTest, ClassDefinition) {
    wstringstream ss(L"class Person { name:string,family:string }");
    const ParseTree *pt = Parser(ss).parse();

    // creation of new class (JAPIER API)
    // 1. Create class (name)
    // 2. Get attribute type (name, smth)
    // 3. Create attribute (name, class, type)
    MockContext *context = new MockContext();
    MockClass *cls = new MockClass();
    EXPECT_CALL(*context, CreateClass(wstring(L"Person")))
        .WillOnce(Return(cls));
    EXPECT_CALL(*cls, AddAttribute(wstring(L"name"), wstring(L"string")))
        .WillOnce(Return(true));
    EXPECT_CALL(*cls, AddAttribute(wstring(L"family"), wstring(L"string")))
        .WillOnce(Return(true));

    Interpreter interpreter;
    ASSERT_TRUE(interpreter.Execute(*context, pt));

    delete pt;
    delete context;
}

TEST(InterpreterTest, ObjectCreation) {
    wstringstream ss(L"!p:Person");
    const ParseTree *pt = Parser(ss).parse();

    MockContext context;
    MockClass *cls = new MockClass();
    MockObject *obj = new MockObject();
    MockVariable *var = new MockVariable();

    EXPECT_CALL(context, FindClass(wstring(L"Person"))).WillOnce(Return(cls));
    EXPECT_CALL(context, CreateObject()).WillOnce(Return(obj));
    EXPECT_CALL(context, GetVariable(wstring(L"p"))).WillOnce(Return((RVariable *)NULL));
    EXPECT_CALL(context, CreateVariable(wstring(L"p"), _)).WillOnce(Return(var));

    EXPECT_CALL(*var, setCurrent(obj)).WillOnce(Return());
    EXPECT_CALL(*obj, SpecifyType(cls)).WillOnce(Return(true));

    Interpreter interpreter;
    ASSERT_TRUE(interpreter.Execute(context, pt));
    delete var;
    delete obj;
}

TEST(InterpreterTest, VariableReuseObjectCreation) {
    wstringstream ss(L"!p:Person !p:Person");
    const ParseTree *pt = Parser(ss).parse();

    MockContext context;
    MockClass *clsFirst = new MockClass(), *clsSecond = new MockClass();
    MockObject *objFirst = new MockObject(), *objSecond = new MockObject();
    MockVariable *var = new MockVariable();

    EXPECT_CALL(context, FindClass(wstring(L"Person"))).WillOnce(Return(clsFirst)).WillOnce(Return(clsSecond));
    EXPECT_CALL(context, GetVariable(wstring(L"p"))).WillOnce(Return((RVariable *)NULL)).WillOnce(Return(var));
    EXPECT_CALL(context, CreateVariable(wstring(L"p"), clsFirst)).WillOnce(Return(var)); // for first obj
    EXPECT_CALL(*var, IsSubclassOf(clsSecond)).WillOnce(Return(true)); // for second obj

    EXPECT_CALL(context, CreateObject()).WillOnce(Return(objFirst)).WillOnce(Return(objSecond));
    EXPECT_CALL(*objFirst, SpecifyType(clsFirst)).WillOnce(Return(true));
    EXPECT_CALL(*var, setCurrent(objFirst)).WillOnce(Return());

    EXPECT_CALL(*objSecond, SpecifyType(clsSecond)).WillOnce(Return(true));
    EXPECT_CALL(*var, setCurrent(objSecond)).WillOnce(Return());

    Interpreter interpreter;
    ASSERT_TRUE(interpreter.Execute(context, pt));

    delete objFirst;
    delete objSecond;
    delete var;
    delete pt;
}

TEST(InterpreterTest, DefineAssociation) {
    wstringstream ss(L"associate members:Person with family:Family");
    const ParseTree *pt = Parser(ss).parse();

    MockContext context;
    MockClass *clsPerson = new MockClass();
    MockClass *clsFamily = new MockClass();

    EXPECT_CALL(context, FindClass(wstring(L"Person"))).WillOnce(Return(clsPerson));
    EXPECT_CALL(context, FindClass(wstring(L"Family"))).WillOnce(Return(clsFamily));

    EXPECT_CALL(context, CreateAssociation(wstring(L"family"), clsPerson, wstring(L"members"), clsFamily)).WillOnce(Return(true));

    Interpreter interpreter;
    ASSERT_TRUE(interpreter.Execute(context, pt));
    delete pt;
}

TEST(InterpreterTest, CreateAssociation) {
    wstringstream ss(L"p.!family.f ");
    const ParseTree *pt = Parser(ss).parse();

    MockContext context;
    MockVariable *varPerson = new MockVariable();
    MockVariable *varFamily = new MockVariable();

    MockObject *objPerson = new MockObject();
    MockObject *objFamily = new MockObject();

    EXPECT_CALL(context, GetVariable(wstring(L"p"))).WillOnce(Return(varPerson));
    EXPECT_CALL(context, GetVariable(wstring(L"f"))).WillOnce(Return(varFamily));

    EXPECT_CALL(*varPerson, getCurrent()).WillOnce(Return(objPerson));
    EXPECT_CALL(*varFamily, getCurrent()).WillOnce(Return(objFamily));
    EXPECT_CALL(*objPerson, AddLink(wstring(L"family"), objFamily)).WillOnce(Return(true));

    Interpreter interpreter;
    ASSERT_TRUE(interpreter.Execute(context, pt));

    delete varPerson;
    delete varFamily;
    delete objPerson;
    delete objFamily;

    delete pt;
}

TEST(InterpreterTest, SimpleTransformation) {
    wstringstream ss(L"**x:X x.**aa.y y.**bb.z x.!cc.z");
    const ParseTree *pt = Parser(ss).parse();

    MockContext context;
    MockClass *classX = new MockClass();
    MockClass *classY1 = new MockClass();
    MockClass *classY2 = new MockClass();
    MockClass *classZ1 = new MockClass();
    MockClass *classZ2 = new MockClass();
    MockClass *classZ3 = new MockClass();
    MockClass *classZ4 = new MockClass();

    MockVariable *variableX = new MockVariable();
    MockVariable *variableY = new MockVariable();
    MockVariable *variableZ = new MockVariable();

    MockObject *objectX1 = new MockObject();
    MockObject *objectY1 = new MockObject();
    MockObject *objectZ1 = new MockObject();
    MockObject *objectX2 = new MockObject();
    MockObject *objectY2 = new MockObject();
    MockObject *objectZ2 = new MockObject();

    MockIterable *iterableY = new MockIterable();
    MockIterable *iterableZ = new MockIterable();

    EXPECT_CALL(context, FindClass(wstring(L"X"))).WillOnce(Return(classX));
    EXPECT_CALL(context, GetVariable(wstring(L"x")))
        .WillOnce(Return((RVariable *)NULL))
        .WillRepeatedly(Return(variableX));

    EXPECT_CALL(context, GetVariable(wstring(L"y")))
        .WillOnce(Return((RVariable *)NULL))
        .WillRepeatedly(Return(variableY));

    EXPECT_CALL(context, GetVariable(wstring(L"z")))
        .WillOnce(Return((RVariable *)NULL))
        .WillRepeatedly(Return(variableZ));

    EXPECT_CALL(context, CreateVariable(wstring(L"x"), classX)).WillOnce(Return(variableX));
    EXPECT_CALL(context, CreateVariable(wstring(L"y"), classY1)).WillOnce(Return(variableY));
    EXPECT_CALL(context, CreateVariable(wstring(L"z"), classZ1)).WillOnce(Return(variableZ));

    EXPECT_CALL(context, GetLinkedObjects(objectX1, wstring(L"aa"))).WillOnce(Return(iterableY));
    EXPECT_CALL(context, GetLinkedObjects(objectX2, wstring(L"aa"))).WillOnce(Return(iterableY));
    EXPECT_CALL(context, GetLinkedObjects(objectY1, wstring(L"bb"))).Times(2).WillRepeatedly(Return(iterableZ)); // x1 x2
    EXPECT_CALL(context, GetLinkedObjects(objectY2, wstring(L"bb"))).Times(2).WillRepeatedly(Return(iterableZ)); // x1 x2

    EXPECT_CALL(*iterableY, getClass())
        .WillOnce(Return(classY1)) // x1
        .WillOnce(Return(classY2)); // x2

    EXPECT_CALL(*iterableZ, getClass())
        .WillOnce(Return(classZ1)) // x1 y1
        .WillOnce(Return(classZ2)) // x1 y2
        .WillOnce(Return(classZ3)) // x2 y1
        .WillOnce(Return(classZ4));// x2 y2

    EXPECT_CALL(*variableX, MoveFirst()).WillOnce(Return(true));
    EXPECT_CALL(*variableX, MoveNext()).WillOnce(Return(true)).WillOnce(Return(false));
    { // 4times for x1 and 4 for x2
        ::testing::InSequence s;
        EXPECT_CALL(*variableX, getCurrent()).Times(5).WillRepeatedly(Return(objectX1)); // once for each linking and once for getlinkedobjects
        EXPECT_CALL(*variableX, getCurrent()).Times(5).WillRepeatedly(Return(objectX2));
    }

    EXPECT_CALL(*variableY, IsSubclassOf(classY2)).WillOnce(Return(true));

    EXPECT_CALL(*variableY, setSource(iterableY)).WillOnce(Return()).WillOnce(Return());

    EXPECT_CALL(*variableY, MoveFirst()).WillOnce(Return(true)).WillOnce(Return(true));

    EXPECT_CALL(*variableY, MoveNext())
        .WillOnce(Return(true)).WillOnce(Return(false)) // x1
        .WillOnce(Return(true)).WillOnce(Return(false));// x2

    EXPECT_CALL(*variableY, getCurrent())
        .WillOnce(Return(objectY1)).WillOnce(Return(objectY2)) // x1 - once for get linked objects
        .WillOnce(Return(objectY1)).WillOnce(Return(objectY2));// x2 - once for get linked objects

    EXPECT_CALL(*variableZ, IsSubclassOf(classZ2)).WillOnce(Return(true));
    EXPECT_CALL(*variableZ, IsSubclassOf(classZ3)).WillOnce(Return(true));
    EXPECT_CALL(*variableZ, IsSubclassOf(classZ4)).WillOnce(Return(true));

    EXPECT_CALL(*variableZ, setSource(iterableZ)).Times(4).WillRepeatedly(Return());

    EXPECT_CALL(*variableZ, MoveFirst()).Times(4).WillRepeatedly(Return(true));

    EXPECT_CALL(*variableZ, MoveNext())
        .WillOnce(Return(true)).WillOnce(Return(false)) // x1 y1
        .WillOnce(Return(true)).WillOnce(Return(false)) // x1 y2
        .WillOnce(Return(true)).WillOnce(Return(false)) // x2 y1
        .WillOnce(Return(true)).WillOnce(Return(false));// x2 y2

    EXPECT_CALL(*variableZ, getCurrent())
        .WillOnce(Return(objectZ1)).WillOnce(Return(objectZ2)) // x1 y1
        .WillOnce(Return(objectZ1)).WillOnce(Return(objectZ2)) // x1 y2
        .WillOnce(Return(objectZ1)).WillOnce(Return(objectZ2)) // x2 y1
        .WillOnce(Return(objectZ1)).WillOnce(Return(objectZ2));// x2 y2

    EXPECT_CALL(*objectX1, AddLink(wstring(L"cc"), objectZ1)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*objectX1, AddLink(wstring(L"cc"), objectZ2)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*objectX2, AddLink(wstring(L"cc"), objectZ1)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*objectX2, AddLink(wstring(L"cc"), objectZ2)).Times(2).WillRepeatedly(Return(true));

    ASSERT_TRUE(Interpreter().Execute(context, pt));

    delete variableX;
    delete variableY;
    delete variableZ;

    delete objectX1;
    delete objectY1;
    delete objectZ1;
    delete objectX2;
    delete objectY2;
    delete objectZ2;

    delete iterableY;
    delete iterableZ;
}

TEST(InterpreterTest, ExpressionBlock) {
    wstringstream ss(L"{ p.attr = \"first\", p.attr = \"second\" }");
    const ParseTree *pt = Parser(ss).parse();

    MockContext context;
    MockVariable *variableP = new MockVariable();
    MockObject *objectP = new MockObject();

    EXPECT_CALL(context, GetVariable(wstring(L"p"))).WillOnce(Return(variableP)).WillOnce(Return(variableP));
    EXPECT_CALL(*variableP, getCurrent()).WillOnce(Return(objectP)).WillOnce(Return(objectP));

    // FIXME: should check also values of second argument
    EXPECT_CALL(*objectP, SetAttribute(wstring(L"attr"), _)).WillOnce(Return(true)).WillOnce(Return(true));

    bool actual = Interpreter().Execute(context, pt);
    delete variableP;
    delete objectP;
    delete pt;

    ASSERT_TRUE(actual);
}

TEST(InterpreterTest, AlternatePattern) {
    wstringstream ss(L"[*p:Person { p.name == \"Janis\" } | !p:Person { p.name = \"Janis\" } ]");
    const ParseTree *pt = Parser(ss).parse();

    MockContext context;
    MockVariable var;
    MockObject objPerson;
    MockClass *clsPerson = new MockClass(), *clsPerson2 = new MockClass();

    EXPECT_CALL(context, GetVariable(wstring(L"p"))).Times(4).WillOnce(Return((RVariable*)NULL)).WillRepeatedly(Return(&var));
    EXPECT_CALL(context, FindClass(wstring(L"Person"))).WillOnce(Return(clsPerson)).WillOnce(Return(clsPerson2));
    EXPECT_CALL(context, CreateVariable(wstring(L"p"), clsPerson)).WillOnce(Return(&var));
    EXPECT_CALL(context, CreateObject()).WillOnce(Return(&objPerson));

    EXPECT_CALL(var, MoveFirst()).WillOnce(Return(false));
    EXPECT_CALL(var, MoveNext()).WillOnce(Return(false));
    EXPECT_CALL(var, getCurrent()).WillOnce(Return((RObject*)NULL)).WillOnce(Return(&objPerson));
    EXPECT_CALL(var, setCurrent((RObject*)&objPerson)).WillOnce(Return());
    EXPECT_CALL(var, IsSubclassOf(clsPerson2)).WillOnce(Return(true));

    EXPECT_CALL(objPerson, SetAttribute(wstring(L"name"), _)).WillOnce(Return(true));
    EXPECT_CALL(objPerson, SpecifyType(clsPerson2)).WillOnce(Return(true));

    bool actual = Interpreter().Execute(context, pt);
    ASSERT_TRUE(actual);
}
