#ifndef __MYCPP_UTF8_H__
#define __MYCPP_UTF8_H__

#include <mycpp/unicode.h>
#include <mycpp/string.h>

namespace MyCpp {

bool utf8_validate (const char    *str,
		    unsigned long  len,
		    unsigned long *valid_len);

/* Validates a zero-terminated string. */
bool utf8_validate_sz (const char    *str,
		       unsigned long *valid_len);

const char* utf8_valid_next (const char *str);

unsigned long utf8_valid_char_len (const char *uchar);

Unichar utf8_valid_to_unichar (const char *cbuf);

/* This may return NULL. */
/* TODO I guess that if this was written myself,
 * it would be guaranteed that the conversion succeeds. */
Ref<String> ucs4_to_utf8 (const Unichar *str,
			  unsigned long  len);

// ret_str must be at least 6 bytes long.
// Returns the length of the resulting string.
Size unichar_to_utf8 (Unichar uc,
		      Byte *ret_str);

}

#endif /* __MYCPP_UTF8_H__ */

