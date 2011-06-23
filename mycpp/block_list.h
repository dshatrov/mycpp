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

#ifndef __MYCPP_BLOCK_LIST_H__
#define __MYCPP_BLOCK_LIST_H__

#include <mycpp/enumeration.h>

namespace MyCpp {

/* c BlockList
 *
 * Динамический массив, выделяющий память блоками по N элементов,
 * строится на базе связного списка порядка N.
 * Ограничения: только рост, доступ к элементам последовательный
 * начиная с первого
 */
template <class T>
class BlockList {
protected:
    class Block {
    public:
	Block *next;
	T      data;
    };

    Block *first;   // первый блок в списке
    Block *last;    // последний блок в списке
    Block *current; // текущий блок в перечислении
    unsigned N;     // количество элементов в блоке
    unsigned n;     // общее количество целевых элементов в массиве
    unsigned lastSize;	// количество элементов в последнем блоке

    unsigned blocksNeeded (unsigned size) {
	if (size % N > 0)
	    return size / N + 1;

	return size / N;
    }

public:
    /* c*/
    class Data {
    public:
	T        *data;	//<
	unsigned  size;	//<
    };

    /*m*/
    void beginEnumeration () {
	current = first;
    }

    /*m*/
    void getNextBlock (Data *data) {
	if (current != NULL) {
	    data->data = &current->data;
	    if(current->next != NULL)
		data->size = N;
	    else
		data->size = lastSize;

	    current = current->next;
	} else {
	    data->data = NULL;
	    data->size = 0;
	}
    }

    /*m*/
    unsigned getSize () {
	return n;
    }

    /*m*/
    unsigned getBlockSize () {
	return N;
    }

    /*m Приписать данные в конец массива */
    void append (const T &data, unsigned size) {
	unsigned i, j;
	// 1) Выделение памяти
	unsigned blocks = blocksNeeded(n + size) - blocksNeeded(n);
	if (blocks > 0) {
	    Block *current = new Block;
	    if (first == NULL)
		first = current;
	    for (i = 0; i < blocks - 1; i++) {
		current->next = new Block;
		current = current->next;
	    }
	}

	// 2) Копирование
	unsigned pos = 0;
	// а) Заполнение свободных позиций в последнем блоке
	for (i = 0; (i < N - lastSize) && (pos < size); i++) {
	    last[ i + lastSize] = data [i];
	}

	// б) Заполнение новых блоков
	Block *current = last->next;
	unsigned limit = N;
	for (i = 0; i < blocks; i++) {
	    if (i == blocks - 1) {
		limit = newLastSize;
		last = current;
	    }

	    for (j = 0; j < limit; j++)
		current->data [j] = data [j];

	    current = current->next;
	}

	lastSize = newLastSize;
    }

    /*m*/
    BlockList (unsigned N) {
	this->N  = N;
	first    = NULL;
	current  = NULL;
	lastSize = 0;
    }
};

/* c BlockListEnumeration */
template <class T>
class BlockListEnumeration : public Enumeration<T> {
protected:
    BlockList<T> *dla;
    BlockList<T>::Data data;
    unsigned pos;
    unsigned globalPos;

public:
    /*m*/
    void start () {
	dla->beginEnumeration ();
	pos       = 0;
	globalPos = 0;
	data      = NULL;
    }

    /*m*/
    void hasNext () {
	if (globalPos == dla->getSize ())
	    return false;

	return true;
    }

    /*m*/
    T next() {
	if (pos % dla->getBlockSize () == 0) {
	    dla->getNextBlock (&data);
	    pos = 0;
	}

	pos ++;
	globalPos ++;
	return data->data [pos - 1];
    }

    /*m*/
    BlockListEnumeration (BlockList<T> *dla) {
	this->dla = dla;
    }
};

};

#endif /*__MYCPP_BLOCK_LIST_H__*/

