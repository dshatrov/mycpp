/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2008  Dmitry Shatrov
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

#ifndef __MYCPP__TREE_H__
#define __MYCPP__TREE_H__

#include <mycpp/simply_referenced.h>
#include <mycpp/avl_tree.h>
#include <mycpp/base_util.h>

namespace MyCpp {

template <class T, class K>
class Tree
{
protected:
    class Node
    {
    public:
	template <class C>
	class Comparator_Key : public Comparator<Node*, C>
	{
	protected:
	    Comparator<K, C> const &comparator;

	public:
	    bool greater (Node * const &node,
			  C      const &key) const
	    {
		return comparator.greater (node->key, key);
	    }

	    bool equals (Node * const &node,
			 C      const &key) const
	    {
		return comparator.equals (node->key, key);
	    }

	    Comparator_Key (Comparator<K, C> const &comparator)
		: comparator (comparator)
	    {
	    }
	};

	enum Type
	{
	    _Intermediate,
	    _Final
	};

	Type node_type;

	K key;

	Node (Type type)
	    : node_type (type)
	{
	}

	virtual ~Node ()
	{
	}
    };

public:
    class Node_Intermediate : public Node
    {
	friend class Tree;

    protected:
	AvlTree<Node*> subnodes;

	Node_Intermediate ()
	    : Node (Node::_Intermediate)
	{
	}

	~Node_Intermediate ()
	{
	    typename AvlTree<Node*>::TopLeftIterator subnodes_iter (subnodes);
	    while (!subnodes_iter.done ())
		delete subnodes_iter.next ().value;
	}
    };

    class Node_Final : public Node
    {
	friend class Tree;

    public:
	T value;

    protected:
	Node_Final ()
	    : Node (Node::_Final)
	{
	}
    };

protected:
    AvlTree<Node*> root_subnodes;

    template <class C>
    Node_Final* lookup_internal (Iterator<C> &key_iter,
				 Comparator<K, C> const &cmp,
				 bool create)
    {
	AvlTree<Node*> *cur_subnodes = &root_subnodes;
	while (!key_iter.done ()) {
	    C key = key_iter.next ();

	    typename AvlTree<Node*>::Node *avl_node = cur_subnodes->lookup (key, typename Node::template Comparator_Key<C> (cmp));
	    if (avl_node == NULL) {
		if (!create)
		    break;

		Node *new_node;
		if (!key_iter.done ())
		    new_node = new Node_Intermediate;
		else
		    new_node = new Node_Final;

		new_node->key = key;

		avl_node = cur_subnodes->addFor (key, typename Node::template Comparator_Key<C> (cmp));
		avl_node->value = new_node;
	    }

	    Node * const &node = avl_node->value;
	    if (node->node_type == Node::_Final) {
		if (key_iter.done ())
		    return static_cast <Node_Final*> (node);

		break;
	    }

	    if (node->node_type != Node::_Intermediate)
		abortIfReached ();

	    Node_Intermediate * const &int_node = static_cast <Node_Intermediate*> (node);
	    cur_subnodes = &int_node->subnodes;
	}

	return NULL;
    }

public:
    template <class C>
    Node_Final* lookup_addFor (Iterator<C> &key_iter,
			       Comparator<K, C> const &cmp)
    {
	return lookup_internal (key_iter, cmp, true /* create */);
    }

    template <class C>
    Node_Final* lookup (Iterator<C> &key_iter,
			Comparator<K, C> const &cmp)
    {
	return lookup_internal (key_iter, cmp, false /* create */);
    }

    template <class C>
    Node_Intermediate* lookupIntermediate (Node_Intermediate *node,
					   C const &key,
					   Comparator<K, C> const &cmp)
    {
	AvlTree<Node*> *subnodes;
	if (node == NULL)
	    subnodes = &root_subnodes;
	else
	    subnodes = &node->subnodes;

	typename AvlTree<Node*>::Node *avl_node = subnodes->lookup (key, typename Node::template Comparator_Key<C> (cmp));
	if (avl_node == NULL ||
	    avl_node->value->node_type != Node::_Intermediate)
	{
	    return NULL;
	}

	return static_cast <Node_Intermediate*> (avl_node->value);
    }

    template <class C>
    Node_Final* lookupFinal (Node_Intermediate *node,
			     C const &key,
			     Comparator<K, C> const &cmp)
    {
	AvlTree<Node*> *subnodes;
	if (node == NULL)
	    subnodes = &root_subnodes;
	else
	    subnodes = &node->subnodes;

	typename AvlTree<Node*>::Node *avl_node = subnodes->lookup (key, typename Node::template Comparator_Key<C> (cmp));
	if (avl_node == NULL ||
	    avl_node->value->node_type != Node::_Final)
	{
	    return NULL;
	}

	return static_cast <Node_Final*> (avl_node->value);
    }

    ~Tree ()
    {
	typename AvlTree<Node*>::TopLeftIterator root_subnodes_iter (root_subnodes);
	while (!root_subnodes_iter.done ())
	    delete root_subnodes_iter.next ().value;
    }
};

template <class K, class T>
class Tree_SR : public Tree <K, T>,
		public virtual SimplyReferenced
{
};

}

#endif /* __MYCPP__TREE_H__ */

