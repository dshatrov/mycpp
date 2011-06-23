#include <mycpp/data_cache.h>

namespace MyCpp {

void
DataCache::lock ()
{
    pagesMutex.lock ();
}

void
DataCache::unlock ()
{
    pagesMutex.unlock ();
}

char*
DataCache::getBuffer (bool   immediate,
		      unsigned long long pos,
		      unsigned long len,
		      void **bufferData,
		      void  *userData)
{
}

void
DataCache::
requestComplete (bool  immediate,
		 unsigned long long pos,
		 unsigned long len,
		 char *buf,
		 bool  aborted,
		 bool  eof,
		 Ref<Exception> exc,
		 void *bufferData,
		 void *userData)
{
}

Ref<DataCache::Request>
DataCache::requestPage (unsigned long long pos,
			PageCallback     pageCallback,
			void            *callbackData,
			RefCallback      refCallback,
			RefCallback      unrefCallback,
			void            *refData)
    throw (InternalException)
{
    pagesMutex.lock ();

    /* The first thing to do is to check for each potential
     * request whether the data for it is already available
     * in the cache. If the data is already available, we return it
     * to the caller immediately, othrewise we issue a request
     * to the underlying AsyncDataSource if it is not issued allready.
     */

    DataPage *dataPage;

    AvlTree<DataPage>::Node *pageNode;
    pageNode = pages.lookup (page, DataPage::UllComparator ());
    if (pageNode != NULL) {
	dataPage = &pageNode->value;

	pagePool->lock ();

	if (dataPage->reclaimed) {
	    /* Bad luck. The page has just been reclaimed,
	     * we can do nothing about it. */
	}

	if (dataPage->issuedRequest.isNull () &&
	    !dataPage->page.isNull ())
	{
	    /* Cache hit */
	    if (refCallback != NULL)
		refCallback (refData);

	    pagesMutex.unlock ();

	    if (pageCallback != NULL)
		pageCallback (this,
			      pos,
			      NULL,
			      dataPage->page->data,
			      callbackData);

	    if (unrefCallback != NULL)
		unrefCallback (refData);

	    return;
	}
    } else {
	pageNode = pages.addFor (pos, DataPage::UllComparator ());
	dataPage = pageNode->value;

	dataPage->pos = pos;
	dataPage->page = NULL;
	dataPage->validLen = 0;
	dataPage->issuedRequest = NULL;
	dataPage->reclaimed = false;
    }

    Ref<DataCache::Request> req = grab (new DataCache::Request);
    req->pos           = pos;
    req->pageCallback  = pageCallback;
    req->callbackData  = callbackData;
    req->refCallback   = refCallback;
    req->unrefCallback = unrefCallback;
    req->refData       = refData;

    if (dataPage->issuedRequest.isNull ()) {
	Ref<IssuedRequest> ir = grab (new IssuedRequest);
	dataPage->issuedRequest = ir;

	ir->pos = pos;

	dataPage->issuedRequest->ownRequests.append (
		req,
		dataPage->issuedRequest->ownRequests.last);

	try {
	    ir->adsRequest = extSource->requestData (
				     pos,
				     pagePool->getPageSize (),
				     getBufferCallback,
				     requestComplete,
				     (DataPage*) dataPage,
				     referencedRefCallback,
				     referencedUnrefCallback,
				     (Referenced*) this);
	} catch (InternalException &exc) {
	    pages.remove (pageNode);
	    pagesMutex.unlock ();
	    throw InternalException (String::nullString (), exc.clone);
	}
    } else
	dataPage->issuedRequest->ownRequests.append (
		req,
		dataPage->issuedRequest->ownRequests.last);

    pagesMutex.unlock ();

    return NULL;
}

void
DataCache::Source::
cancelRequest (Request *request)
{
}

void
DataCache::Source::
cancelAllRequests ()
{
}

DataCache::Acceptor::
Acceptor (DataCache *dcache)
{
    this->dcache = dcache;
}

unsigned long
DataCache::Acceptor::
submitData (unsigned long long pos,
	    const char    *buf,
	    unsigned long  len)
{
    return 0;
}

Ref<AsyncDataSource>
DataCache::getSource ()
{
    return Ref<AsyncDataSource> (&source, this);
}

Ref<AsyncDataAcceptor>
DataCache::getAcceptor ()
{
    return Ref<AsyncDataAcceptor> (&acceptor, this);
}

DataCache::DataCache (AsyncDataSource   *extSource,
		      AsyncDataAcceptor *extAcceptor,
		      PagePool          *pagePool,
		      unsigned long      maxPages,
		      bool               preallocate)
    : source   (this),
      acceptor (this)
{
    this->extSource   = extSource;
    this->extAcceptor = extAcceptor;

    this->pagePool = pagePool;
    this->maxPages = maxPages;

    npages = 0;
}

};

