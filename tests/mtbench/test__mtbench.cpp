#include <mycpp/mycpp.h>

#include "tscmeasure.h"

using namespace MyCpp;

class A : public SimplyReferenced
{
};

class B
{
};

void f (int i)
{
}

/* Результаты на eee pc 901:
 *     A:  6443230869
 *     B:  1379067516
 *     A2: 711562113
 *     B2: 86055057
 */
int main (void)
{
    myCppInit ();

    Size const num_objs = 10000000;

    TscMeasurement tm;

    Ref<A> *a = new Ref<A> [num_objs];
    tsc_start_measurement (&tm);
    for (Size i = 0; i < num_objs; i++) {
	a [i] = grab (new A);
    }
    tsc_stop_measurement (&tm);

    errf->print ("A:  ").print (tsc_get_ticks (&tm)).pendl ();

    B **b = new B* [num_objs];
    tsc_start_measurement (&tm);
    for (Size i = 0; i < num_objs; i++) {
	b [i] = new B;
    }
    tsc_stop_measurement (&tm);

    errf->print ("B:  ").print (tsc_get_ticks (&tm)).pendl ();

    {
	AtomicInt a;
	tsc_start_measurement (&tm);
	for (Size i = 0; i < num_objs; i++) {
	    a.set (i);
	}
	tsc_stop_measurement (&tm);

	errf->print ("A2: ").print (tsc_get_ticks (&tm)).pendl ();
    }

    {
	int a;
	tsc_start_measurement (&tm);
	for (Size i = 0; i < num_objs; i++) {
	    a = i * 13;
	    errf->print ("");
	}
	tsc_stop_measurement (&tm);

	errf->print ("B2: ").print (tsc_get_ticks (&tm)).pendl ();
    }

    {
	int a = 0;
	tsc_start_measurement (&tm);
	for (Size i = 0; i < num_objs; i++) {
	    compareByteArrays (ConstMemoryDesc ((char*) i, 0), ConstMemoryDesc ((char*) i, 0));
	}
	tsc_stop_measurement (&tm);

	errf->print ("B3: ").print (tsc_get_ticks (&tm)).pendl ();
    }

    return 0;
}

