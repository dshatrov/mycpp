/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2009-2010 Dmitry Shatrov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <mycpp/comparator.h>
#include <mycpp/array_holder.h>
#include <mycpp/list.h>
#include <mycpp/avl_tree.h>
#include <mycpp/object.h>
#include <mycpp/mutex.h>
#include <mycpp/file.h>
#include <mycpp/base_util.h>

namespace MyCpp {

class CachedFile : public File,
		   public virtual Object
{
protected:
    class Page : public SimplyReferenced
    {
    public:
	enum PageType
	{
	    _WritePage = 0,
	    _RwPage
	};

	PageType page_type;
	Uint64 page_offset;
	List<Page*>::Element *lru_link;
//	AvlTree<Page*>::Node *dirty_link;
	List<Page*>::Element *dirty_link;
	AvlTreeBase< Ref<Page> >::Node *pages_link;

	Page (PageType page_type)
	{
	    this->page_type = page_type;
	}
    };

    class WritePage : public Page
    {
    public:
	ArrayHolder<Byte> data;

	Size data_start;
	Size data_len;

	WritePage ()
	    : Page (_WritePage)
	{
	}
    };

    class RwPage : public Page
    {
    public:
	ArrayHolder<Byte> data;

	RwPage ()
	    : Page (_RwPage)
	{
	}
    };

  // mt_const

    Ref<File> file;

    Size page_size;
    Size max_pages;

  // (end mt_const)

  // mt_mutex state_mutex

    AvlTree< Ref<Page>,
	     MemberExtractor< Page,
			      Uint64,
			      &Page::page_offset >,
	     DirectComparator<Uint64> >
	    pages;

    List<Page*> lru_list;
    Size num_pages;

//    AvlTree<Page*> dirty_pages;
    List<Page*> dirty_pages;

    // If true, then the last of known (touched) pages is in the page cache.
    Bool last_page_valid;
    Uint64 last_page_offset;
    // Note: last_page_size is valid only if the last page is an RwPage
    Size last_page_size;

    Bool file_size_valid;
    Uint64 file_size;

    // Seek position for this CachedFile
    Uint64 seek_pos;
    // Seek position for the underlying File
    Uint64 file_pos;

    Bool is_error;
    Bool closed;

  // (end mt_mutex state_mutex)

    Mutex state_mutex;

    void writePage (Page *_page)
	     throw (IOException,
		    InternalException);

    void releasePage (Page *_page)
	       throw (IOException,
		      InternalException);

    Ref<RwPage> convertPage (WritePage *old_page)
		      throw (IOException,
			     InternalException);

    Ref<Page> getPage_write (Uint64 page_offset)
		      throw (IOException,
			     InternalException);

    Ref<RwPage> getPage_read (Uint64 page_offset)
		       throw (IOException,
			      InternalException);

    void do_flush ()
	    throw (IOException,
		   InternalException);

public:
    IOResult read (MemoryDesc const &mem,
		   Size *nread)
	    throw (IOException,
		   InternalException);

    IOResult write (ConstMemoryDesc const &mem,
		    Size *nwritten)
	     throw (IOException,
		    InternalException);

    void seek (Int64 offset,
	       SeekOrigin origin)
	throw (IOException,
	       InternalException);

    Uint64 tell ()
	  throw (InternalException);

    void flush ()
	 throw (IOException,
		InternalException);

    void sync ()
	throw (IOException,
	       InternalException);

    void close (bool flush_data)
	 throw (IOException,
		InternalException);

    CachedFile (File *file,
		Size  page_size,
		Size  max_pages);
};

}

