#ifndef JAPIER_DLL_H /* header for programs USING the DLL */
#define JAPIER_DLL_H

//#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tda_unicode.h"
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
  #define PATH_SEPARATOR_CHAR '\\'
  #define PATH_SEPARATOR_STR "\\"
  #ifdef BUILD_DLL
   #ifdef __cplusplus
           #define EXPORT extern "C" __declspec(dllexport) __stdcall
   #else
           #define EXPORT __declspec(dllexport) __stdcall
   #endif // __cplusplus
  #else
   #ifdef __cplusplus
           #define EXPORT extern "C" __declspec(dllimport) __stdcall
   #else
           #define EXPORT __declspec(dllimport) __stdcall
   #endif // __cplusplus
  #endif // BUILD_DLL
#else
  #define PATH_SEPARATOR_CHAR '/'
  #define PATH_SEPARATOR_STR "/"
  #ifdef BUILD_DLL
   #ifdef __cplusplus
   #define EXPORT extern "C" //_export
   #else
   #define EXPORT extern //_export
   #endif
  #else
   #ifdef __cplusplus
   #define EXPORT extern "C" _import
   #else
   #define EXPORT extern _import
   #endif
  #endif
#endif

//Datu tipu shēma -- ja vajadzēs, varēs definēt arī citas
#define TYPE_SCHEMA 1

#if TYPE_SCHEMA == 1
 //typedef char            INT1;
 typedef unsigned char  UINT1;
 //typedef short           INT2;
 typedef unsigned short UINT2;
 typedef int             INT4;
 typedef unsigned int   UINT4;
#endif

//Datutipu bāzes tipi -- izmaiņu gadījumā jāsaskaņo ar CreateDataType
#define INTEGER  1
#define STRING   2
#define BOOLEAN  3
#define REAL     4

//Lietu tipi
#define TYPE_UNDEFINED    0
#define TYPE_CLASS        1
#define TYPE_ASSOCIATION  2
#define TYPE_ATTRIBUTE    3
#define TYPE_OBJECT       4
#define TYPE_ASSOCLINK    5
#define TYPE_ATTRLINK     6
#define TYPE_DATAVALUE    7
#define TYPE_DATATYPE     8
#define TYPE_ENUMVALUE    9
#define TYPE_CHARS       10
#define TYPE_ITERATOR    11

// Maksimālais virkņu garums
#define MAX_ID_NAME_LENGTH 2047
#define MAX_ID_NAME_LENGTH1 MAX_ID_NAME_LENGTH+1

// Atgriešanās kodi
#define OK    0

//Kļūdu kodi
#define ERR_REPOSITORY_CLOSED         1
#define ERR_NO_PAGE_MEMORY            2
#define ERR_UNKNOWN_TYPE              3
#define ERR_IMPOSSIBLE_CREATE_FILE    4
#define ERR_ASSOCIATION_EXPECTED      5
#define ERR_ATTRIBUTE_EXPECTED        6
#define ERR_OBJECT_PART_EXPECTED      7
#define ERR_DATAVALUE_EXPECTED        8
#define ERR_EMPTY_NAME                9
#define ERR_CLASS_PART_EXPECTED      10
#define ERR_INCORRECT_BASETYPE       11
#define ERR_DUPLICATE_NAME           12
#define ERR_CHARS_EXPECTED           13
#define ERR_DATATYPE_EXPECTED        14
#define ERR_TYPE_SUBCLASSOFF_LOOP    15
#define ERR_NULL_REFERENCE           16
#define ERR_ITERATOR_EXPECTED        17
#define ERR_UNEXPECTED_TYPE          18
#define ERR_INCOMPATIBLE_TYPES       19
#define ERR_ENUMVALUE_EXPECTED       20
#define ERR_ENUMVALUE_LIST_EXPECTED  21
#define ERR_DUPLICATE_ENUMVALUE      22
#define ERR_ASSOCLINK_EXPECTED       23
#define ERR_ATTRLINK_EXPECTED        24
#define ERR_FILE_NOT_FOUND           25
#define ERR_VERSION_MISMATCH         26
#define ERR_MAKE_DIRECTORY_ERROR     27
#define ERR_ALREADY_OPENED           28

