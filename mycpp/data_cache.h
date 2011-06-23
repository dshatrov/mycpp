#ifndef __MYCPP_DATA_CACHE_H__
#define __MYCPP_DATA_CACHE_H__

#include <mycpp/object.h>
#include <mycpp/page_pool.h>
#include <mycpp/mutex.h>
#include <mycpp/avl_tree.h>
#include <mycpp/async_data_source.h>
#include <mycpp/async_data_acceptor.h>

namespace MyCpp {

/*c Data cache. */
class DataCache : public virtual Object
{
public:
    typedef void (*PageCallback) (DataCache *dataCache,
				  unsigned long long pos,
				  Ref<Exception>     exc,
				  char      *pageData,
				  void      *userData);

protected:
    class IssuedRequest;

    class Request : public SimplyReferenced
    {
    public:
	unsigned long long pos;

	PageCallback  pageCallback;
	void         *callbackData;

	RefCallback      refCallback;
	RefCallback      unrefCallback;
	void            *refData;

	IssuedRequest *issuedRequest;
    };

    class IssuedRequest : public SimplyReferenced
    {
    public:
	unsigned long long pos;

	Ref<AsyncDataSource::Request> adsRequest;
	List< Ref<Request> > ownRequests;
    };

    class DataPage {
    public:
	class UllComparator :
		public AvlTree<DataPage>::Comparator<unsigned long long>
	{
	public:
	    bool greater (const DataPage &dataPage,
			  const unsigned long long &pos) const
	    {
		return dataPage.pos > pos;
	    }

	    bool equals (const DataPage &dataPage,
			 const unsigned long long &pos) const
	    {
		return dataPage.pos == pos;
	    }
	};

	Ref<PagePool::Page> page;
	unsigned long long  pos;
	unsigned long       validLen;

	Ref<IssuedRequest> issuedRequest;

	bool reclaimed;
    };

    Ref<AsyncDataSource>   extSource;
    Ref<AsyncDataAcceptor> extAcceptor;

    Ref<PagePool> pagePool;
    unsigned long maxPages;
    unsigned long npages;

    AvlTree<DataPage> pages;
    Mutex pagesMutex;

    List<Request> requestQueue;

    bool preallocate;

    static char* getBuffer (bool   immediate,
			    unsigned long long pos,
			    unsigned long len,
			    void **bufferData,
			    void  *usreData);

    static void requestComplete (bool  immediate,
				 unsigned long long pos,
				 unsigned long len,
				 char *buf,
				 bool  aborted,
				 bool  eof,
				 Ref<Exception> exc,
				 void *bufferData,
				 void *userData);

public:
    /*m*/
    void lock ();

    /*m*/
    void unlock ();

    /*m*/
    unsigned long getPageSize ();

    /*m*/
    Ref<Request> requestPage (unsigned long long pos,
			      PageCallback  pageCallback,
			      void         *callbackData,
			      RefCallback   refCallback,
			      RefCallback   unrefCallback,
			      void         *refData);

    /*m*/
    void cancelRequest (Request *req);

    /*m*/
    void cancelAllRequests ();

    /*m The constructor. */
    DataCache (AsyncDataSource   *extSource,
	       AsyncDataAcceptor *extAcceptor,
	       PagePool          *pagePool,
	       unsigned long      maxPages,
	       bool               preallocate);
};

};

#endif /* __MYCPP_DATA_CACHE_H__ */

