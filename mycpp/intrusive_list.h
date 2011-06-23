#ifndef __MYCPP__INTRUSIVE_LIST_H__
#define __MYCPP__INTRUSIVE_LIST_H__

// FIXME Thy include list.h here?
#include <mycpp/list.h>

namespace MyCpp {

template <int a = 0>
class IntrusiveListElement
{
    template <class T, class Element> friend class IntrusiveList;

private:
    IntrusiveListElement *next;
    IntrusiveListElement *previous;
};

template <class T, class Element = IntrusiveListElement<> >
class IntrusiveList
{
private:
    Element *first;
    Element *last;

    static T* objForElement (Element * const el /* non-null */)
    {
	return static_cast <T*> (el);
    }

    static Element* elementForObj (T * const obj /* non-null */)
    {
	return static_cast <Element*> (obj);
    }

public:
    T* getFirst () const
    {
	return objForElement (first);
    }

    T* getLast () const
    {
	return objForElement (last);
    }

    static T* getNext (T * const obj /* non-null */)
    {
	return objForElement (elementForObj (obj)->next);
    }

    static T* getPrevious (T * const obj /* non-null */)
    {
	return objForElement (elementForObj (obj)->previous);
    }

    bool isEmpty () const
    {
	return first == NULL;
    }

    void append (T * const obj /* non-null */)
    {
	append (obj, getLast ());
    }

    void append (T * const obj /* non-null */,
		 T * const to_obj)
    {
	Element * const el = elementForObj (obj);

	if (to_obj == NULL) {
	    el->next = NULL;
	    el->previous = NULL;
	    first = el;
	    last  = el;
	    return;
	}

	Element * const to_el = elementForObj (to_obj);

	el->next = to_el->next;
	el->previous = to_el;

	if (to_el->next != NULL)
	    to_el->next->previous = el;

	to_el->next = el;

	if (to_el == last)
	    last = el;
    }

    void prepend (T * const obj /* non-null */)
    {
	prepend (obj, getFirst ());
    }

    void prepend (T * const obj /* non-null */,
		  T * const to_obj)
    {
	Element * const el = elementForObj (obj);

	if (to_obj == NULL) {
	    el->next = NULL;
	    el->previous = NULL;
	    first = el;
	    last  = el;
	    return;
	}

	Element * const to_el = elementForObj (to_obj);

	el->previous = to_el->previous;
	el->next = to_el;

	if (to_el->previous != NULL)
	    to_el->previous->next = el;

	to_el->previous = el;

	if (to_el == first)
	    first = el;
    }

    void remove (T * const obj /* non-null */)
    {
	Element * const el = elementForObj (obj);

	if (el == first)
	    first = el->next;
	else
	    el->previous->next = el->next;

	if (el == last)
	    last = el->previous;
	else
	    el->next->previous = el->previous;
    }

    IntrusiveList ()
	: first (NULL),
	  last (NULL)
    {
    }
};

}

#endif /* __MYCPP__INTRUSIVE_LIST_H__ */

