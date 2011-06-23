#include <mycpp/direct_array_file.h>
#include <mycpp/base_util.h>
#include <mycpp/mem_util.h>

#include <mycpp/buffer.h>

namespace MyCpp {

static void
noop_allocate_memory (Buffer * /* self */,
		      Size     /* len */)
{
    // No-op
}

static void
noop_free_memory (Buffer * /* self */)
{
    // No-op
}

static Buffer::Vtable noop_vtable = {
    noop_allocate_memory,
    noop_free_memory
};

static void
def_allocate_memory (Buffer *self,
		     Size    len)
{
    self->data = new Byte [len];
}

static void
def_free_memory (Buffer *self)
{
    delete[] self->data;
}

static Buffer::Vtable def_vtable = {
    def_allocate_memory,
    def_free_memory
};

#if 0
MemoryDesc
Buffer::getMemoryDesc () const
{
    if (data == NULL)
	return MemoryDesc ();

    return MemoryDesc (data, size);
}
#endif

MemoryDesc
Buffer::getMemoryDesc (Size start_offset,
		       Size len) const
{
    if (data == NULL) {
	abortIf (start_offset != 0 ||
		 len != 0);

	return MemoryDesc ();
    }

    assertBoundaries (start_offset, len, size);

    return MemoryDesc (data + start_offset, len);
}

void
Buffer::writeMemory (ConstMemoryDesc const &mdesc,
		     Size start_offset,
		     Size len)
{
    if (data == NULL) {
	abortIf (start_offset != 0 ||
		 len != 0);
	return;
    }

    abortIf (len > mdesc.getLength ());
    assertBoundaries (start_offset, len, size);

    copyMemory (MemoryDesc (data + start_offset, size - start_offset), mdesc);
}

void
Buffer::allocate (Size len)
{
    if (data != NULL)
	vtable->freeMemory (this);

    if (len == 0) {
	data = NULL;
	this->size = 0;
    } else {
	vtable->allocateMemory (this, len);
	this->size = len;
    }
}

LinearBufferInfo
Buffer::createNew (Size len)
{
    LinearBufferInfo ret_info;

    ret_info.buffer = grab (new Buffer (len));
    ret_info.data_length = len;

    return ret_info;
}

LinearBufferInfo
Buffer::forPrintTask (const PrintTask &pt,
		      Size maxSize)
{
    LinearBufferInfo ret_info;

    Size len = pt.chainLength ();
    if (len > maxSize)
	return ret_info;

    ret_info.buffer = grab (new Buffer (len));
    ret_info.data_length = len;

    DirectArrayFile daf (ret_info.getMemoryDesc ());
    // FIXME chainOut() throws (IOException, InternalException),
    // but this method's declaration says nothing about that.
    pt.chainOut (&daf);
    daf.oflush ();

    return ret_info;
}

LinearBufferInfo
Buffer::forData (ConstMemoryDesc const &mdesc)
{
    LinearBufferInfo ret_info;
    ret_info.buffer = grab (new Buffer (mdesc.getLength ()));
    ret_info.data_length = mdesc.getLength ();

    copyMemory (ret_info.getMemoryDesc (), mdesc);

    return ret_info;
}

Buffer::Buffer (Size len)
{
    vtable = &def_vtable;

    if (len == 0) {
	data = NULL;
	this->size = 0;
    } else {
	vtable->allocateMemory (this, len);
	this->size = len;
    }
}

Buffer::Buffer ()
{
    vtable = &def_vtable;

    data = NULL;
    size = 0;
}

Buffer::~Buffer ()
{
    if (data != NULL)
	vtable->freeMemory (this);
}

static void
ext_allocate_memory (Buffer *_self,
		     Size    len)
{
    Buffer_ExtAlloc *self = static_cast <Buffer_ExtAlloc*> (_self);

    self->data = new Byte [len];
    self->free_callback = NULL;
}

static void
ext_free_memory (Buffer *_self)
{
    Buffer_ExtAlloc *self = static_cast <Buffer_ExtAlloc*> (_self);

    if (self->data == NULL)
	return;

    if (self->free_callback != NULL)
	self->free_callback (self->data);
    else
	delete[] self->data;
}

static Buffer::Vtable ext_vtable = {
    ext_allocate_memory,
    ext_free_memory
};

Buffer_ExtAlloc::Buffer_ExtAlloc (Byte         *ext_data,
				  Size          ext_size,
				  FreeCallback  ext_free_callback)
{
    vtable = &ext_vtable;

    data = ext_data;
    size = ext_size;
    free_callback = ext_free_callback;
}

Buffer_ExtAlloc::~Buffer_ExtAlloc ()
{
    vtable->freeMemory (this);
    data = NULL;

    vtable = &noop_vtable;
}

}