#define ERROR_TYPE_COUNT             28  // tāds pats kā pēdējās kļūdas kods

// Meklēšanas veidi
#define GET_CLASS_FROM_OBJECT                 1
#define GET_CLASS_FROM_ATTRIBUTE              2
#define GET_CLASS_FROM_ASSOCIATION_AS_RANGE   3
#define GET_CLASS_FROM_ASSOCIATION_AS_DOMAIN  4
#define GET_SUBCLASS_FROM_CLASS               5
#define GET_CLASS_FROM_SUBCLASS               6
#define GET_CLASS                             7
#define GET_SUBCLASS_FROM_CLASS_COMPLETE      8
#define GET_CLASS_FROM_SUBCLASS_COMPLETE      9

#define GET_ATTRIBUTE_FROM_CLASS             11
#define GET_ATTRIBUTE_FROM_CLASS_COMPLETE    12
#define GET_ATTRIBUTE_FROM_DATATYPE          21
#define GET_ATTRIBUTE_FROM_ATTRLINK          22
#define GET_ATTRIBUTE                        23

#define GET_ATTRLINK_FROM_ATTRIBUTE          31
#define GET_ATTRLINK_FROM_OBJECT             32
#define GET_ATTRLINK_FROM_DATAVALUE          33
#define GET_ATTRLINK                         34
#define GET_ASSOCIATION_FROM_CLASS_AS_RANGE  41
#define GET_ASSOCIATION_FROM_CLASS_AS_DOMAIN 42
#define GET_ASSOCIATION_FROM_ASSOCLINK       43
#define GET_ASSOCIATION                      44
#define GET_ASSOCIATION_FROM_CLASS_AS_RANGE_COMPLETE  45
#define GET_ASSOCIATION_FROM_CLASS_AS_DOMAIN_COMPLETE 46

#define GET_ASSOCLINK_FROM_ASSOCIATION       51
#define GET_ASSOCLINK_FROM_OBJECT_AS_OBJECT  52
#define GET_ASSOCLINK_FROM_OBJECT_AS_SUBJECT 53
#define GET_ASSOCLINK                        54
#define GET_ASSOCLINK_FROM_OBJECT_AS_OBJECT_AND_ASSOCIATION  55
#define GET_ASSOCLINK_FROM_OBJECT_AS_SUBJECT_AND_ASSOCIATION 56



#define GET_DATATYPE_FROM_ATTRIBUTE          61
#define GET_DATATYPE_FROM_DATAVALUE          62
#define GET_DATATYPE_FROM_ENUMVALUE          63
#define GET_DATATYPE                         64

#define GET_DATAVALUE_FROM_ATTRLINK          71
#define GET_DATAVALUE_FROM_DATATYPE          72
#define GET_DATAVALUE                        73
#define GET_DATAVALUE_FROM_OBJECT_AND_ATTRIBUTE 74

#define GET_OBJECT_FROM_ASSOCLINK_AS_OBJECT  81
#define GET_OBJECT_FROM_ASSOCLINK_AS_SUBJECT 82
#define GET_OBJECT_FROM_ATTRLINK             83
#define GET_OBJECT_FROM_CLASS                84
#define GET_OBJECT_FROM_CLASS_AND_SUBCLASSES 85
#define GET_OBJECT                           86
#define GET_OBJECT_FROM_DATAVALUE_AND_ATTRIBUTE   87
#define GET_O_OBJECT_FROM_OBJECT_AND_ASSOCIATION  88
#define GET_S_OBJECT_FROM_OBJECT_AND_ASSOCIATION  89

#define GET_ENUMVALUE_FROM_DATATYPE          91
#define GET_ENUMVALUE                        92

