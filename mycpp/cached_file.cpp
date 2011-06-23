#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/cached_file.h>

// Note: CachedFile may be the type of errf and outf.
#define DEBUG(a) ;

namespace MyCpp {

// Cached file. The cache is a simple page cache with some optimizations.
// 
// Page reclamation strategy: basic LRU (Least Recently Used) list.
//
// One may seek beyond the end of file provided that underlying file
// supports that.
//
// For consequent writes to a page we don't read that page until the first
// read operation.

// TODO: The user may be asking for data from WritePage which is already
// available, but we read from the underlynig file anyway.
// +TODO: getPage_read() and getPage_write() should accept desired file region
// as a parameter. Some relevant optimizations will be possible then.

// TODO: Flush dirty pages sorted by page offsets (AvlTree<Page*> dirty_pages).

// TODO: pread() and pwrite() would allow simultaneous calls
// to read() and write(), there would be a possibility to unlock state_mutex.

// TODO: Figure out what POSIX says about file positions for read() and write().

// FIXME: Why is NULL pointer sometimes passed to the underlying file's read() method?
//        (testing with MyDB)

// TODO: Don't remove WritePages from the cache after flushing.
//       This is non-trivial since we'll have to track the flushed part of the page.

// TODO: Make sure that LRU works as it should (untested).

// TODO: SeekEnd is untested.

static inline Uint64
seekPos_to_pageOffset (Uint64 seek_pos,
		       Size page_size)
{
    abortIf ((Size) (Uint64) page_size != page_size);
    return seek_pos - (seek_pos % (Uint64) page_size);
}

void
CachedFile::writePage (Page *_page)
    throw (IOException,
	   InternalException)
{
    abortIf (_page == NULL);

    state_mutex.assertLocked ();

    DEBUG (
	errf->print ("MyCpp.CachedFile.writePage: "
		     "_page->page_offset: ").print (_page->page_offset).pendl ();
    );

    if (_page->dirty_link == NULL)
	return;

    dirty_pages.remove (_page->dirty_link);
    _page->dirty_link = NULL;

    if (_page->page_type == Page::_WritePage) {
	WritePage * const &page = static_cast <WritePage*> (_page);

	if (page->data_len > 0) {
	    Uint64 pos = page->page_offset + (Uint64) page->data_start;
	    if (file_pos != pos) {
		file->seek (pos, SeekOrigin::SeekBeg);
		file_pos = pos;
	    }

	    abortIf (page->data_len > page_size   ||
		     page_size - page->data_len < page->data_start);
	    Size nwritten;
	    DEBUG (
		errf->print ("MyCpp.CachedFile.writePage: calling file->fullWrite(): "
			     "page->data_start: ").print (page->data_start).print (", "
			     "page->data_len: ").print (page->data_len).pendl ();
	    );
	    IOResult res = file->fullWrite (ConstMemoryDesc (page->data + page->data_start, page->data_len), &nwritten);
	    if (res != IOResultNormal) {
		is_error = true;

		if (res == IOResultEof)
		    throw IOException ();

		throw InternalException ();
	    }

	    if (nwritten != page->data_len) {
		is_error = true;
		throw InternalException ();
	    }

	    file_pos += nwritten;
	}
    } else
    if (_page->page_type == Page::_RwPage) {
	RwPage * const &page = static_cast <RwPage*> (_page);

	if (file_pos != page->page_offset) {
	    file->seek (page->page_offset, SeekOrigin::SeekBeg);
	    file_pos = page->page_offset;
	}

	Size towrite;
	if (page->page_offset == last_page_offset) {
	    abortIf (last_page_size > page_size);
	    towrite = last_page_size;
	} else
	    towrite = page_size;

	Size nwritten;
	DEBUG (
	    errf->print ("MyCpp.CachedFile.writePage: calling file->fullWrite(): "
			 "towrite: ").print (towrite).pendl ();
	);
	IOResult res = file->fullWrite (ConstMemoryDesc (page->data, towrite), &nwritten);
	if (res != IOResultNormal) {
	    is_error = true;

	    if (res == IOResultEof)
		throw IOException ();

	    throw InternalException ();
	}

	if (nwritten != towrite) {
	    is_error = true;
	    throw InternalException ();
	}

	file_pos += nwritten;
    } else
	abortIfReached ();
}

void
CachedFile::releasePage (Page *_page)
    throw (IOException,
	   InternalException)
{
  // Note: Don't release the data array and don't clobber any numeric fields.
  // They are supposed to be intact after releasePage() returns.

    abortIf (_page == NULL);

    state_mutex.assertLocked ();

    if (_page->dirty_link != NULL) {
/*
	errf->print ("MyCpp.CachedFile.releasePage: "
		     "WARNING: releasing a dirty page "
		     "(offset ").print (_page->page_offset).print (")").pendl ();
	abortIfReached ();
*/
	dirty_pages.remove (_page->dirty_link);
    }

    lru_list.remove (_page->lru_link);
    pages.remove (_page->pages_link);
}

Ref<CachedFile::RwPage>
CachedFile::convertPage (WritePage *old_page)
    throw (IOException,
	   InternalException)
{
    abortIf (old_page == NULL);

    state_mutex.assertLocked ();

    // We should hold an additional reference to the page so that it doesn't get deleted
    // after a call to releasePage().
    Ref<WritePage> old_page_ref (old_page);

    releasePage (old_page);

    Ref<RwPage> page = grab (new RwPage);
    page->page_offset = old_page->page_offset;
    page->data.allocate (page_size);

    // Read the page and copy the new data
    if (file_pos != old_page->page_offset) {
	file->seek (old_page->page_offset, SeekOrigin::SeekBeg);
	file_pos = old_page->page_offset;
    }

    Size nread;
    // TODO Read only unmodified part of the page (be careful when updating last_page_size below)
    IOResult res = file->fullRead (MemoryDesc (page->data, page_size), &nread);
    DEBUG (
	errf->print ("MyCpp.CachedFile.convertPage: fullRead(): ")
	     .print ("res: ").print (res == IOResultNormal ? "IOResultNormal" :
				     (res == IOResultAgain ? "IOResultAgain" :
				      (res == IOResultEof ? "IOResultEof" : "(invalid)")))
	     .print (", nread: ").print (nread).pendl ();
    );
    if (res != IOResultNormal) {
	if (res == IOResultEof) {
	    nread = 0;
	} else {
	    is_error = true;
	    throw InternalException ();
	}
    }

    file_pos += nread;

    // We can read a short page if none of the higher pages have been
    // written to disk yet, so nread can be less than page_size.

    if (old_page->data_start > nread) {
	DEBUG (
	    errf->print ("MyCpp.CachedFile.convertPage: calling zeroMemory()").pendl ();
	);
	zeroMemory (MemoryDesc (page->data, page_size).getRegion (nread, old_page->data_start - nread));
    }

    abortIf (last_page_offset < old_page->page_offset);
    if (last_page_offset == old_page->page_offset) {
	Size new_size;

	if (old_page->data_start + old_page->data_len > nread)
	    new_size = old_page->data_start + old_page->data_len;
	else
	    new_size = nread;

	// last_page_size is not valid here, since the last page was a WritePage

	DEBUG (
	    errf->print ("MyCpp.CachedFile.convertPage: "
			 "setting last_page_size: ").print (new_size).pendl ();
	);
	last_page_size = new_size;
    }

    if (old_page->data_len > 0) {
	abortIf (old_page->data_len > page_size ||
		 page_size - old_page->data_len < old_page->data_start);
	DEBUG (
	  errf->print ("MyCpp.CachedFile.convertPage: calling copyMemory(): ")
	       .print ("page_size: ").print (page_size).print (", ")
	       .print ("old_page->data_start: ").print (old_page->data_start).print (", ")
	       .print ("old_page->data_len: ").print (old_page->data_len).pendl ();
	);
	copyMemory (MemoryDesc (page->data, page_size).getRegionOffset (old_page->data_start),
		    ConstMemoryDesc (old_page->data + old_page->data_start, old_page->data_len));

	page->dirty_link = dirty_pages.append (page);
    } else {
	page->dirty_link = NULL;
    }

    page->lru_link = lru_list.prepend (page);
    page->pages_link = pages.add (static_cast <Page*> (page));

    return page;
}

Ref<CachedFile::Page>
CachedFile::getPage_write (Uint64 page_offset)
    throw (IOException,
	   InternalException)
{
    state_mutex.assertLocked ();

    DEBUG (
	errf->print ("MyCpp.CachedFile.getPage_write: "
		     "page_offset: ").print (page_offset).pendl ();
    );

    AvlTreeBase< Ref<Page> >::Node *page_node;
    page_node = pages.lookup (page_offset);

    if (page_node == NULL) {
	abortIf (num_pages > max_pages);
	if (num_pages == max_pages) {
	    List<Page*>::Element *lru_el = lru_list.last;
	    if (lru_el != NULL) {
		abortIf (lru_el->data == NULL);
		writePage (lru_el->data);
		releasePage (lru_el->data);
		// lru_el is not valid anymore

		num_pages --;
	    } else {
		// FIXME: Is it possible that there'll be no pages
		// in the LRU list?
		abortIfReached ();
	    }
	}

	Ref<WritePage> page = grab (new WritePage);
	page->page_offset = page_offset;
	page->data.allocate (page_size);
	page->data_start = 0;
	page->data_len = 0;
	page->dirty_link = NULL;
	page->lru_link = lru_list.prepend (page);
	page->pages_link = pages.add (static_cast <Page*> (page));

	num_pages ++;

	page_node = page->pages_link;

	if (page_offset > last_page_offset) {
	    AvlTreeBase< Ref<Page> >::Node *lp_node = pages.lookup (last_page_offset);
	    if (lp_node != NULL) {
		Ref<Page> const &_lp = lp_node->value;
		if (_lp->page_type == Page::_WritePage) {
		    // Do nothing
		} else
		if (_lp->page_type == Page::_RwPage) {
		    RwPage * const &lp = static_cast <RwPage*> (_lp.ptr ());
		    if (last_page_size < page_size) {
			DEBUG (
			    errf->print ("MyCpp.CachedFile.getPage_write: calling zeroMemory()").pendl ();
			);
			zeroMemory (MemoryDesc (lp->data, page_size).getRegionOffset (last_page_size));
		    }
		} else
		    abortIfReached ();
	    }

	    last_page_offset = page_offset;
	}
    } else {
	lru_list.remove (page_node->value->lru_link);
	page_node->value->lru_link = lru_list.prepend (page_node->value);
    }

    abortIf (page_node == NULL ||
	     page_node->value.isNull ());
    return page_node->value;
}

Ref<CachedFile::RwPage>
CachedFile::getPage_read (Uint64 page_offset)
    throw (IOException,
	   InternalException)
{
    state_mutex.assertLocked ();

    DEBUG (
	errf->print ("MyCpp.CachedFile.getPage_read: "
		     "page_offset: ").print (page_offset).pendl ();
    );

    AvlTreeBase< Ref<Page> >::Node *page_node;
    page_node = pages.lookup (page_offset);

    if (page_node == NULL) {
	DEBUG (
	    errf->print ("MyCpp.CachedFile.getPage_read: "
			 "the page is not in the cache").pendl ();
	);

	abortIf (num_pages > max_pages);
	if (num_pages == max_pages) {
	    List<Page*>::Element *lru_el = lru_list.last;
	    if (lru_el != NULL) {
		abortIf (lru_el->data == NULL);
		writePage (lru_el->data);
		releasePage (lru_el->data);
		// lru_el is not valid anymore

		num_pages --;
	    }
	}

	ArrayHolder<Byte> tmp_data (page_size);

	if (file_pos != page_offset) {
	    file->seek (page_offset, SeekOrigin::SeekBeg);
	    file_pos = page_offset;
	}

	Size nread;
	IOResult res = file->fullRead (MemoryDesc (tmp_data, page_size), &nread);
	if (res != IOResultNormal) {
	    if (res == IOResultEof)
		return NULL;

	    is_error = true;
	    throw InternalException ();
	}

	file_pos += nread;

	DEBUG (
	    errf->print ("MyCpp.CachedFile.getPage_read: "
			 "nread: ").print (nread).pendl ();
	);

	Ref<RwPage> page = grab (new RwPage);
	page->page_offset = page_offset;
	page->data.setPointer (tmp_data);
	tmp_data.releasePointer ();
	page->dirty_link = NULL;
	page->lru_link = lru_list.prepend (page);
	page->pages_link = pages.add (static_cast <Page*> (page));

	num_pages ++;

	page_node = page->pages_link;

	if (page_offset == last_page_offset) {
	    DEBUG (
		errf->print ("MyCpp.CachedFile.getPage_read: "
			     "setting last_page_size: ").print (nread).pendl ();
	    );
	    last_page_size = nread;
	} else
	if (page_offset > last_page_offset) {
	    AvlTreeBase< Ref<Page> >::Node *lp_node = pages.lookup (last_page_offset);
	    if (lp_node != NULL) {
		Ref<Page> const &_lp = lp_node->value;
		if (_lp->page_type == Page::_WritePage) {
		    // Do nothing
		} else
		if (_lp->page_type == Page::_RwPage) {
		    RwPage * const &lp = static_cast <RwPage*> (_lp.ptr ());
		    if (last_page_size < page_size) {
			DEBUG (
			    errf->print ("MyCpp.CachedFile.getPage_read: calling zeroMemory()").pendl ();
			);
			zeroMemory (MemoryDesc (lp->data, page_size).getRegionOffset (last_page_size));
		    }
		} else
		    abortIfReached ();
	    }

	    DEBUG (
		errf->print ("MyCpp.CachedFile.getPage_read: "
			     "setting last_page_size (#2): ").print (nread).pendl ();
	    );
	    last_page_size = nread;
	    last_page_offset = page_offset;
	}
    } else {
	abortIf (page_node->value.isNull ());

	DEBUG (
	    errf->print ("MyCpp.CachedFile.getPage_read: "
			 "the page is in the cache").pendl ();
	);

	if (page_node->value->page_type == Page::_WritePage) {
	    DEBUG (
		errf->print ("MyCpp.CachedFile.getPage_read: ")
		     .print ("calling convertPage(), page_offset: ").print (page_node->value->page_offset).pendl ();
	    );
	    Ref<RwPage> page = convertPage (static_cast <WritePage*> (page_node->value.ptr ()));
	    // page_node is not valid anymore

	    page_node = page->pages_link;
	} else {
	    lru_list.remove (page_node->value->lru_link);
	    page_node->value->lru_link = lru_list.prepend (page_node->value);
	}
    }

    abortIf (page_node == NULL ||
	     page_node->value.isNull () ||
	     page_node->value->page_type != Page::_RwPage);
    return static_cast <RwPage*> (page_node->value.ptr ());
}

IOResult
CachedFile::read (MemoryDesc const &mem,
		  Size *ret_nread)
    throw (IOException,
	   InternalException)
{
    if (ret_nread != NULL)
	*ret_nread = 0;

    Size remaining = mem.getLength ();

    Bool eof;

    {
      MutexLock state_lock (state_mutex);

	DEBUG (
	    errf->print ("MyCpp.CachedFile.read: ")
		 .print ("seek_pos: ").print (seek_pos).print (", ")
		 .print ("mem.getLength(): ").print (mem.getLength ()).pendl ();
	);

	if (is_error || closed)
	    throw InternalException ();

	if (mem.getMemory () == NULL ||
	    mem.getLength () == 0)
	{
	    return IOResultNormal;
	}

	Size req_offset = seekPos_to_pageOffset (seek_pos, page_size);
	while (remaining > 0) {
	    Ref<RwPage> page = getPage_read (req_offset);
	    if (page == NULL) {
		if (remaining == mem.getLength ()) {
		    DEBUG (
			errf->print ("MyCpp.CachedFile.read: "
				     "no page, returning Eof").pendl ();
		    );
		    return IOResultEof;
		}

		break;
	    }

	    abortIf (page->page_offset != req_offset);

	    Size tocopy;
	    if (remaining == mem.getLength ()) {
	      // First page

		DEBUG (
		    errf->print ("MyCpp.CachedFile.read: first page").pendl ();
		);

		Size data_start = seek_pos % page_size;

		if (page->page_offset == last_page_offset) {
		  // Last page

		    DEBUG (
			errf->print ("MyCpp.CachedFile.read: last page").pendl ();
		    );

		    // The last page in the cache is not the last
		    // in the file, but we've just called getPage_read(),
		    // so they should match.

		    abortIf (last_page_size > page_size);

		    if (last_page_size <= data_start) {
		      // Attempting to read beyound the end of the file
			DEBUG (
			    errf->print ("MyCpp.CachedFile.read: last_page_size <= data_start").pendl ();
			);
			tocopy = 0;
			eof = true;
		    } else {
			if (remaining < last_page_size - data_start)
			    tocopy = remaining;
			else
			    tocopy = last_page_size - data_start;
		    }
		} else {
		    if (remaining < page_size - data_start)
			tocopy = remaining;
		    else
			tocopy = page_size - data_start;
		}

		if (tocopy > 0)
		    copyMemory (mem, ConstMemoryDesc (page->data + data_start, tocopy));
	    } else {
		if (page->page_offset == last_page_offset) {
		  // Last page

		    DEBUG (
			errf->print ("MyCpp.CachedFile.read: last page").pendl ();
		    );

		    if (remaining < last_page_size)
			tocopy = remaining;
		    else
			tocopy = last_page_size;
		} else {
		    if (remaining < page_size)
			tocopy = remaining;
		    else
			tocopy = page_size;
		}

		copyMemory (mem.getRegionOffset (mem.getLength () - remaining), ConstMemoryDesc (page->data, tocopy));
	    }

	    remaining -= tocopy;
	    req_offset += page_size;

	    DEBUG (
		errf->print ("MyCpp.CachedFile.read: "
			     "tocopy: ").print (tocopy).print (", "
			     "req_offset: ").print (req_offset).pendl ();
	    );

	    if (page->page_offset == last_page_offset &&
		last_page_size < page_size)
	    {
		break;
	    }
	}

	seek_pos += mem.getLength () - remaining;
	DEBUG (
	    errf->print ("MyCpp.CachedFile.read: "
			 "setting seek_pos: ").print (seek_pos).pendl ();
	);
    }

    if (ret_nread != NULL)
	*ret_nread = mem.getLength () - remaining;

    if (eof)
	return IOResultEof;

    return IOResultNormal;
}

IOResult
CachedFile::write (ConstMemoryDesc const &mem,
		   Size *ret_nwritten)
    throw (IOException,
	   InternalException)
{
    DEBUG (
	errf->print ("MyCpp.CachedFile.write: "
		     "seek_pos: ").print (seek_pos).print (", "
		     "mem.getLength(): ").print (mem.getLength ()).pendl ();
    );

    if (ret_nwritten != NULL)
	*ret_nwritten = 0;

    Size remaining = mem.getLength ();

    {
      MutexLock state_lock (state_mutex);

	if (is_error || closed)
	    throw InternalException ();

	if (mem.getMemory () == NULL ||
	    mem.getLength () == 0)
	{
	    return IOResultNormal;
	}

	Size req_offset = seekPos_to_pageOffset (seek_pos, page_size);
	while (remaining > 0) {
	    Ref<Page> _page = getPage_write (req_offset);
	    abortIf (_page == NULL);

	    Size data_start;
	    Size tocopy;

	    if (remaining == mem.getLength ()) {
	      // First page

		data_start = seek_pos % page_size;

		if (remaining < page_size - data_start)
		    tocopy = remaining;
		else
		    tocopy = page_size - data_start;
	    } else {
		data_start = 0;

		if (remaining < page_size)
		    tocopy = remaining;
		else
		    tocopy = page_size;
	    }

	    if (_page->page_type == Page::_WritePage) {
		WritePage * const &page = static_cast <WritePage*> (_page.ptr ());

		abortIf (page->data_len > page_size ||
			 page_size - page->data_len < page->data_start);

		DEBUG (
		    errf->print ("MyCpp.CachedFile.write: ")
			 .print ("page->data_start: ").print (page->data_start).print (", ")
			 .print ("page->data_len: ").print (page->data_len).print (", ")
			 .print ("data_start: ").print (data_start).print (", ")
			 .print ("tocopy: ").print (tocopy).pendl ();
		);

		if (page->data_len > 0 &&
			(page->data_start + page->data_len < data_start ||
			 data_start + tocopy < page->data_start))
		{
		  // The regions do not overlap, so we're falling back to using RwPage.

		    DEBUG (
			errf->print ("MyCpp.CachedFile.write: ")
			     .print ("calling convertPage(), page_offset: ").print (page->page_offset).pendl ();
		    );
		    _page = convertPage (page);
		} else {
		    DEBUG (
			errf->print ("MyCpp.CachedFile.write: calling copyMemory(): "
				     "data_start: ").print (data_start).print (", "
				     "from (").print (mem.getLength () - remaining).print (", ")
			     .print (tocopy).print (")").pendl ();
		    );
		    copyMemory (MemoryDesc (page->data, page_size).getRegionOffset (data_start),
				mem.getRegion (mem.getLength () - remaining, tocopy));

		    if (page->data_len == 0) {
			page->data_start = data_start;
			page->data_len = tocopy;
		    } else {
			Size new_start = data_start <= page->data_start ? data_start : page->data_start;
			Size new_end = data_start + tocopy >= page->data_start + page->data_len ?
					       data_start + tocopy : page->data_start + page->data_len;

			page->data_start = new_start;
			page->data_len = new_end - new_start;
		    }
		}
	    }

	    if (_page->page_type == Page::_RwPage) {
		RwPage * const &page = static_cast <RwPage*> (_page.ptr ());

		DEBUG (
		    errf->print ("MyCpp.CachedFile.write: calling copyMemory(): "
				 "data_start: ").print (data_start).print (", "
				 "to (").print (mem.getLength () - remaining).print (", ")
			 .print (tocopy).print (")").pendl ();
		);
		copyMemory (MemoryDesc (page->data, page_size).getRegionOffset (data_start),
			    mem.getRegion (mem.getLength () - remaining, tocopy));

		if (last_page_offset == page->page_offset &&
		    data_start + tocopy > last_page_size)
		{
		    if (data_start > last_page_size) {
			DEBUG (
			    errf->print ("MyCpp.CachedFile.write: calling zeroMemory(): "
					 "last_page_size: ").print (last_page_size).print (", "
					 "data_start: ").print (data_start).pendl ();
			);
			zeroMemory (MemoryDesc (page->data + last_page_size, data_start - last_page_size));
		    }

		    DEBUG (
			errf->print ("MyCpp.CachedFile.write: "
				     "setting last_page_size: ").print (data_start + tocopy).pendl ();
		    );
		    last_page_size = data_start + tocopy;
		}
	    }

	    remaining -= tocopy;
	    req_offset += page_size;

	    DEBUG (
		errf->print ("MyCpp.CachedFile.write: ")
		     .print ("remaining: ").print (remaining).print (", ")
		     .print ("req_offset: ").print (req_offset).pendl ();
	    );

	    if (_page->dirty_link == NULL)
		_page->dirty_link = dirty_pages.append (_page);
	}

	seek_pos += mem.getLength () - remaining;
	DEBUG (
	    errf->print ("MyCpp.CachedFile.write: "
			 "setting seek_pos: ").print (seek_pos).pendl ();
	);
    }

    if (ret_nwritten != NULL)
	*ret_nwritten = mem.getLength () - remaining;

    return IOResultNormal;
}

// TODO Perhaps it would make sense to support keeping current "Cur" offset
// for the underlying file. A simple "seek_pos = file->tell ()" in the constructor
// would do.
void
CachedFile::seek (Int64 offset,
		  SeekOrigin origin)
    throw (IOException,
	   InternalException)
{
  MutexLock state_lock (state_mutex);

    if (is_error || closed)
	throw InternalException ();

    switch (origin) {
    case SeekOrigin::SeekBeg:
	abortIf (offset < 0);
	abortIf ((Int64) (Uint64) offset != offset);
	seek_pos = (Uint64) offset;
	DEBUG (
	    errf->print ("MyCpp.CachedFile.seek: SeekBeg: "
			 "setting seek_pos: ").print (seek_pos).pendl ();
	);

	break;
    case SeekOrigin::SeekCur:
	if (offset < 0) {
// TODO Sane signed/unsigned conversions
//	    abortIf ((Int64) (Uint64) (-offset) != -offset);
	    if ((Uint64) (-offset) > seek_pos) {
		is_error = true;
		throw InternalException ();
	    }

	    seek_pos -= (Uint64) (-offset);
	    DEBUG (
		errf->print ("MyCpp.CachedFile.seek: SeekCur: "
			     "setting seek_pos: ").print (seek_pos).pendl ();
	    );
	} else {
	    if ((Uint64) offset + seek_pos < seek_pos) {
		is_error = true;
		throw InternalException ();
	    }

	    seek_pos += (Uint64) offset;
	    DEBUG (
		errf->print ("MyCpp.CachedFile.seek: SeekCur: "
			     "setting seek_pos: ").print (seek_pos).pendl ();
	    );
	}

	break;
    case SeekOrigin::SeekEnd:
	if (!file_size_valid) {
	    // TODO File::tell () should throw IOExceptions
	    try {
		file->seek (0, SeekOrigin::SeekEnd);
	    } catch (IOException &exc) {
		is_error = true;
		throw InternalException (String::nullString (), exc.clone ());
	    }

	    file_pos = file->tell ();

	    AvlTreeBase< Ref<Page> >::Node *lp_node = pages.getRightmost ();
	    abortIf (lp_node == NULL);

	    Ref<Page> const &_lp = lp_node->value;
	    if (!_lp.isNull ()) {
		Uint64 lp_end = 0;

		if (_lp->page_type == Page::_WritePage) {
		    WritePage * const &lp = static_cast <WritePage*> (_lp.ptr ());
		    lp_end = last_page_offset + lp->data_start + lp->data_len;
		} else
		if (_lp->page_type == Page::_RwPage) {
		    lp_end = last_page_offset + last_page_size;
		} else
		    abortIfReached ();

		if (lp_end > file_pos)
		    file_size = lp_end;
		else
		    file_size = file_pos;
	    } else
		file_size = file_pos;

	    file_size_valid = true;
	}

	if (offset < 0) {
	    if ((Uint64) (-offset) > file_size) {
		is_error = true;
		throw InternalException ();
	    }

	    seek_pos = file_size - (Uint64) (-offset);
	    DEBUG (
		errf->print ("MyCpp.CachedFile.seek: SeekEnd: "
			     "setting seek_pos: ").print (seek_pos).pendl ();
	    );
	} else {
	    if (file_size + (Uint64) offset < file_size) {
		is_error = true;
		throw InternalException ();
	    }

	    seek_pos = file_size + (Uint64) offset;
	    DEBUG (
		errf->print ("MyCpp.CachedFile.seek: SeekEnd: "
			     "setting seek_pos: ").print (seek_pos).pendl ();
	    );
	}

	break;
    }
}

Uint64
CachedFile::tell ()
    throw (InternalException)
{
  MutexLock state_lock (state_mutex);

    if (is_error || closed)
	throw InternalException ();

    return seek_pos;
}

void
CachedFile::do_flush ()
    throw (IOException,
	   InternalException)
{
    state_mutex.assertLocked ();

    DEBUG (
	errf->print ("MyCpp.CachedFile.do_flush").pendl ();
    );

    List<Page*>::Element *page_el = dirty_pages.first;
    while (page_el != NULL) {
	Page *_page = page_el->data;
	abortIf (_page == NULL);

	page_el = page_el->next;

	DEBUG (
	    errf->print ("MyCpp.CachedFile.do_flush: calling writePage(): "
			 "_page->page_offset: ").print (_page->page_offset).pendl ();
	);
	writePage (_page);

	if (_page->page_type == Page::_WritePage) {
	    releasePage (_page);
	    abortIf (num_pages < 1);
	    num_pages --;
	} else
	if (_page->page_type == Page::_RwPage) {
	    _page->dirty_link = NULL;
	} else
	    abortIfReached ();
    }

    dirty_pages.clear ();
}

void
CachedFile::flush ()
    throw (IOException,
	   InternalException)
{
  MutexLock state_lock (state_mutex);

    if (is_error || closed)
	throw InternalException ();

    do_flush ();
}

void
CachedFile::sync ()
    throw (IOException,
	   InternalException)
{
    flush ();

    {
      MutexLock state_lock (state_mutex);

	if (is_error || closed)
	    throw InternalException ();
    }

    file->sync ();
}

void
CachedFile::close (bool flush_data)
    throw (IOException,
	   InternalException)
{
    {
      MutexLock state_lock (state_mutex);

	if (is_error || closed)
	    throw InternalException ();

	closed = true;

	if (flush_data)
	    do_flush ();
    }

    file->close (flush_data);
}

CachedFile::CachedFile (File *file,
			Size  page_size,
			Size  max_pages)
{
    abortIf (file == NULL);
    abortIf (page_size == 0);

    this->file = file;
    this->page_size = page_size;
    this->max_pages = max_pages;

    num_pages = 0;
    seek_pos = 0;
    file_pos = 0;

    last_page_offset = 0;
    last_page_size = 0;
}

}

