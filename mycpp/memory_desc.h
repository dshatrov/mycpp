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

#ifndef __MYCPP__MEMORY_DESC_H__
#define __MYCPP__MEMORY_DESC_H__


#include <libmary/types.h>


#include <mycpp/types.h>
#include <mycpp/base_util.h>


namespace MyCpp {

class MemoryDesc
{
protected:
    Byte *mem;
    Size  len;

public:
    Byte& operator [] (Size index) const
    {
	return mem [index];
    }

    Byte* getMemory () const
    {
	return mem;
    }

    Size getLength () const
    {
	return len;
    }

    MemoryDesc getRegion (Size region_start,
			  Size region_len) const
    {
	abortIf (region_start > len);
	abortIf (len - region_start < region_len);

	if (mem == NULL)
	    return MemoryDesc ((Byte*) NULL, region_len);

	return MemoryDesc (mem + region_start, region_len);
    }

    MemoryDesc getRegionOffset (Size region_start) const
    {
	abortIf (region_start > len);

	if (mem == NULL)
	    return MemoryDesc ((Byte*) NULL, len - region_start);

	return MemoryDesc (mem + region_start, len - region_start);
    }

    MemoryDesc& operator = (MemoryDesc const &mdesc)
    {
	mem = mdesc.getMemory ();
	len = mdesc.getLength ();
	return *this;
    }

    MemoryDesc (MemoryDesc const &mdesc)
    {
	mem = mdesc.getMemory ();
	len = mdesc.getLength ();
    }

    MemoryDesc (Byte *mem,
		Size  len)
    {
	this->mem = mem;
	this->len = len;
    }

    MemoryDesc (char *mem,
		Size  len)
    {
	// TODO reinterpret_cast?
	this->mem = reinterpret_cast <Byte*> (mem);
	this->len = len;
    }

    template <class T>
    static MemoryDesc forObject (T &obj)
    {
	// TODO C-style cast?
	return MemoryDesc ((Byte*) &obj, sizeof obj);
    }

    MemoryDesc ()
    {
	mem = NULL;
	len = 0;
    }

  // LibMary compatibility.

    operator M::Memory () const
    {
	return M::Memory ((M::Byte*) mem, len);
    }

    operator M::ConstMemory () const
    {
	return M::ConstMemory ((M::Byte const *) mem, len);
    }

    MemoryDesc (M::Memory const &mem)
	: mem (mem.mem()),
	  len (mem.len())
    {
    }
};

class ConstMemoryDesc
{
protected:
    const Byte *mem;
    Size len;

public:
    const Byte& operator [] (Size index) const
    {
	return mem [index];
    }

    const Byte* getMemory () const
    {
	return mem;
    }

    Size getLength () const
    {
	return len;
    }

    ConstMemoryDesc getRegion (Size region_start,
			       Size region_len) const
    {
	abortIf (region_start > len);
	abortIf (len - region_start < region_len);

	if (mem == NULL)
	    return ConstMemoryDesc ((Byte*) NULL, region_len);

	return ConstMemoryDesc (mem + region_start, region_len);
    }

    ConstMemoryDesc getRegionOffset (Size region_start) const
    {
	abortIf (region_start > len);

	if (mem == NULL)
	    return ConstMemoryDesc ((Byte*) NULL, len - region_start);

	return ConstMemoryDesc (mem + region_start, len - region_start);
    }

    ConstMemoryDesc& operator = (MemoryDesc const &mdesc)
    {
	mem = mdesc.getMemory ();
	len = mdesc.getLength ();
	return *this;
    }

    ConstMemoryDesc& operator = (ConstMemoryDesc const &mdesc)
    {
	mem = mdesc.getMemory ();
	len = mdesc.getLength ();
	return *this;
    }

    ConstMemoryDesc (MemoryDesc const &mdesc)
    {
	mem = mdesc.getMemory ();
	len = mdesc.getLength ();
    }

    ConstMemoryDesc (ConstMemoryDesc const &mdesc)
    {
	mem = mdesc.getMemory ();
	len = mdesc.getLength ();
    }

    ConstMemoryDesc (const Byte *mem,
		     Size len)
    {
	this->mem = mem;
	this->len = len;
    }

    ConstMemoryDesc (const char *mem,
		     Size len)
    {
	this->mem = reinterpret_cast <const Byte*> (mem);
	this->len = len;
    }

    ConstMemoryDesc (const char *str)
    {
	mem = reinterpret_cast <const Byte*> (str);
	if (str != NULL)
	    len = countStrLength (str);
	else
	    len = 0;
    }

    template <class T>
    static ConstMemoryDesc forObject (T &obj)
    {
	return ConstMemoryDesc ((Byte const *) &obj, sizeof (obj));
    }

    template <size_t N>
    static ConstMemoryDesc forString (const char (&str) [N])
    {
	abortIf (sizeof (str) == 0);
	return ConstMemoryDesc ((Byte const *) &str, sizeof (str) - 1);
    }

    ConstMemoryDesc ()
    {
	mem = NULL;
	len = 0;
    }

  // LibMary compatibility.

    operator M::ConstMemory () const
    {
	return M::ConstMemory ((M::Byte const *) mem, len);
    }

    ConstMemoryDesc (M::Memory const &mem)
	: mem (mem.mem()),
	  len (mem.len())
    {
    }

    ConstMemoryDesc (M::ConstMemory const &mem)
	: mem (mem.mem()),
	  len (mem.len())
    {
    }
};

}

#endif /* __MYCPP__MEMORY_DESC_H__ */

