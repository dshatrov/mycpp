#ifndef __MYCPP_UNICODE_H__
#define __MYCPP_UNICODE_H__

//#include <glib/gunicode.h>
#include <glib.h>

#include <mycpp/types.h>
#include <mycpp/iterator.h>

namespace MyCpp {

typedef gunichar Unichar;

bool unicode_isAlphanumeric (Unichar c);

bool unicode_isAlpha (Unichar c);

bool unicode_isDigit (Unichar c);

bool unicode_isSpace (Unichar c);

// It's fuzzy because of CR+LF.
FuzzyResult unicode_isNewline (Unichar c);

// Recognizes "all possible" (according to the Unicode spec) newline combinations.
FuzzyResult unicode_isNewline (Iterator<Unichar const &> &char_iter,
			       Size *ret_num_chars);

}

#include <mycpp/utf8.h>

#endif /* __MYCPP_UNICODE_H__ */

