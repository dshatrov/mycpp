#ifndef __MYCPP__MULTI_MAP_H__
#define __MYCPP__MULTI_MAP_H__

#include <mycpp/avl_tree.h>

namespace MyCpp {

template < class T,
	   class Extractor,
	   class Comparator >
	class MultiMap_anybase;

// Note: It is not recommended to refer to this class directly.
// Use MultiMap<>:: instead.
//
template <class T>
class MultiMap_common
{
public:
    // Copyable.
    class Entry
    {
	template < class C,
		   class Extractor,
		   class Comparator >
	friend class MultiMap_anybase;

#if 0
// TODO ? What are the rules for template friends?
	template < class Extractor,
		   class Comparator >
	friend class MultiMap_anybase< T, Extractor, Comparator >;
#endif

    private:
	typename AvlTreeBase<T>::Node *avl_tree_node;

    public:
	T& getData () const
	{
	    abortIf (avl_tree_node == NULL);
	    return avl_tree_node->value;
	}

	bool isNull () const
	{
	    return avl_tree_node == NULL;
	}

	static bool equal (Entry const &left,
			   Entry const &right)
	{
	    return left.avl_tree_node == right.avl_tree_node;
	}

	// Do not use this constructor in user's code.
	// It is public fot MyCpp::StatefulExtractorIterator.
	template <class C>
	Entry (C avl_tree_node)
	    : avl_tree_node (Pointer< typename AvlTreeBase<T>::Node > (avl_tree_node))
	{
	}

	Entry ()
	    : avl_tree_node (NULL)
	{
	}
    };

    // TODO Use typedefs instead all such macros.
    #ifdef MyCpp_MultiMap_Iterator_base
    #error
    #endif
    #define MyCpp_MultiMap_Iterator_base							\
	    StatefulExtractorIterator< Entry,						\
				       DirectExtractor< typename AvlTreeBase<T>::Node& >,	\
				       typename AvlTreeBase<T>::Iterator,			\
				       IteratorBase >

    template <class IteratorBase = EmptyBase>
    class Iterator_ : public MyCpp_MultiMap_Iterator_base
    {
    public:
	template < class Extractor,
		   class Comparator >
	Iterator_ (MultiMap_anybase< T, Extractor, Comparator > const &multi_map)
	    : MyCpp_MultiMap_Iterator_base (multi_map.avl_tree.createIterator ())
	{
	}
    };

    #undef MyCpp_MultiMap_Iterator_base

    typedef Iterator_<> Iterator;

    #ifdef MyCpp_MultiMap_DataIterator_base
    #error
    #endif
    // TODO Use AvlTree::DataIterator instead
    #define MyCpp_MultiMap_DataIterator_base						\
	    StatefulExtractorIterator< T&,						\
				       MemberExtractor< typename AvlTreeBase<T>::Node,	\
							T,				\
							&AvlTreeBase<T>::Node::value >,	\
				       typename AvlTreeBase<T>::Iterator,		\
				       IteratorBase >

    template <class IteratorBase = EmptyBase>
    class DataIterator_ : public MyCpp_MultiMap_DataIterator_base
    {
    public:
	template < class Extractor,
		   class Comparator >
	DataIterator_ (MultiMap_anybase< T, Extractor, Comparator > const &multi_map)
	    : MyCpp_MultiMap_DataIterator_base (multi_map.avl_tree.createIterator ())
	{
	}
    };

    #undef MyCpp_MultiMap_DataIterator_base

    typedef DataIterator_<> DataIterator;

