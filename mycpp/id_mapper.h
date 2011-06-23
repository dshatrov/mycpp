/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2008  Dmitry M. Shatrov
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

#ifndef __MYCPP__ID_MAPPER_H__
#define __MYCPP__ID_MAPPER_H__

#include <mycpp/simply_referenced.h>
#include <mycpp/iterator.h>

namespace MyCpp {

/*c <t>IdMapper</t> is responsible for
 * issuing unique numeric identificators
 * from a given range.
 *
 * Remember that IdMapper is NOT MT-safe.
 */
class IdMapperBase
{
public:
    /*s A single <t>IdMapper</t>'s ID node. */
    class Node
    {
    public:
	Node *top;		//< The parent node.
	Node *left;		//< The left subnode.
	Node *right;		//< The right subnode.
	int   balance;		/*< Node's balance
				 * (see <t>AvlTree::Node</t> for details. */

	unsigned long weight;	/*< Node's weight (total number of nodes
				 * in both subtrees). */

	unsigned long value;	/*< Node's value
				 * (the allocated identificator). */

	virtual ~Node ()
	{
	}
    };

    /* Goes from top to bottom, from left to right. */
    class TopLeftIterator : public StatefulIterator<Node&>
    {
    protected:
	Node *node;

    public:
	Node& next ()
	{
	    Node* ret = node;

	    assert (node);

	    if (node->left != NULL)
		node = node->left;
	    else
	    if (node->right != NULL) {
		node = node->right;
	    } else {
		while (node->top != NULL) {
		    if (node->top->right == node ||
			node->top->right == NULL)
		    {
			node = node->top;
		    } else {
			node = node->top->right;
			break;
		    }
		}

		if (node->top == NULL)
		    node = NULL;
	    }

	    return *ret;
	}

	bool done ()
	{
	    return node == NULL;
	}

	TopLeftIterator (IdMapperBase &mapper)
	{
	    node = mapper.top;
	}

	TopLeftIterator (Node *node)
	{
	    this->node = node;
	}
    };

protected:
    /*> The parent node. */
    Node *top;
    /*> Maximum identificator for this <t>IdMapper</t>. */
    unsigned long const limit;

    Node* rotateSingleLeft  (Node *node);
    Node* rotateSingleRight (Node *node);
    Node* rotateDoubleLeft  (Node *node);
    Node* rotateDoubleRight (Node *node);

    // The returned Node* will later be freed using 'delete' operator.
    virtual Node* newNode () = 0;

    /*m Allocates a new identificator.
     *
     * Returns NULL if out of free ids.
     *
     * @data User's data to be associated with the new identificator. */
    Node* add ();

    /*m Searches for a node with the given identificator.
     *
     * @value The idntificator of the node to be found. */
    Node* lookup (unsigned long value);

    /*m Removes a node from the mapper.
     *
     * @node The node to be removed from the mapper. */
    void remove (Node *node);

public:
    /*m Clears the mapper, removing all identificator mappings. */
    void clear ();

    /*m The constructor.
     *
     * @limit Maximum identificator for this <t>IdMapper</t>. */
    IdMapperBase (unsigned long limit);

    virtual ~IdMapperBase ();
};

template <class T>
class IdMapper : public IdMapperBase
{
public:
    class Node : public IdMapperBase::Node
    {
    public:
	T data;
    };

    class DataIterator : public StatefulIterator<T&>
    {
    protected:
	StatefulIterator<IdMapperBase::Node&> &iter;

    public:
	T& next ()
	{
	    return (static_cast <Node&> (iter.next ())).data;
	}

	bool done ()
	{
	    return iter.done ();
	}

	DataIterator (StatefulIterator<IdMapperBase::Node&> &iter)
	    : iter (iter)
	{
	}
    };

protected:
    Node* newNode ()
    {
	return new Node;
    }

public:
    Node* getTop ()
    {
	return static_cast <Node*> (top);
    }

    Node* add (const T &data)
    {
	Node *node = static_cast <Node*> (IdMapperBase::add ());
	node->data = data;
	return node;
    }

    Node* lookup (unsigned long value)
    {
	return static_cast <Node*> (IdMapperBase::lookup (value));
    }

    void remove (Node *node)
    {
	IdMapperBase::remove (node);
    }

    /* A limit of 0 means maximum possible limit. */
    IdMapper (unsigned long limit = 0)
	: IdMapperBase (limit)
    {
    }

#if 0
    IdMapper ()
	: IdMapperBase (0)
    {
    }
#endif
};

#if 0
template <class T>
class IdMapper_SR : public IdMapper<T>,
		    public virtual SimplyReferenced
{
    IdMapper_SR (unsigned long limit)
	: IdMapper<T> (limit)
    {
    }
};
#endif

}

#endif /* __MYCPP__ID_MAPPER_H__ */

