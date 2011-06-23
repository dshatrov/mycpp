/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006  Dmitry M. Shatrov
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

#include <mycpp/id_mapper.h>

namespace MyCpp {

IdMapperBase::Node*
IdMapperBase::rotateSingleLeft (Node *node)
{
    Node *bub = node->right;

    node->right = bub->left;
    if (bub->left != NULL)
	bub->left->top = node;

    bub->top = node->top;
    if (node->top != NULL) {
	if (node->top->left == node)
	    node->top->left = bub;
	else
	    node->top->right = bub;
    }

    bub->left = node;
    node->top = bub;

    if (bub->balance == 0) {
	bub->balance = -1;
	node->balance = 1;
    } else {
	bub->balance = 0;
	node->balance = 0;
    }

    bub->left->weight =
	(bub->left->left == NULL ?
	    0 : 1 + bub->left->left->weight) +
	(bub->left->right == NULL ?
	    0 : 1 + bub->left->right->weight);
    bub->weight = 2 +
	bub->left->weight + bub->right->weight;

    if (top == node)
	top = bub;

    return bub;
}

IdMapperBase::Node*
IdMapperBase::rotateSingleRight (Node *node)
{
    Node *bub = node->left;

    node->left = bub->right;
    if (bub->right != NULL)
	bub->right->top = node;

    bub->top = node->top;
    if (node->top != NULL) {
	if (node->top->right == node)
	    node->top->right = bub;
	else
	    node->top->left = bub;
    }

    bub->right = node;
    node->top = bub;

    if (bub->balance == 0) {
	bub->balance = 1;
	node->balance = -1;
    } else {
	bub->balance = 0;
	node->balance = 0;
    }

    bub->right->weight =
	(bub->right->right == NULL ?
	    0 : 1 + bub->right->right->weight) +
	(bub->right->left == NULL ?
	    0 : 1 + bub->right->left->weight);
    bub->weight = 2 +
	bub->right->weight + bub->left->weight;

    if (top == node)
	top = bub;

    return bub;
}

IdMapperBase::Node*
IdMapperBase::rotateDoubleLeft (Node *node)
{
    Node *bub = node->right->left;

    bub->top = node->top;
    if (node->top != NULL) {
	if (node->top->left == node)
	    node->top->left = bub;
	else
	    node->top->right = bub;
    }

    node->right->left = bub->right;
    if (bub->right != NULL)
	bub->right->top = node->right;

    bub->right = node->right;
    node->right->top = bub;

    node->right = bub->left;
    if (bub->left != NULL)
	bub->left->top = node;

    bub->left = node;
    node->top = bub;

    if (bub->balance == 0) {
	node->balance = 0;
	bub->right->balance = 0;
    } else
    if (bub->balance == -1) {
	node->balance = 0;
	bub->right->balance = 1;
    } else {
	node->balance = -1;
	bub->right->balance = 0;
    }
    bub->balance = 0;

    bub->left->weight =
	(bub->left->left == NULL ? 0 :
	    1 + bub->left->left->weight) +
	(bub->left->right == NULL ? 0 :
	    1 + bub->left->right->weight);
    bub->right->weight =
	(bub->right->left == NULL ? 0 :
	    1 + bub->right->left->weight) +
	(bub->right->right == NULL ? 0 :
	    1 + bub->right->right->weight);
    bub->weight = 2 +
	bub->left->weight + bub->right->weight;

    if (top == node)
	top = bub;

    return bub;
}

IdMapperBase::Node*
IdMapperBase::rotateDoubleRight (Node *node)
{
    Node *bub = node->left->right;

    bub->top = node->top;
    if (node->top != NULL) {
	if (node->top->right == node)
	    node->top->right = bub;
	else
	    node->top->left = bub;
    }

    node->left->right = bub->left;
    if (bub->left != NULL)
	bub->left->top = node->left;

    bub->left = node->left;
    node->left->top = bub;

    node->left = bub->right;
    if (bub->right != NULL)
	bub->right->top = node;

    bub->right = node;
    node->top = bub;

    if (bub->balance == 0) {
	node->balance = 0;
	bub->left->balance = 0;
    } else
    if (bub->balance == 1) {
	node->balance = 0;
	bub->left->balance = -1;
    } else {
	node->balance = 1;
	bub->left->balance = 0;
    }
    bub->balance = 0;

    bub->left->weight =
	(bub->left->left == NULL ? 0 :
	    1 + bub->left->left->weight) +
	(bub->left->right == NULL ? 0 :
	    1 + bub->left->right->weight);
    bub->right->weight =
	(bub->right->left == NULL ? 0 :
	    1 + bub->right->left->weight) +
	(bub->right->right == NULL ? 0 :
	    1 + bub->right->right->weight);
    bub->weight = 2 +
	bub->left->weight + bub->right->weight;

    if (top == node)
	top = bub;

    return bub;
}

IdMapperBase::Node*
IdMapperBase::add ()
{
    if (top != NULL &&
	top->weight == limit)
    {
	return NULL;
    }

    Node *node = newNode ();
    Node *ret = node;
    bool left = false;

    Node *cur = top;
    if (cur != NULL) {
	unsigned long w, tw;
	tw = 0;
	while (1) {
	    cur->weight ++;
	    w = cur->left == NULL ? 0 : cur->left->weight + 1;
	    if (w + tw < cur->value) {
		if (cur->left == NULL) {
		    cur->left = node;
		    left = true;
		    node->value = cur->value - 1;
		    break;
		}

		cur = cur->left;
	    } else {
		if (cur->right == NULL) {
		    cur->right = node;
		    left = false;
		    node->value = cur->value + 1;
		    break;
		}

		tw += 1 + w;
		cur = cur->right;
	    }
	}
    } else {
	top = node;
	node->value = 0;
    }

    node->top     = cur;
    node->left    = NULL;
    node->right   = NULL;
    node->balance = 0;
    node->weight  = 0;

    node = node->top;
    while (node != NULL) {
	if (left == false) {
	    if (node->balance == 1) {
		if (node->right->balance == -1)
		    node = rotateDoubleLeft (node);
		else
		    node = rotateSingleLeft (node);
	    } else
		node->balance ++;
	} else {
	    if (node->balance == -1) {
		if (node->left->balance == 1)
		    node = rotateDoubleRight (node);
		else
		    node = rotateSingleRight (node);
	    } else
		node->balance --;
	}

	if (node->top == NULL ||
	    node->balance == 0)
	    break;

	if (node->top->left == node)
	    left = true;
	else
	    left = false;

	node = node->top;
    }

    return ret;
}

void
IdMapperBase::remove (Node *node)
{
    Node *repl      = NULL,
	 *tobalance = NULL;
    bool left = false;

    if (node->balance == 0 &&
	node->left    == NULL)
    {
	if (node->top != NULL) {
	    if (node->top->left == node) {
		node->top->left = NULL;
		left = true;
	    } else {
		node->top->right = NULL;
		left = false;
	    }

	    tobalance = node->top;
	} else
	    top = NULL;

	delete node;
    } else {
	if (node->balance == 1) {
	    repl = node->right;
	    while (repl->left != NULL)
		repl = repl->left;

	    if (repl->top != node) {
		repl->top->left = repl->right;
		if (repl->right != NULL)
		    repl->right->top = repl->top;
		left = true;
	    } else
		left = false;
	} else {
	    repl = node->left;
	    while (repl->right != NULL)
		repl = repl->right;

	    if (repl->top != node) {
		repl->top->right = repl->left;
		if (repl->left != NULL)
		    repl->left->top = repl->top;
		left = false;
	    } else
		left = true;
	}

	repl->balance = node->balance;
	repl->weight  = node->weight;

	if (repl->top != node)
	    tobalance = repl->top;
	else
	    tobalance = repl;

	repl->top = node->top;
	if (node->left != repl) {
	    repl->left = node->left;
	    if (node->left != NULL)
		node->left->top = repl;
	}
	if (node->right != repl) {
	    repl->right = node->right;
	    if (node->right != NULL)
		node->right->top = repl;
	}

	if (node->top != NULL) {
	    if (node->top->left == node)
		node->top->left = repl;
	    else
		node->top->right = repl;
	}

	if (top == node)
	    top = repl;

	delete node;
    }

    node = tobalance;
    while (node != NULL) {
	node->weight --;
	if (left) {
	    if (node->balance == 1) {
		if (node->right->balance == -1)
		    node = rotateDoubleLeft (node);
		else
		    node = rotateSingleLeft (node);
	    } else
		node->balance ++;
	} else {
	    if (node->balance == -1) {
		if (node->left->balance == 1)
		    node = rotateDoubleRight (node);
		else
		    node = rotateSingleRight (node);
	    } else
		node->balance --;
	}

	if (node->top == NULL ||
	    node->balance != 0)
	    break;

	if (node->top->left == node)
	    left = true;
	else
	    left = false;

	node = node->top;
    }

    if (node != NULL) {
	node = node->top;
	while (node != NULL) {
	    node->weight --;
	    node = node->top;
	}
    }
}

void
IdMapperBase::clear ()
{
    Node *node,
	 *tmp;

    node = top;
    while (node != NULL) {
	while (1) {
	    if (node->left != NULL)
		node = node->left;
	    else
	    if (node->right != NULL)
		node = node->right;
	    else
		break;
	}

	while (1) {
	    tmp = node;
	    node = node->top;
	    delete tmp;

	    if (node == NULL)
		break;

	    if (node->left == tmp) {
		node->left = NULL;
		if (node->right != NULL) {
		    node = node->right;
		    break;
		}
	    } else
		node->right = NULL;
	}
    }

    top = NULL;
}

IdMapperBase::Node*
IdMapperBase::lookup (unsigned long value)
{
    Node *node = top;
    while (node != NULL) {
	if (node->value == value)
	    break;

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
    }

    return node;
}

IdMapperBase::IdMapperBase (unsigned long limit)
    : limit (limit == 0 ? ULONG_MAX : limit)
{
    top = NULL;
}

IdMapperBase::~IdMapperBase ()
{
    clear ();
}

}

