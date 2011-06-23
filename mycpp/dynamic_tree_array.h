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

#ifndef __MYCPP__DYNAMIC_TREE_ARRAY_H__
#define __MYCPP__DYNAMIC_TREE_ARRAY_H__

#include <climits>

#include <mycpp/simply_referenced.h>
#include <mycpp/array.h>
#include <mycpp/util.h>
#include <mycpp/io.h>

#define DEBUG(a)

namespace MyCpp {

/*c A dynamic tree-based pseudo-array.
 *
 * This is an array that allocates memory by N-element blocks,
 * which are then placed into an N-leaf tree. */
template <class T>
class DynamicTreeArray : public Array <T>,
			 public virtual SimplyReferenced
{
protected:
    /* Базовый класс для общего типа указателей */
    class Node
    {
    public:
	virtual ~Node()
	{
	}
    };

    /* Блок содержит указатели на узлы нижнего уровня */
    class Block : public Node
    {
    protected:
	unsigned long nnodes;

    public:
	Node** next;

	Block (unsigned long nnodes)
	{
	    DEBUG( printf("new Block, %u (%x)\n", N, (unsigned)this); )

	    this->nnodes = nnodes;
	    next = new Node* [nnodes];
	    unsigned long i;
	    for(i = 0; i < nnodes; i++)
		next [i] = NULL;
	}

	~Block()
	{
	    DEBUG( printf("~Block (%x)\n", (unsigned)this); )

	    unsigned long i;
	    for (i = 0; i < nnodes; i++) {
		// Оптимизация: если встречен нулевой элемент, то и далее
		// все нулевые, за исключением случая, когда collapse выставляет
		// 1-й элемент в NULL, чтобы сохранить нужную ветвь дерева.
		if (next [i] == NULL) {
		    if (i != 0)
			break;
		} else
		    delete next [i];
	    }

	    delete[] next;
	}
    };

    /* Блок содержит массив с целевыми элементами */
    class Container : public Node
    {
    public:
	T *array;

	Container (unsigned long nnodes)
	{
	    DEBUG( printf("new Container, %u (%x)\n",
			  nnodes, (unsigned)this); )

	    array = new T [nnodes];
	}

	~Container()
	{
	    DEBUG( printf("~Container (%x)\n", (unsigned)this); )

	    delete[] array;
	}
    };

    Node *head;	                  // голова дерева
    unsigned nlevels;             // количество уровней дерева, не считая целевые элементы
    unsigned sizeLimit;           // общее количество целевых элементов в массиве
    unsigned long currentSize;

    unsigned nnodesPerBlock;	  // количество элементов в блоке
    unsigned blockSizeOrder;	  // степень двойки, nlementsPerBlock = 2 ** blockSizeOrder)

    /*
     * 0-й уровень - сами элементы
     * 1-й уровень - контейнеры
     * 2-й и далее - блоки
     */

    /* Логарифм по основанию 2, округленный в большую сторону 
     */
    unsigned long
    high_log2p (unsigned long blockSizeOrder,
		unsigned long desiredValue)
    {
	/* TODO Optimize this */

	if (desiredValue == 0)
	    abortIfReached ();

	unsigned long order = 0,
		      ret = 0,
		      curValue = 0,
		      prvValue = 0;
	while (curValue < desiredValue) {
	    curValue = 1 << order;
	    if (curValue <= prvValue)
		abortIfReached ();
	    prvValue = curValue;

	    order += blockSizeOrder;
	    ret ++;
	}

	return ret - 1;
    }

    // Количество узлов на уровне l
    unsigned long nnodesOnLevel (unsigned long level)
    {
	unsigned long power = 1 << (level * blockSizeOrder);
	return currentSize / power + (currentSize % power > 0 ? 1 : 0);
    }

    /* Расширение дерева - создание недостающих узлов и установка связей
     * pos - номер блока block на своём уровне, начиная с 0
     * l   - уровень блока block */
    void expand (Block         *block,
		 unsigned long  pos,
		 unsigned long  l)
    {
	/* TODO Implement in a non-recursive fashion. */

	DEBUG( printf("expand, block: %x, pos: %u, l: %u\n",
			(unsigned)block, pos, l); )

	if (l < 2)
	    abortIfReached ();

	// Узел имеет подузлы от 0 до limit (не включая limit,
	// т.е. limit подузлов).
	// Если узел является последним в уровне, то он не заполнен,
	// и для него limit <= nnodesPerBlock
	unsigned long limit;
	if (pos + 1 == nnodesOnLevel (l)) {
	    DEBUG( printf("pos + 1 == nnodesOnLevel (l) (== %u)\n", pos + 1); )

	    limit = nnodesOnLevel (l - 1) - pos * nnodesPerBlock;

	    DEBUG( printf("limit = %u\n", limit); )
	} else
	    limit = nnodesPerBlock;

	unsigned long i;
	for (i = 0; i < limit; i++) {
	    // Блоки до 2-го уровня содержат блоки,
	    // блоки 2-го уровня содержат контейнеры
	    if (l != 2) {
		// Если находимся в первой позиции уровня, в котором находится
		// корень старого дерева, то переносим старый корень
		// в эту позицию
		if (l == nlevels + 1 &&
		    pos == 0         &&
		    i == 0)
		{
		    DEBUG( printf("l == nlevels + 1 (== %u) && pos == 0\n", l); )
		    DEBUG( printf("\tkeeping block #0 (%x)\n",
				    (unsigned)head); )

		    // (nlevels >= 2) => (head != NULL)
		    block->next [0] = head;
		    expand (static_cast <Block*> (block->next [i]), 0, l - 1);
		} else {
		    DEBUG( printf("expanding block #%u\n", i); )

		    // Если указатель на узел != 0, то он относится к
		    // старому дереву
		    if (block->next [i] == NULL)
			block->next [i] = new Block (nnodesPerBlock);

		    expand (static_cast <Block*> (block->next [i]),
			    pos * nnodesPerBlock + i,
			    l - 1);
		}
	    } else {
	      /* Implies (l == 2) */
		// Если в старом дереве всего один уровень, то первый контейнер
		// первого уровня - это контейнер старого дерева
		if (pos == 0     &&
		    i == 0       &&
		    nlevels == 1 &&
		    head != NULL)
		{
		    DEBUG( printf("keeping container #0\n"); )

		    block->next [i] = head;
		} else {
		    DEBUG( printf("expanding container #%u\n", i); )

		    if (block->next [i] == NULL)
			block->next [i] = new Container (nnodesPerBlock);
		}
	    }
	}
    }

    /* Сжатие дерева - удаление лишних узлов
     * collapse нерекурсивна, поэтому содержит _все проверки,
     * в отличие от рекурсивной expand, проверки для которой выполняет resize */
    void collapse ()
    {
	DEBUG( printf("collapse\n"); )

	unsigned long newlevels = 0; // новое количество уровней
	if (currentSize > 1)
	    newlevels = high_log2p (blockSizeOrder, currentSize);
	else
	if (currentSize == 1)
	    newlevels = 1;

	DEBUG( printf("newlevels = %u\n", newlevels); )

	// Если количество уровней уменьшилось, очистим дерево так,
	// чтобы его корнем стал узел первого "нужного" уровня
	if (newlevels < nlevels) {
	    DEBUG( printf("cleaning up levels from %u down to %u\n",
		nlevels, newlevels); )

	    // На лишнем уровне путь к полезной части дерева проходит
	    // через первый узел
	    unsigned long i;
	    for (i = nlevels; i > newlevels && i > 1; i--) {
		Block *block = static_cast <Block*> (head);
		head = block->next [0];
		block->next [0] = NULL;

		DEBUG( printf("cleaning level #%u (%x)\n",
				i, (unsigned)block); )

		delete block;
	    }

	    // Если новое дерево пусто, очистим последний уровень отдельно
	    // (на этом этапе остался всего один контейнер)
	    if (newlevels == 0) {
		DEBUG( printf("deleting last container (%x)\n",
				(unsigned)head); )

		delete head;
		head = NULL;
	    }
	}

	nlevels = newlevels;

	// Теперь очистим каждый уровень от лишних узлов.

	// Избегаем переполнения: в пустом дереве нечего очищать.
	if (nlevels >= 2) {
	    Block *block = static_cast <Block*> (head);
	    unsigned long l;	// текущий уровень
	    for (l = nlevels - 1; l >= 1; l--) {
		// Индекс первого из блоков, подлежащих удалению.
		unsigned long bn = nnodesOnLevel (l) % nnodesPerBlock;

		DEBUG( printf("collapsing level %u: from %u to %u\n", l, bn, nnodesPerBlock); )

		unsigned long i;
		for (i = bn; i < nnodesPerBlock; i++) {
		    if (block->next [i] == NULL)
			break;

		    DEBUG( printf("deleting block #%u (%x)\n",
			    i, (unsigned) block->next [i]); )

		    delete block->next [i];
		    block->next [i] = NULL;
		}

		// После очистки верхнего уровня лишние блоки есть
		// только в самой левой ветви дерева.
		block = static_cast <Block*> (block->next [bn - 1]);
	    }
	}
    }

public:
    /*m Resizes this array.
     *
     * @newSize The new size of the array. */
    void resize (unsigned long newSize)
    {
	DEBUG( printf("resize, %u\n", newSize); )

	// currentSize должно быть обновлено до вызова expand / collapse
	if (newSize > currentSize) {
	    // Увеличение
	    currentSize = newSize;
	    unsigned long newlevels = 1; // новое кол-во уровней, не может равняться
					 // нулю, т. к. newSize > sizeLimit => newSize > 0
	    // Для newSize == 1 формула не работает: нет уровня "элемент",
	    // самый мелкий уровень - контейнер
	    if (newSize > 1)
		newlevels = high_log2p (blockSizeOrder, newSize);

	    Node *newhead = head;
	    // Отсутствие блоков (всего 1 контейнер) - особый случай,
	    // на который не рассчитан метод expand
	    if (nlevels == 0 && newlevels == 1)
		newhead = new Container (nnodesPerBlock);
	    else
	    if (nlevels == 1 && newlevels == 1) {
		// Как был 1 контейнер, так и остаётся
		DEBUG( printf("levels == 1 && newlevels == 1\n"); )
	    } else {
		// Если кол-во уровней выросло, то у дерева будет новый корень
		if (newlevels > nlevels)
		    newhead = new Block (nnodesPerBlock);

		expand (static_cast <Block*> (newhead), 0, newlevels);
	    }

	    head = newhead;
	    nlevels = newlevels;
	} else
	if (newSize < sizeLimit) {
	    // Уменьшение
	    currentSize = newSize;
	    collapse ();
	}
    }

  /* Array interface */

    unsigned long getCurrentSize ()
    {
	return currentSize;
    }

    unsigned long getSizeLimit ()
    {
	return sizeLimit;
    }

    /*m Provides access to an element with a specified index.
     *
     * @num The index of the element to access. */
    T& element (unsigned long index)
    {
	DEBUG( printf ("element, %u\n", index); )

	if (index >= sizeLimit)
	    abortIfReached ();

	if (index >= currentSize)
	    resize (index + 1);

	unsigned long i;
	Node *node = head;

	DEBUG( printf("running down from level %u\n", nlevels); )

	for (i = nlevels; i > 1; i--) {
	    unsigned long m = 1 << (blockSizeOrder * (i - 1));
	    unsigned long curIndex = index / m;
	    unsigned long nextIndex = index % m;

	    DEBUG( printf("diving to node #%u\n", curIndex); )

	    node = (static_cast <Block*> (node))->next [curIndex];
	    index = nextIndex;
	}

	DEBUG( printf("container index: %u\n", index); )

	return (static_cast <Container*> (node))->array [index];
    }

  /* (End of Array interface) */

    /*m The constructor.
     *
     * @blockSizeOrder The number of leaves per tree node and, at the same time,
     * the number of elements in a single array block. */
    DynamicTreeArray (unsigned long blockSizeOrder)
    {
	DEBUG( printf("new DynamicTreeArray, %u\n", blockSizeOrder); )
	this->blockSizeOrder = blockSizeOrder;

	nnodesPerBlock = 1 << blockSizeOrder;
	if (nnodesPerBlock == 0)
	    abortIfReached ();

	currentSize = 0;

	sizeLimit = 0;
	unsigned long i;
	for (i = 1; ; i++) {
	    if (blockSizeOrder * i >= sizeof (unsigned long) * CHAR_BIT)
		break;

	    sizeLimit = 1 << (blockSizeOrder * i);
	}

	head = NULL;
	nlevels = 0;
    }

    ~DynamicTreeArray ()
    {
	if (head != NULL)
	    delete head;
    }
};

#if 0
template <class T>
class DynamicTreeArray_SR : public DynamicTreeArray<T>,
			    public virtual SimplyReferenced
{
public:
    DynamicTreeArray_SR (unsigned long block_size_order)
	: DynamicTreeArray (block_size_order)
    {
    }
};
#endif

}

#endif /*__MYCPP__DYNAMIC_TREE_ARRAY_H__*/

