#ifndef __MYCPP__VSLAB_H__
#define __MYCPP__VSLAB_H__

#include <mycpp/vstack.h>
#include <mycpp/intrusive_list.h>
#include <mycpp/io.h>


#define DEBUG(a) ;


namespace MyCpp {

template <class T> class VSlab;
template <class T> class VSlabRef;

class VSlabBase
{
    template <class T> friend class VSlabRef;
    template <class T> friend class VSlab;

public:
    class Node
    {
    public:
	Size refcount;
    };

    virtual void free (Node *node) = 0;

    virtual ~VSlabBase ()
    {
    }
};

template <class T>
class VSlab : public VSlabBase
{
    template <class C> friend class VSlabRef;

private:
    class Node : public VSlabBase::Node,
		 public IntrusiveListElement<>
    {
    public:
	T obj;
    };

public:
    class Ref
    {
	friend class VSlab;
	template <class C> friend class VSlabRef;

    private:
	VSlab *vslab;
	Node *node;

	Ref (VSlab * const vslab /* non-null */,
	     Node  * const node  /* non-null */)
	    : vslab (vslab),
	      node (node)
	{
	    node->refcount ++;
	}

    public:
	operator T* () const
	{
	    return &node->obj;
	}

	T& operator * () const
	{
	    return node->obj;
	}

	Ref& operator = (Ref const &ref)
	{
	    if (this == &ref)
		return *this;

	    if (node != NULL) {
		abortIf (node->refcount == 0);
		node->refcount --;
		if (node->refcount == 0)
		    vslab->free (node);
	    }

	    vslab = ref->vslab;
	    node = ref->node;

	    node->refcount ++;

	    return *this;
	}

	Ref (Ref const &ref)
	    : vslab (ref.vslab),
	      node (ref.node)
	{
	    node->refcount ++;
	}

	~Ref ()
	{
	    abortIf (node->refcount == 0);
	    node->refcount --;
	    if (node->refcount == 0)
		vslab->free (node);
	}
    };

    typedef Node* AllocKey;

private:
    IntrusiveList<Node> free_nodes;

#if 0
// Enable this after getting rid of AllocKey
    void free (Node * const node)
    {
	free_nodes.push_front (*node);

      // Note that no destructors are called.
    }
#endif

    virtual void free (VSlabBase::Node * const node)
    {
	free (static_cast <Node*> (node));
    }

public:
    VStack vstack;

    // Deprecated method
    T* alloc (AllocKey * const ret_key)
    {
	Node *node = NULL;
	if (free_nodes.isEmpty ()) {
	    node = new (vstack.push (sizeof (Node))) Node;
	    node->refcount = 1;
	} else {
	    node = free_nodes.getFirst ();
	    node->refcount = 1;
	    free_nodes.remove (free_nodes.getFirst ());
	}

	if (ret_key != NULL)
	    *ret_key = node;

	return &node->obj;
    }

    Ref alloc ()
    {
	Node *node = NULL;
	if (free_nodes.isEmpty ()) {
	    node = new (vstack.push (sizeof (Node))) Node;
	    node->refcount = 0;
	} else {
	    node = free_nodes.getFirst ();
	    node->refcount = 0;
	    free_nodes.remove (free_nodes.getFirst ());
	}

	DEBUG (
	    errf->print ("VSlab.alloc: returning node 0x").printHex ((Uint64) node).print (", obj 0x").printHex ((Uint64) &node->obj).pendl ();
	)

	return Ref (this, node);
    }

    // Deprecated method
    void free (AllocKey const key)
    {
	Node * const &node = key;

	DEBUG (
	    errf->print ("VSlab.free: freeing node 0x").printHex ((Uint64) node).print (", obj 0x").printHex ((Uint64) &node->obj).pendl ();
	)

	free_nodes.append (node);

      // Note that no destructors are called.
    }

    VSlab (Size prealloc = 1024)
	: vstack (prealloc * sizeof (T))
    {
    }

    ~VSlab ()
    {
      // TODO Call destructors for all free_nodes
    }
};

template <class T>
class VSlabRef
{
    template <class C> friend class VSlabRef;

private:
    VSlabBase *vslab;
    VSlabBase::Node *node;
    T *obj;

    VSlabRef (VSlabBase       * const vslab /* non-null */,
	      VSlabBase::Node * const node  /* non-null */,
	      T               * const obj   /* non-null */)
	: vslab (vslab),
	  node (node),
	  obj (obj)
    {
	node->refcount ++;
    }

public:
    bool isNull () const
    {
	return node == NULL;
    }

    operator T* () const
    {
	return obj;
    }

    T* operator -> () const
    {
	return obj;
    }

    T& operator * () const
    {
	return *obj;
    }

    VSlabRef& operator = (VSlabRef const &ref)
    {
	if (this == &ref)
	    return *this;

	if (node != NULL) {
	    abortIf (node->refcount == 0);
	    node->refcount --;
	    if (node->refcount == 0)
		vslab->free (node);
	}

	vslab = ref.vslab;
	node = ref.node;
	obj = ref.obj;

	if (node != NULL)
	    node->refcount ++;

	return *this;
    }

#if 0
    template <class C>
    VSlabRef& operator = (VSlabRef const &ref)
    {
	if (node != NULL) {
	    abortIf (node->refcount == 0);
	    node->refcount --;
	    if (node->refcount == 0)
		vslab->free (node);
	}

	vslab = ref.vslab;
	node = ref.node;
	obj = ref.obj;

	if (node != NULL)
	    node->refcount ++;
    }
#endif

    template <class C>
    VSlabRef& operator = (typename VSlab<C>::Ref const &ref)
    {
	if (node != NULL) {
	    abortIf (node->refcount == 0);
	    node->refcount --;
	    if (node->refcount == 0)
		vslab->free (node);
	}

	vslab = ref.vslab;
	node = ref.node;
	obj = ref.obj;

	if (node != NULL)
	    node->refcount ++;
    }

    VSlabRef (VSlabRef const &ref)
	: vslab (ref.vslab),
	  node (ref.node),
	  obj (ref.obj)
    {
	if (node != NULL)
	    node->refcount ++;
    }

    template <class C>
    VSlabRef (VSlabRef<C> const &ref)
	: vslab (ref.vslab),
	  node (ref.node),
	  obj (ref.obj)
    {
	if (node != NULL)
	    node->refcount ++;
    }

    template <class C>
    VSlabRef (typename VSlab<C>::Ref const &ref)
	: vslab (ref.vslab),
	  node (ref.node),
	  obj (ref.obj)
    {
	if (node != NULL)
	    node->refcount ++;
    }

    template <class C>
    static VSlabRef forRef (typename VSlab<C>::Ref const &ref)
    {
	return VSlabRef (ref.vslab, ref.node, &ref.node->obj);
    }

    VSlabRef ()
	: vslab (NULL),
	  node (NULL),
	  obj (NULL)
    {
    }

    ~VSlabRef ()
    {
	if (node == NULL)
	    return;

	abortIf (node->refcount == 0);
	node->refcount --;
	if (node->refcount == 0)
	    vslab->free (node);
    }
};

}


#undef DEBUG


#endif /* __MYCPP__VSLAB_H__ */

