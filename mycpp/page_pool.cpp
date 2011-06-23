#include <mycpp/page_pool.h>
#include <mycpp/util.h>

namespace MyCpp {

PagePool::Page::
Page (unsigned long size)
{
    data = new char [size];
    this->size = size;

    busy   = false;
    locked = false;
}

PagePool::Page::
~Page ()
{
    delete[] data;
}

void
PagePool::allocatePages ()
{
    while (npages < minPages) {
	Ref<Page> page = grab (new Page (pageSize));
	lruPages.prepend (page, lruPages.first);
	page->listLink = lruPages.first;
	npages ++;
    }
}

Ref<PagePool::Page>
PagePool::allocateExtraPage (bool locked)
{
    Ref<Page> page;

    if (npages < maxPages) {
	page = grab (new Page (pageSize));
	if (locked) {
	    lockedPages.append (page, lockedPages.last);
	    page->listLink = lockedPages.last;
	    page->locked = true;
	} else {
	    lruPages.append (page, lruPages.last);
	    page->listLink = lruPages.last;
	}

	npages ++;
    }

    return page;
}

void
PagePool::releaseExtraPages ()
{
    while (npages > minPages &&
	   lruPages.first    &&
	   lruPages.first->data->busy == false)
    {
	lruPages.remove (lruPages.first);
	npages --;
    }
}

void
PagePool::lock ()
{
    pagesMutex.lock ();
}

void
PagePool::unlock ()
{
    pagesMutex.unlock ();
}

Ref<PagePool::Page>
PagePool::getPage (bool             locked,
		   ReclaimCallback  reclaimCallback,
		   void            *reclaimData)
{
    Ref<Page> page;

    if (lruPages.first == NULL ||
	lruPages.first->data->busy)
    {
	page = allocateExtraPage (locked);
    } else {
	page = lruPages.first->data;
	lruPages.remove (lruPages.first);

	if (page->busy) {
	    /* At this moment, the page is neither in lruPages nor
	     * in lockedPages. This means that the callee may perform
	     * any operation on PagePool in the callback. The only thing
	     * to be aware of in the callback is that pagesMutex is
	     * locked on entrance and must be locked on exiting.
	     *
	     * Although pagesMutex may be released in the middle
	     * of the callback, I don't see any possible use-cases
	     * where it would be necessary. */
	    page->reclaimCallback (page, page->reclaimData);
	}

	if (locked) {
	    lockedPages.append (page, lockedPages.last);
	    page->listLink = lockedPages.last;
	    page->locked = true;
	} else {
	    lruPages.append (page, lruPages.last);
	    page->listLink = lruPages.last;
	}
    }

    if (!page.isNull ()) {
	page->reclaimCallback = reclaimCallback;
	page->reclaimData     = reclaimData;
	page->busy = true;
    }

    return page;
}

void
PagePool::putPage (Page *page)
{
    if (page->busy == false)
	abortIfReached_str ("MyCpp.PagePool.putPage: can't put a free page");

    if (page->locked) {
	lockedPages.remove (page->listLink);
	page->locked = false;
    } else
	lruPages.remove (page->listLink);

    lruPages.prepend (page, lruPages.first);
    page->listLink = lruPages.first;
    page->busy = false;

    npages --;

    gotPagesInformer->informAll ((PagePool*) this);

    releaseExtraPages ();
}

void
PagePool::lockPage (Page *page)
{
    if (page->busy == false)
	abortIfReached_str ("MyCpp.PagePool.lockPage: can't lock a free page");

    if (!page->locked) {
	lruPages.remove (page->listLink);
	lockedPages.append (page, lockedPages.last);
	page->listLink = lockedPages.last;
	page->locked = true;
    }
}

void
PagePool::unlockPage (Page *page)
{
    if (page->busy == false)
	abortIfReached_str ("MyCpp.PagePool.lockPage: can't lock a free page");

    if (page->locked) {
	lockedPages.remove (page->listLink);
	lruPages.append (page, lruPages.last);
	page->listLink = lruPages.last;
	page->locked = false;
    }
}

void
PagePool::touchPage (Page *page)
{
    if (page->busy == false)
	abortIfReached_str ("MyCpp.PagePool.touchPage: can't touch a free page");

    if (!page->locked) {
	lruPages.remove (page->listLink);
	lruPages.append (page, lruPages.last);
	page->listLink = lruPages.last;
    }
}

void
PagePool::resetPage (Page *page)
{
    touchPage (page);
}

void
PagePool::informGotPages (GotPagesCallback  callback,
			  void             *callbackData,
			  void             *data)
{
    callback ((PagePool*) data, callbackData);
}

PagePool::PagePool (unsigned long pageSize,
		    unsigned long minPages,
		    unsigned long maxPages)
{
    gotPagesInformer = grab (new Informer<GotPagesCallback> (informGotPages));

    this->pageSize = pageSize;
    this->minPages = minPages;
    this->maxPages = maxPages;

    npages = 0;
}

}

