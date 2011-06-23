#include <cstdlib>
#include <mycpp/mycpp.h>
#include <mycpp/util.h>
#include <mycpp/io.h>

using namespace MyCpp;

class Test_MyLink;

class Test_MyTask
{
protected:
    Test_MyLink const &link;

public:
    Size getTotalLength () const;

    Test_MyTask (Test_MyLink const &link);
};

class Test_MyTaskEntry
{
protected:
    Size len;

public:
    Size getLength () const
    {
	return len;
    }

    Test_MyTaskEntry (Size len)
    {
	this->len = len;
    }
};

class Test_MyLink
{
    friend class Test_MyTask;

protected:
    Test_MyTaskEntry const *task;

    Test_MyLink *first;
    Test_MyLink *next;
    Test_MyLink *previous;

public:
    Test_MyLink add (Test_MyTaskEntry const &task)
    {
	this->task = &task;

	if (previous != NULL)
	    previous->next = this;

	return Test_MyLink (this);
    }

    Test_MyLink (Test_MyTaskEntry const &task)
    {
	this->task = &task;

	first = this;
	previous = NULL;
	next = NULL;
    }

    Test_MyLink (Test_MyLink *previous)
    {
	this->previous = previous;

	if (previous != NULL)
	    first = previous->first;
	else
	    first = this;

	task = NULL;
	next = NULL;
    }
};

Size
Test_MyTask::getTotalLength () const
{
    errf->print ("Test_MyTask.getTotalLength").pendl ();

    Size total_len = 0;

    Test_MyLink *cur = link.first;
    while (cur != NULL) {
	abortIf (cur->task == NULL);
	Size cur_len = cur->task->getLength ();
	errf->print ("Test_MyTask.getTotalLength: cur_len: ").print (cur_len).pendl ();
	total_len += cur_len;

	cur = cur->next;
    }

    return total_len;
}

Test_MyTask::Test_MyTask (Test_MyLink const &link)
    : link (link)
{
}

// a ().b ().c ().d ()

// { ... } =. { ... } =. { ... } =.
//         ^
//         \_точно знаю (_предыдущий_)
// ...но вызываю _его_ метод => могу установить next,
// _когда_ будет вызываться _метод_ на следующем объекте.
// т е "previous->next=this" в add().
// для последнего - в преобразовании в Task (хак... :) - ну и пусть.
// Это план!

// Сейчас: a.add (b).add (c)
// План: add (a).add (b).add (c)

    // gen() -> Link
    // Link.add (task).add ()
    //                (Link)

    //
    //  |
    //  V
    // l1<-l2<-l3<-l4<-l5
    //  |   |   |   |   |
    //  V   V   V   V   V
    // pt1 pt2 pt3 pt4 pt5
    //

// /-------------------\.
// v                   |
// 1 <- 2 <- 3 <- 4 <- 5
//   ->   ->   ->   X

// non-const at construction time (!)
// a (b (c (d (e) ) ) )

static void
do_test (Test_MyTask const &task)
{
    errf->print ("do_test: total length: ").print (task.getTotalLength ()).pendl ();
}

int main (void)
try {
    myCppInit ();

    do_test (Test_MyLink (NULL).add (Test_MyTaskEntry (1)).add (Test_MyTaskEntry (2)).add (Test_MyTaskEntry (3)));

    Ref<String> str = String::forPrintTask ((Pr ("a_")) (Pr ("b_")) (Pr ("c_")));
    errf->print (str).pendl ();

    if (compareStrings (str->getData (), "a_b_c_")) {
	errf->print ("SUCCESS").pendl ();
	return 0;
    }

//    Ref<String> str2 = String::forPrintTask (Pt () << 1.0);
    Ref<String> str2 = String::forPrintTask (Pt (1) << 2);

    errf->print ("FAILURE").pendl ();
    return EXIT_FAILURE;
} catch (Exception &exc) {
    printException (errf, exc);
    errf->print ("FAILURE").pendl ();
    return EXIT_FAILURE;
}

