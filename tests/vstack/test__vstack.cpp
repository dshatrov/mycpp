#include <new>

#include <mycpp/mycpp.h>
#include <mycpp/io.h>

using namespace MyCpp;

class MyClass
{
public:
    int data_member;

    MyClass ()
	: data_member (13)
    {
	errf->print ("MyClass()").pendl ();
    }

    ~MyClass ()
    {
	errf->print ("~MyClass()").pendl ();
    }
};

class MyBase
{
public:
    int base_data_member;

    MyBase ()
	: base_data_member (1)
    {
	errf->print ("MyBase()").pendl ();
    }

    virtual ~MyBase ()
    {
	errf->print ("~MyBase()").pendl ();
    }
};

class MyDerived : public MyBase
{
public:
    int derived_data_member;

    MyDerived ()
	: derived_data_member (2)
    {
	errf->print ("MyDerived()").pendl ();
    }

    ~MyDerived ()
    {
	errf->print ("~MyDerived()").pendl ();
    }
};

int main (void)
{
    myCppInit ();

    VStack vstack (4096 /* block_size */);

    Byte * const stack_ptr = vstack.push_malign (sizeof (MyClass) * 2, sizeof (MyClass));
    MyClass * const myclass = new (stack_ptr) MyClass;
    MyClass * const myclass2 = new (stack_ptr + sizeof (MyClass)) MyClass;
    MyDerived * const myderived = new (vstack.push_malign (sizeof (MyDerived), sizeof (MyDerived))) MyDerived;

    errf->print ("--- constructed ---").pendl ();

    myclass->~MyClass ();
    myderived->~MyDerived ();
}

