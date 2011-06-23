#include <mycpp/embedded_list.h>
#include <mycpp/util.h>

/* NOTE If you find errors in this file, then check out
 * m2/reterm/reterm-module/list.c as well. The code there is
 * essentially the same. */

namespace MyCpp {

void
embeddedListAppend (EmbeddedListHead    *head,
		    EmbeddedListElement *new_el,
		    EmbeddedListElement *old_el)
{
    if (head   == NULL ||
	new_el == NULL)
    {
	abortIfReached ();
    }

    if (old_el == NULL) {
	new_el->next = new_el;
	new_el->previous = new_el;
	head->first = new_el;
    } else {
	new_el->next = old_el->next;
	new_el->previous = old_el;
	old_el->next->previous = new_el;
	old_el->next = new_el;
    }
}

void
embeddedListPrepend (EmbeddedListHead    *head,
		     EmbeddedListElement *new_el,
		     EmbeddedListElement *old_el)
{
    if (head   == NULL ||
	new_el == NULL)
    {
	abortIfReached ();
    }

    if (old_el == NULL) {
	new_el->next = new_el;
	new_el->previous = new_el;
	head->first = new_el;
    } else {
	new_el->next = old_el;
	new_el->previous = old_el->previous;
	old_el->previous->next = new_el;
	old_el->previous = new_el;

	if (head->first == old_el)
	    head->first = new_el;
    }
}

void
embeddedListRemove (EmbeddedListHead    *head,
		    EmbeddedListElement *el)
{
    if (head == NULL ||
	el   == NULL)
    {
	abortIfReached ();
    }

    if (el->next == el) {
	head->first = NULL;
    } else {
	if (el->next != NULL) {
	  /* Implies el->previous != NULL */
	    el->next->previous = el->previous;
	    el->previous->next = el->next;
	}

	if (head->first == el)
	    head->first = el->next;
    }

    el->next = NULL;
    el->previous = NULL;
}

}

