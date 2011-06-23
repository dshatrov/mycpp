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

#ifndef __MYCPP__LINEAR_BUFFER_INFO_H__
#define __MYCPP__LINEAR_BUFFER_INFO_H__

#include <mycpp/buffer.h>

namespace MyCpp {

class Buffer;

class LinearBufferInfo_param;

class LinearBufferInfo
{
public:
    Ref<Buffer> buffer;
    Size data_length;

    Byte& operator [] (Size index) const
    {
	abortIf (index >= data_length);
	return buffer.der () [index];
    }

    MemoryDesc getMemoryDesc () const
    {
	if (buffer.isNull ())
	    return MemoryDesc ();

	return buffer->getMemoryDesc (0, data_length);
    }

    MemoryDesc getMemoryDesc_offset (Size offset) const
    {
	if (buffer.isNull ()) {
	    abortIf (offset > 0);
	    return MemoryDesc ();
	}

	abortIf (offset > data_length);
	return buffer->getMemoryDesc (offset, data_length - offset);
    }

    void copy (ConstMemoryDesc const &mdesc)
    {
	*this = Buffer::forData (mdesc);
    }

    inline LinearBufferInfo (LinearBufferInfo_param const &buf_info_param);

    LinearBufferInfo (Buffer *buffer,
		      Size    data_length)
    {
	this->buffer = buffer;
	this->data_length = data_length;
    }

    LinearBufferInfo ()
    {
	data_length = 0;
    }
};

class LinearBufferInfo_param
{
public:
    Buffer *buffer;
    Size data_length;

    MemoryDesc getMemoryDesc () const
    {
	if (buffer == NULL)
	    return MemoryDesc ();

	return buffer->getMemoryDesc (0, data_length);
    }

    LinearBufferInfo_param (LinearBufferInfo const &buf_info)
    {
	buffer = buf_info.buffer;
	data_length = buf_info.data_length;
    }

    LinearBufferInfo_param (Buffer *buffer,
			    Size    data_length)
    {
	this->buffer = buffer;
	this->data_length = data_length;
    }

    LinearBufferInfo_param ()
    {
	buffer = NULL;
	data_length = 0;
    }
};

LinearBufferInfo::LinearBufferInfo (LinearBufferInfo_param const &buf_info_param)
{
    buffer = buf_info_param.buffer;
    data_length = buf_info_param.data_length;
}

}

#endif /* __MYCPP__LINEAR_BUFFER_INFO_H__ */

