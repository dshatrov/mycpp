/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2009-2012 Dmitry Shatrov
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


#include <mycpp/unicode.h>


namespace MyCpp {

FuzzyResult unicode_isNewline (Iterator<Unichar const &> &char_iter,
			       Size *ret_num_chars)
{
    if (ret_num_chars != NULL)
	*ret_num_chars = 0;

    Bool cr;
    while (!char_iter.done ())
    {
	Unichar const &c = char_iter.next ();

	if (cr) {
	    if (ret_num_chars != NULL) {
		if (c == NewlineCharacter::LF)
		    *ret_num_chars = 2;
		else
		    *ret_num_chars = 1;
	    }

	    return FuzzyResult::Yes;
	}

	switch (c) {
	    case NewlineCharacter::CR:
		cr = true;
		break;
	    case NewlineCharacter::LF:
	    case NewlineCharacter::NEL:
	    case NewlineCharacter::FF:
	    case NewlineCharacter::LS:
	    case NewlineCharacter::PS:
		*ret_num_chars = 1;
		return FuzzyResult::Yes;
	    default:
		return FuzzyResult::No;
	}

    }

    abortIf (!cr);
    if (ret_num_chars != NULL)
	*ret_num_chars = 1;
    return FuzzyResult::Maybe;
}

}

