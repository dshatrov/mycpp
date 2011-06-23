#ifndef __MYCPP__VSTACK_H__
#define __MYCPP__VSTACK_H__

#include <mycpp/types.h>
#include <mycpp/list.h>

#include <mycpp/io.h>


#define DEBUG(a) ;

#define FUNC_NAME(a) ;


namespace MyCpp {

class VStack
{
public:
    typedef Size Level;

private:
    class Block
    {
    public:
	Byte *buf;

	Size start_level;
	Size height;
    };

    Size const block_size;
    bool const shrinking;

    Size level;

    List<Block> blocks;
    List<Block>::Element *cur_block_el;

    Byte* addBlock (Size const num_bytes)
    {
      FUNC_NAME (
	static char const * const _func_name = "VStack.addBlock";
      )

	Byte *ret_buf = NULL;

	if (cur_block_el != NULL &&
	    cur_block_el->next != NULL)
	{
	    DEBUG (
		errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": reusing allocated block").pendl ();
	    )

	    Block &block = cur_block_el->next->data;
	    block.start_level = level;
	    block.height = num_bytes;

	    cur_block_el = cur_block_el->next;
	    ret_buf = block.buf;
	} else {
	    DEBUG (
		errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": allocating a new block").pendl ();
	    )

	    Block &block = blocks.appendEmpty ()->data;
	    block.buf = new Byte [block_size];
	    block.start_level = level;
	    block.height = num_bytes;

	    cur_block_el = blocks.last;
	    ret_buf = block.buf;
	}

	level += num_bytes;

	DEBUG (
	    errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": new level ").print (level).pendl ();
	)

	return ret_buf;
    }

public:
    Byte* push (Size num_bytes)
    {
      FUNC_NAME (
	static char const * const _func_name = "VStack.push";
      )

	DEBUG (
	    errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": ").print (num_bytes).print (" bytes").pendl ();
	)

	abortIf (num_bytes > block_size);

	if (blocks.isEmpty () ||
	    block_size - cur_block_el->data.height < num_bytes)
	{
	    return addBlock (num_bytes);
	}

	Block &block = cur_block_el->data;

	Size const prv_height = block.height;
	block.height += num_bytes;

	level += num_bytes;

	DEBUG (
	    errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": new level ").print (level).pendl ();
	)

	return block.buf + prv_height;
    }

    Byte* push_malign (Size num_bytes)
    {
	return push_malign (num_bytes, num_bytes);
    }

    // Returned address meets alignment requirements for an object of class A
    // if sizeof(A) is @alignment.
    // Returned address is always a multiple of @alignment bytes away from
    // the start of the corresponding block.
    Byte* push_malign (Size num_bytes, Size alignment)
    {
      FUNC_NAME (
	static char const * const _func_name = "VStack.push_malign";
      )

	DEBUG (
	    errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": ").print (num_bytes).print (" bytes, alignment ").print (alignment).pendl ();
	)

	abortIf (num_bytes > block_size);

	if (blocks.isEmpty () ||
	    block_size - cur_block_el->data.height < num_bytes)
	{
	    return addBlock (num_bytes);
	}

	Block &block = cur_block_el->data;

	Size new_height = block.height + num_bytes;
	Size delta = new_height % alignment;
	if (delta > 0)
	    new_height = new_height - delta + alignment;

	if (new_height > block_size)
	    return addBlock (num_bytes);

	Size const prv_height = block.height;
	block.height = new_height;

	level += new_height - prv_height;

	DEBUG (
	    errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": new level ").print (level).pendl ();
	)

	return block.buf + prv_height;
    }

    Level getLevel () const
    {
	return level;
    }

    void setLevel (Level const &new_level)
    {
      FUNC_NAME (
	static char const * const _func_name = "VStack.set_level";
      )

	DEBUG (
	    errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": level ").print (new_level).pendl ();
	)

	if (!blocks.isEmpty ()) {
	    Block &cur_block = cur_block_el->data;
	    if (cur_block.start_level > new_level) {
		List<Block>::Element * const prv_block_el = cur_block_el->previous;

		if (shrinking) {
		    DEBUG (
			errf->print (_func_name).print (" 0x").printHex ((Size) this).print (": deleting block").pendl ();
		    )

		    delete[] cur_block.buf;
		    blocks.remove (cur_block_el);
		  // 'cur_block_el' is not valid anymore
		}

		cur_block_el = prv_block_el;
	    } else {
		cur_block.height = new_level - cur_block.start_level;
	    }
	}

	level = new_level;
    }

    VStack (Size block_size /* > 0 */,
	    bool shrinking = false)
	: block_size (block_size),
	  shrinking (shrinking),
	  level (0),
	  cur_block_el (NULL)
    {
    }

    ~VStack ()
    {
	List<Block>::DataIterator blocks_iter (blocks);
	while (!blocks_iter.done ()) {
	    Block &block = blocks_iter.next ();
	    delete[] block.buf;
	}
    }
};

}


#ifdef DEBUG
#undef DEBUG
#endif

#ifdef FUNC_NAME
#undef FUNC_NAME
#endif


#endif /* __MYCPP__VSTACK_H__ */

