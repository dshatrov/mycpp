#include <mycpp/mycpp.h>

using namespace MyCpp;

class MyClass : public IntrusiveListElement<>
{
public:
};

int main (void)
{
    IntrusiveList<MyClass> list;

    MyClass a, b, c;

    list.append (&a);
    list.append (&b, &a);
    list.prepend (&c);

    list.remove (&a);
}

