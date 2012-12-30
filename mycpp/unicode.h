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


#ifndef __MYCPP_UNICODE_H__
#define __MYCPP_UNICODE_H__


#include <glib.h>

#include <mycpp/types.h>
#include <mycpp/iterator.h>


namespace MyCpp {

typedef gunichar Unichar;

// Unicode code points for newline characters.
namespace NewlineCharacter {
    enum {
	CR  = 0x0d,
	LF  = 0x0a,
	NEL = 0x85,
	VT  = 0x0b, // not a newline character
	FF  = 0x0c,
	LS  = 0x2028,
	PS  = 0x2029
    };
}

static inline bool unicode_isAlphanumeric (Unichar c)
{
    return g_unichar_isalnum (c);
}

static inline bool unicode_isAlpha (Unichar c)
{
    return g_unichar_isalpha (c);
}

static inline bool unicode_isDigit (Unichar c)
{
    return g_unichar_isdigit (c);
}

static inline bool unicode_isSpace (Unichar c)
{
    return g_unichar_isspace (c);
}

// It's fuzzy because of CR+LF.
static inline FuzzyResult unicode_isNewline (Unichar c)
{
    switch (c) {
	case NewlineCharacter::CR:
	    return FuzzyResult::Maybe;
	case NewlineCharacter::LF:
	case NewlineCharacter::NEL:
	case NewlineCharacter::FF:
	case NewlineCharacter::LS:
	case NewlineCharacter::PS:
	    return FuzzyResult::Yes;
	default:
	    return FuzzyResult::No;
    }
}

// Recognizes "all possible" (according to the Unicode spec) newline combinations.
FuzzyResult unicode_isNewline (Iterator<Unichar const &> &char_iter,
			       Size *ret_num_chars);

}

#include <mycpp/utf8.h>


#endif /* __MYCPP_UNICODE_H__ */

