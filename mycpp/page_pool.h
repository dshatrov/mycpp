#ifndef __MYCPP__PAGE_POOL_H__
#define __MYCPP__PAGE_POOL_H__

#include <mycpp/object.h>
#include <mycpp/informer.h>
#include <mycpp/list.h>
#include <mycpp/mutex.h>

namespace MyCpp {

/* 07.04.24
 * PagePool is meant to be an interface, but since I'm not currently
 * planning to implement any other page replacement algorithm than
 * basic LRU, I leave this as a complete class, with a possibility
 * of making it an interface with the same name in mind.
 * All public methods of this class except the constructor are meant
 * to be present in such interface.
 */

/*c A limited set of pages of data.
 *
 * <t>PagePool</t> is a general-purpose centralized pool of pages.
 * Here, "general purpose" means that decision on page replacement
 * is based only on its access history. No factors other than
 * page accesses are taken into account, and nothing but the history
 * of page accesses contributes to the prediction about possibility
 * for this page to be used in future.
 *
 * The fact of page access is registered by calling the <c>touch</c>
 * method.
 */
class PagePool : public virtual Object
{
public:
    /*t A callback that gets called when one or more pages become
     * spare in the <t>PagePool</t>.
     *
     * @pagePool A <t>PagePool</t> that has triggered the event.
     * @userData User's data for this callback.
     */
    typedef void (*GotPagesCallback) (PagePool *pagePool,
				      void     *userData);

    class Page;

    /*t A callbcak that gets called when a page is about to be
     * reclaimed.
     *
     * Page reclamation is unconditional. After returning from this
     * callback, the user of the page loses any rights to use it,
     * and nothing can be done about this from within the callback.
     *
     * @page A page that is to be reclaimed.
     * @userData User's data for this callback.
     */
    typedef void (*ReclaimCallback) (Page *page,
				     void *userData);

    /*c A page of data. */
    class Page : public virtual SimplyReferenced
    {
	friend class PagePool;
    protected:
	List< Ref<Page> >::Element *listLink;

	ReclaimCallback  reclaimCallback;
	void            *reclaimData;

	bool busy;
	bool locked;

	Page (unsigned long size);

    public:
	/*> A pointer to page's data. */
	char *data;
	/*> Size of this page, in bytes. */
	unsigned long size;

	~Page ();
    };

protected:
    unsigned long pageSize;
    unsigned long minPages;
    unsigned long maxPages;
    unsigned long npages;

    List< Ref<Page> > lockedPages;
    List< Ref<Page> > lruPages;

    Mutex pagesMutex;

    Ref< Informer<GotPagesCallback> > gotPagesInformer;

    static void informGotPages (GotPagesCallback  callback,
				void             *callbackData,
				void             *data);

    void allocatePages ();

    Ref<Page> allocateExtraPage (bool locked);

    void releaseExtraPages ();

public:
    /*m*/
    Ref< Informer<GotPagesCallback> > getGotPagesInformer ()
    {
	return gotPagesInformer;
    }

    /*m Returns size of pages contained in the <t>PagePool</t>. */
    unsigned long getPageSize ();

    /*m Locks the <t>PagePool</t>. */
    void lock ();

    /*m Unlocks the <t>PagePool</t>. */
    void unlock ();

    /*m*/
    Ref<Page> getPage (bool             locked,
		       ReclaimCallback  reclaimCallback,
		       void            *reclaimData);

    /*m*/
    void putPage (Page *page);

    /*m*/
    void lockPage (Page *page);

    /*m*/
    void unlockPage (Page *page);

    /*m*/
    void touchPage (Page *page);

    /*m*/
    void resetPage (Page *page);

    /*m The constructor.
     *
     * @pageSize Size of pages to be stored in the <t>PagePool</t>.
     * @minPages Minimum number of pages to be stored in the <t>PagePool</t>
     *           at any time.
     * @maxPages Maximum number of pages that can be stored in
     *           the <t>PagePool</t> at one time.
     */
    PagePool (unsigned long pageSize,
	      unsigned long minPages,
	      unsigned long maxPages);
};

}

#endif /* __MYCPP__PAGE_POOL_H__ */