// Neaizņemt 100 - 109 !!!

//  Papildus norāde, ka jāmeklē pēc vārda (jālieto kopā ar iepriekšējiem,
// izveidojot izteksmi ar loģisko "UN")
//  Vērtība - divnieka pakāpe par kuru visas vienkāršās vērtības ir mazākas.
#define GET_BY_NAME                         256
#define GET_CLASS_BY_NAME                   GET_BY_NAME + TYPE_CLASS
#define GET_ATTRIBUTE_BY_NAME                GET_BY_NAME + TYPE_ATTRIBUTE
#define GET_ASSOCIATION_BY_NAME             GET_BY_NAME + TYPE_ASSOCIATION
#define GET_DATATYPE_BY_NAME                GET_BY_NAME + TYPE_DATATYPE
#define GET_DATAVALUE_BY_NAME               GET_BY_NAME + TYPE_DATAVALUE
#define GET_ENUMVALUE_BY_NAME               GET_BY_NAME + TYPE_ENUMVALUE

// Loģiskās konstantes
#define FALSE 0
#define TRUE  1

// Struktūru salīdzināšanas konstantes
#define COMP_LT 1
#define COMP_EQ 2
#define COMP_GT 4

// Asociācijas virziens
 #define UNKNOWN        0
 #define ASSOC_DIRECT   1
 #define ASSOC_INVERSE  2

#ifdef BUILD_DLL
#include "jap_ref.h"
#else
// jebkuras "lietas" references apraksts
typedef
union reference
{
 UINT1 x[6];
} REFERENCE;
#endif

// Asociāciju īpašību konstantes
#define DELETE_DOMAIN_WITH_PARTNER 1
#define DELETE_RANGE_WITH_PARTNER  2
#define ORDERED_DOMAIN             3
#define ORDERED_RANGE              4

// Darbības ar repozitoriju
EXPORT      INT4  OpenRepository(void);
EXPORT      INT4  SaveRepository(const char* fileDirectoryName);
EXPORT      INT4  LoadRepository(const char* fileDirectoryName);
EXPORT      void  CloseRepository(void);

// Jebkura veida lietas (objekts, klase, utt.) izmešana
EXPORT      void  Remove(REFERENCE ref);
EXPORT      void  MarkToDiscard(REFERENCE ref);  //Reāli izmet tikai pie DeleteAll izsaukšanas
EXPORT      void  DeleteAll(void);

// Darbības ar objektiem
EXPORT REFERENCE  CreateObject(void);

// Darbības ar klasēm
EXPORT  REFERENCE  CreateClass(const char* name);
EXPORT       INT4  SpecifySuperclass(REFERENCE subclass, REFERENCE superclass);
EXPORT       INT4  CancelSuperclass(REFERENCE subclass, REFERENCE superclass);
EXPORT       INT4  SpecifyType (REFERENCE obj, REFERENCE cl);
EXPORT       INT4  CancelType (REFERENCE obj, REFERENCE cl);
EXPORT       INT4  TypeSubclassOfPathExists(REFERENCE from, REFERENCE to);

//Darbības ar asociācijām
EXPORT REFERENCE  CreateAssociation(const char* name, const char* inverseName, REFERENCE domain, REFERENCE range,
                                    UINT1 _delete_domain_with_partner, UINT1 _delete_range_with_partner,
                                    UINT1 _ordered_domain, UINT1 _ordered_range);
EXPORT      UINT1 GetAssociationProperty(REFERENCE r, UINT1 property_to_check);

//Darbības ar atribūtiem
EXPORT REFERENCE  CreateAttribute(const char* name, REFERENCE domain, REFERENCE range);

//Darbības ar asociāciju saitēm
EXPORT REFERENCE  CreateAssocLink(REFERENCE assoc, REFERENCE subj, REFERENCE obj );
EXPORT REFERENCE  GetAssocLinkObject(REFERENCE assoc_link);
EXPORT REFERENCE  GetAssocLinkSubject(REFERENCE assoc_link);

