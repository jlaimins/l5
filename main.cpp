#include <iostream>
#include <sstream>
#include <fstream>

#include "src/parser.h"
#include "src/context.h"
#include "src/interpreter.h"

#include <jr/japier.h>

using namespace std;

int main(int argc, char** argv)
{
    Context context;
    if ( argc >= 3 ) {
        context.LoadRepository(argv[2]);
    }

    const ParseTree *tree;
    if ( argc >= 2 ) {
        std::wifstream ss(argv[1]);
        if(ss.bad()) {
            std::wcerr << L"Could not open file: " << argv[1] << endl;
            return -1;
        }

        tree = Parser(ss).parse();
    }
    else {
        cout << "Usage: " << argv[0] << "definition.l5 [repository]" << endl
             << "    definition.l5: Transformation definition in L5 language" << endl
             << "    repository: (optional) repository name to load and save results to" << endl;
        return -1;
        // std::wifstream ss("E:\\l5\\examples\\person-family.l5");
        // std::wifstream ss("E:\\l5\\examples\\graph.l5");
        // tree = Parser(ss).parse();
    }

    Interpreter interpreter;
    cout << "=====================================================" << endl
         << (interpreter.Execute(context, tree) ? "Success" : "Failed") << endl;

    delete tree;

    REFERENCE it = ::CreateIterator0(GET_CLASS);
    REFERENCE p = ::GetFirst(it);
    char buf[MAX_ID_NAME_LENGTH1];
    while (::CompareReferences(p, ::NULL_Ref()) != 2)
    {
        cout << "=====================================================" << endl
             << "Class - GetName = " << ::GetName(p, buf) << "; " << buf << endl;
        ::PrintReference("", p, 1);

        REFERENCE it2 = ::CreateIterator(p, GET_ASSOCIATION_FROM_CLASS_AS_DOMAIN);
        REFERENCE p2 = ::GetFirst(it2);
        while (::CompareReferences(p2, ::NULL_Ref()) != COMP_EQ) {
            cout << "=====================================================" << endl
                 << "Reference - Domain" << endl;
            ::PrintReference("", p2, 2);
            p2 = ::GetNext(it2);
        }

        it2 = ::CreateIterator(p, GET_ASSOCIATION_FROM_CLASS_AS_RANGE);
        p2 = ::GetFirst(it2);
        while (::CompareReferences(p2, ::NULL_Ref()) != COMP_EQ) {
            cout << "=====================================================" << endl
                 << "Reference - Range" << endl;
            ::PrintReference("", p2, 2);
            p2 = ::GetNext(it2);
        }

        it2 = ::CreateIterator(p, GET_ATTRIBUTE_FROM_CLASS);
        p2 = ::GetFirst(it2);
        while (::CompareReferences(p2, ::NULL_Ref()) != COMP_EQ) {
            cout << "=====================================================" << endl
                 << "Attribute - GetName = " << ::GetName(p2, buf) << ": " << buf << endl;
            ::PrintReference("", p2, 2);

            REFERENCE it3 = ::CreateIterator(p2, GET_DATATYPE_FROM_ATTRIBUTE);
            REFERENCE p3 = ::GetFirst(it3);
            while ( ::CompareReferences(p3, ::NULL_Ref()) != COMP_EQ)
            {
                cout << "=====================================================" << endl
                     << "DataType - GetName = " << ::GetName(p3, buf) << "; " << buf << endl;
                ::PrintReference("", p3, 3);
                p3 = ::GetNext(it3);
            }

            p2 = ::GetNext(it2);
        }


        it2 = ::CreateIterator(p, GET_OBJECT_FROM_CLASS);
        p2 = ::GetFirst(it2);
        while(::CompareReferences(p2, ::NULL_Ref()) != COMP_EQ)
        {
            cout << "=====================================================" << endl
                 << "Object" << endl;
            ::PrintReference("", p2, 2);

            REFERENCE it3 = ::CreateIterator(p2, GET_ASSOCLINK_FROM_OBJECT_AS_OBJECT);
            REFERENCE p3 = ::GetFirst(it3);
            while (::CompareReferences(p3, ::NULL_Ref()) != COMP_EQ) {
                cout << "=====================================================" << endl
                     << "Link as object" << endl;
                ::PrintReference("", p3, 3);
                p3 = ::GetNext(it3);
            }

            it3 = ::CreateIterator(p2, GET_ASSOCLINK_FROM_OBJECT_AS_SUBJECT);
            p3 = ::GetFirst(it3);
            while (::CompareReferences(p3, ::NULL_Ref()) != COMP_EQ) {
                cout << "=====================================================" << endl
                     << "Link as subject" << endl;
                ::PrintReference("", p3, 3);
                p3 = ::GetNext(it3);
            }

            p2 = ::GetNext(it2);
        }

        p = ::GetNext(it);
    }

    if ( argc >= 3 )
        context.SaveRepository(argv[2]);

	return 0;
}
