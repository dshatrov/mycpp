#include <mycpp/mycpp.h>
#include <mycpp/io.h>

using namespace MyCpp;

class A : public virtual SimplyReferenced
{
};

class B : public A
{
};

static void
f (A *)
{
}

static void
do_test ()
{
    Ref<A> a = grab (new B);

    f (grab (new B));
}

int main (void)
try {
    myCppInit ();

    do_test ();

    errf->print ("SUCCESS").pendl ();
    return EXIT_SUCCESS;
} catch (Exception &exc) {
    printException (errf, exc);
    errf->print ("FAILURE").pendl ();
    return EXIT_FAILURE;
}