//Darbības ar atribūtu saitēm
EXPORT REFERENCE  CreateAttrLink(REFERENCE attr, REFERENCE subj, REFERENCE obj) ;
EXPORT REFERENCE  GetAttrLinkObject(REFERENCE attr_link);
EXPORT REFERENCE  GetAttrLinkSubject(REFERENCE attr_link);

//Darbības ar datu tipiem
EXPORT REFERENCE  CreateDataType(const char* name, UINT1 base);

//Darbības ar datu vērtībām
EXPORT REFERENCE  CreateDataValue(REFERENCE type, const char* str_value);
EXPORT REFERENCE  CreateDataValueFromEnumValue(REFERENCE enum_value);

//Darbības ar pārskaitāmā tipa vērtībām
EXPORT REFERENCE  CreateEnumValue(REFERENCE type, const char* str_value, INT4 ind);

//Darbības ar iteratoriem
EXPORT REFERENCE  CreateIterator0(INT4 iteratorType);
EXPORT REFERENCE  CreateIterator(REFERENCE starting_point, INT4 iteratorType);
EXPORT REFERENCE  CreateIterator2(REFERENCE starting_point, REFERENCE param, INT4 iteratorType);
EXPORT REFERENCE  CreateIteratorN(const char* name, INT4 iteratorType);
EXPORT REFERENCE  GetFirst(REFERENCE iterator);
EXPORT REFERENCE  GetNext(REFERENCE iterator);

//Vienkāršās (pirmās vērtības) meklēšanas funkcijas
EXPORT REFERENCE  GetFirstClassByName(const char* name);
EXPORT REFERENCE  GetFirstAttributeFromClassByName(REFERENCE cl, const char* name);
EXPORT REFERENCE  GetFirstAssociationFromClassByName(REFERENCE cl, const char* name, UINT1* direction);
EXPORT REFERENCE  GetFirstAssocLinkFromObjectsAndAssoc(REFERENCE sourceObj, REFERENCE targetObj, REFERENCE assoc);

//Darbības ar referencēm
EXPORT      void  PrintReference(const char* pref, REFERENCE r, INT4 level);
EXPORT REFERENCE  NULL_Ref(void); // Atdod nulles referenci (dummy_ref)
EXPORT      INT4  GetReferenceType(REFERENCE r);

//Vārdu un "String" satura atdošanas funkcijas
EXPORT      INT4  GetName(REFERENCE r, char* buff);
EXPORT      INT4  GetReferenceInfo(REFERENCE r, char* buff);
EXPORT      INT4  GetInverseName(REFERENCE r, char* buff);
EXPORT      INT4  GetValue(REFERENCE r, char* buff);

//Indeksu atdošanas funkcijas
EXPORT      INT4  GetIndex(REFERENCE r);
EXPORT     UINT1  GetBaseType(REFERENCE r);

//Cita veida funkcijas
EXPORT      INT4  GetErrorCode(void);
EXPORT      void  GetErrorInformation(char* buff);
EXPORT      void  SetMaxPrintLevel(INT4 value);
EXPORT      void  SetMaxErrorCount(INT4 count);
//EXPORT      void  SetCriticalErrorLevel(INT4 level);
EXPORT      INT4  ResetErrorLevel(void);
EXPORT      void  SetErrorLevel(UINT1 error_type, INT4 level);
  // Noteikt, vai reference joprojām ir "dzīva" - var maldināt, ja notikušas jaunas izvietošanas
EXPORT      INT4  ReferenceIsLive(REFERENCE r);
  // Tehniski salīdzina references (rezultāts: < COMP_LT, = COMP_EQ, > COMP_GT)
EXPORT     UINT1  CompareReferences(REFERENCE r1, REFERENCE r2);

EXPORT REFERENCE  Test(void);

#endif