    // FIXME Virtual destructor?? Why?
    virtual ~MultiMap_common ()
    {
    }
};

// Note: It is not recommended to refer to this class directly.
// Use MultiMap<>:: instead.
//
template < class T,
	   class Extractor,
	   class Comparator >
class MultiMap_anybase
	: public MultiMap_common<T>
{
    friend class MultiMap_common<T>;

private:
    typedef AvlTree< T, Extractor, Comparator> AvlTree_type;

public:
    #ifdef MyCpp_MultiMap_SameKeyIterator_base
    #error
    #endif
    #define MyCpp_MultiMap_SameKeyIterator_base							\
	    StatefulExtractorIterator< typename MultiMap_common<T>::Entry,			\
				       DirectExtractor< typename AvlTreeBase<T>::Node& >,	\
				       typename AvlTree_type::SameKeyIterator,			\
				       IteratorBase >

    template <class IteratorBase = EmptyBase>
    class SameKeyIterator_ : public MyCpp_MultiMap_SameKeyIterator_base
    {
    public:
	SameKeyIterator_ (typename MultiMap_common<T>::Entry const &entry)
	    : MyCpp_MultiMap_SameKeyIterator_base (typename AvlTree_type::SameKeyIterator (entry.avl_tree_node))
	{
	}
    };

    #undef MyCpp_MultiMap_SameKeyIterator_base

    typedef SameKeyIterator_<> SameKeyIterator;

    SameKeyIterator createSameKeyIterator (typename MultiMap_common<T>::Entry const &entry)
    {
	return SameKeyIterator (entry);
    }

    #ifdef MyCpp_MultiMap_SameKeyDataIterator_base
    #error
    #endif

    #define MyCpp_MultiMap_SameKeyDataIterator_base						\
	    StatefulExtractorIterator< T&,							\
				       DirectExtractor<T&>,					\
				       typename AvlTree_type::SameKeyDataIterator,		\
				       IteratorBase >

    template <class IteratorBase = EmptyBase>
    class SameKeyDataIterator_ : public MyCpp_MultiMap_SameKeyDataIterator_base
    {
    public:
	SameKeyDataIterator_ (typename MultiMap_common<T>::Entry const &entry)
	    : MyCpp_MultiMap_SameKeyDataIterator_base (typename AvlTree_type::SameKeyDataIterator (entry.avl_tree_node))
	{
	}
    };

    #undef MyCpp_MultiMap_SameKeyDataIterator_base

    typedef SameKeyDataIterator_<> SameKeyDataIterator;

    SameKeyDataIterator createSameKeyDataIterator (typename MultiMap_common<T>::Entry const &entry)
    {
	return SameKeyDataIterator (entry);
    }

private:
    AvlTree_type avl_tree;

    // Non-copyable.
    MultiMap_anybase& operator = (MultiMap_anybase const &);
    MultiMap_anybase (MultiMap_anybase const &);

public:
    typename MultiMap_common<T>::Entry add (T const &value)
    {
	return typename MultiMap_common<T>::Entry (avl_tree.add (value));
    }

    template <class C>
    typename MultiMap_common<T>::Entry addFor (C const &value)
    {
	return typename MultiMap_common<T>::Entry (avl_tree.addFor (value));
    }

#if 0
// TODO Is the variant below better? If not, then describe why.
    template <class C>
    typename MultiMap_common<T>::Entry lookupValue (C const &value)
    {
	return typename MultiMap_common<T>::Entry (avl_tree.lookupValue (value));
    }
#endif

//#if 0
    typename MultiMap_common<T>::Entry lookupValue (T const &value) const
    {
	return typename MultiMap_common<T>::Entry (avl_tree.lookupLeftmostValue (value));
    }
//#endif

    template <class C>
    typename MultiMap_common<T>::Entry lookup (C const &value) const
    {
	return typename MultiMap_common<T>::Entry (avl_tree.lookupLeftmost (value));
    }

    void remove (typename MultiMap_common<T>::Entry const &entry)
    {
	avl_tree.remove (entry.avl_tree_node);
    }

    void clear ()
    {
	avl_tree.clear ();
    }

    bool isEmpty () const
    {
	return avl_tree.top == NULL;
    }

    MultiMap_anybase ()
    {
    }
};

template < class T,
	   class Extractor = DirectExtractor<T>,
	   class Comparator = DirectComparator<T>,
	   class Base = EmptyBase >
class MultiMap
	: public MultiMap_anybase< T,
				   Extractor,
				   Comparator>,
	  public Base
{
};

}

#endif /* __MYCPP__MULTI_MAP_H__ */

