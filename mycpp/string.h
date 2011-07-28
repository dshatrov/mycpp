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

#ifndef __MYCPP__STRING_H__
#define __MYCPP__STRING_H__

#include <mycpp/simply_referenced.h>
#include <mycpp/memory_desc.h>
#include <mycpp/iterator.h>
#include <mycpp/base_util.h>
#include <mycpp/mem_util.h>
#include <mycpp/strutil.h>

namespace MyCpp {

class PrintTask;

class String : public virtual SimplyReferenced
{
protected:
    char *data;
    unsigned long length;

    virtual void allocateMemory (Size len)
    {
	data = new char [len];
    }

    virtual void freeMemory ()
    {
	delete[] data;
    }

private:
    /* Using implicit assignment and copying is discouraged
     * for String objects. Corresponding methods should be used instead. */

    String& operator = (String const &);
    String (String const &str);

public:
    static char * const noData;

    // TODO getMemoryDesc() calls should not be necessary
    //      (implicit type conversion is needed).
    MemoryDesc getMemoryDesc () const
    {
	return MemoryDesc (data, length);
    }

#if 0
    ConstMemoryDesc getConstMemoryDesc () const
    {
	return ConstMemoryDesc (data, length);
    }
#endif

    char* getData () const
    {
	return data;
    }

    unsigned long getLength () const
    {
	return length;
    }

    // Use this carefully.
    void setLength (unsigned long const length) {
	this->length = length;
    }

    bool equalsTo (const char *str) const
    {
	if (str == NULL) {
	    if (data == noData)
		return true;

	    return false;
	}

	return compareStrings (data, str);
    }

    bool greaterThan (const char *str) const
    {
	if (orderStrings (data, str) == ComparisonGreater)
	    return true;

	return false;
    }

    const char* releasePointer ()
    {
	const char *ret_ptr = this->data;
	this->data = noData;
	length = 0;
	return ret_ptr;
    }

    void setOwn (char *data)
    {
	if (this->data != noData)
	    freeMemory ();

	if (data == NULL) {
	    this->data = noData;
	    length = 0;
	} else {
	    length = countStrLength (data);
	    this->data = data;
	}
    }

    void set (const char *data)
    {
	if (this->data != noData)
	    freeMemory ();

	if (data == NULL) {
	    this->data = noData;
	    length = 0;
	} else {
	    length = countStrLength (data);
	    allocateMemory (length + 1);
	    copyMemory (MemoryDesc (this->data, length + 1), ConstMemoryDesc (data, length + 1));
	}
    }

    void set (const char    *data,
	      unsigned long  len)
    {
	if (this->data != noData)
	    freeMemory ();

	if (data == NULL ||
	    len  == 0)
	{
	    this->data = noData;
	    this->length = 0;
	} else {
	    this->length = len;
	    allocateMemory (len + 1);
	    copyMemory (MemoryDesc (this->data, len + 1), ConstMemoryDesc (data, len));
	    this->data [len] = 0;
	}
    }

    /* Note that allocate() allocates an additional byte
     * for the trailing zero. */
    void allocate (unsigned long len)
    {
	if (data != noData)
	    freeMemory ();

	if (len == 0) {
	    data = noData;
	    this->length = 0;
	} else {
	    allocateMemory (len + 1);
	    data [0] = 0;
	    this->length = len;
	}
    }

    bool isNullString () const
    {
	return data == noData;
    }

    static Ref<String> nullString ()
    {
	return grab (new String ());
    }

    static Ref<String> forData (const char *str)
    {
	return grab (new String (str));
    }

    static Ref<String> catenate (Iterator<String&> &str_iter);

    static Ref<String> forPrintTask (const PrintTask &pt);

    operator char * () const
    {
	return getData ();
    }

    String& operator = (ConstMemoryDesc const &mdesc)
    {
	set ((const char*) mdesc.getMemory (), mdesc.getLength ());
	return *this;
    }

    String (const char *data)
    {
	if (data == NULL) {
	    this->data = noData;
	    length = 0;
	} else {
	    length = countStrLength (data);
	    if (length == 0)
		this->data = noData;
	    else {
		allocateMemory (length + 1);
		copyMemory (MemoryDesc (this->data, length + 1), ConstMemoryDesc (data, length + 1));
	    }
	}
    }

    String (const char    *data,
	    unsigned long  len)
    {
	if (data == NULL ||
	    len  == 0)
	{
	    this->data = noData;
	    this->length = 0;
	} else {
	    this->length = len;
	    allocateMemory (len + 1);
	    copyMemory (MemoryDesc (this->data, len + 1), ConstMemoryDesc (data, len));
	    this->data [len] = 0;
	}
    }

    String (ConstMemoryDesc const &mdesc)
    {
	length = mdesc.getLength ();
	allocateMemory (mdesc.getLength () + 1);
	copyMemory (MemoryDesc (data, mdesc.getLength ()),
		    ConstMemoryDesc (mdesc.getMemory (), mdesc.getLength ()));
	data [mdesc.getLength ()] = 0;
    }

    String ()
    {
	data = noData;
	length = 0;
    }

    virtual ~String ()
    {
	if (data != noData)
	    freeMemory ();
    }
};

class String_ExtAlloc : public String
{
public:
    typedef void (*FreeCallback) (void *data);

protected:
    FreeCallback free_callback;

    void allocateMemory (Size len)
    {
	data = new char [len];
	free_callback = NULL;
    }

    void freeMemory ()
    {
	if (data == NULL ||
	    data == noData)
	{
	    return;
	}

	if (free_callback != NULL)
	    free_callback (data);
	else
	    delete[] data;
    }

public:
    String_ExtAlloc (char         *ext_data,
		     Size          ext_length,
		     FreeCallback  ext_free_callback)
    {
	data = ext_data;
	length = ext_length;
	free_callback = ext_free_callback;
    }

    ~String_ExtAlloc ()
    {
	if (data != noData) {
	    freeMemory ();

	    data = noData;
	    length = 0;
	}
    }
};

}

#endif /* __MYCPP__STRING_H__ */

