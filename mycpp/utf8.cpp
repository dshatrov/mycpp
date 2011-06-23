#include <glib.h>

#include <mycpp/utf8.h>
#include <mycpp/util.h>

namespace MyCpp {

bool
utf8_validate (const char    *str,
	       unsigned long  len,
	       unsigned long *valid_len)
{
#if 0
G_MAXSSIZE is available since glib-2.14, not present in 2.12
    if (len > G_MAXSSIZE) {
	/* TODO Shouldn't fail here.
	 * Break the call up to several g_utf8_validate() calls. */
	abortIfReached ();
    }
#endif

    const gchar *end;
    gboolean valid = g_utf8_validate (str, len, &end);

    if (valid_len != NULL)
	*valid_len = end - str;

    return valid;
}

bool
utf8_validate_sz (const char    *str,
		  unsigned long *valid_len)
{
    const gchar *end;
    gboolean valid = g_utf8_validate (str, -1, &end);

    if (valid_len != NULL)
	*valid_len = end - str;

    return valid;
}

const char*
utf8_valid_next (const char *cbuf)
{
    return g_utf8_next_char (cbuf);
}

unsigned long
utf8_valid_char_len (const char *uchar)
{
    const char *next = utf8_valid_next (uchar);
    return next - uchar;
}

Unichar
utf8_valid_to_unichar (const char *cbuf)
{
    return g_utf8_get_char (cbuf);
}

Ref<String>
ucs4_to_utf8 (const Unichar *str,
	      unsigned long  len)
{
    Ref<String> ret_str;

    /* TODO Blind conversion from 'unsigned long' to 'glong'. */
    gchar *_str = g_ucs4_to_utf8 (str, (glong) len, NULL, NULL, NULL);
    if (_str == NULL)
	return NULL;

    ret_str = grab (new String (_str));

    g_free (_str);

    return ret_str;
}

Size
unichar_to_utf8 (Unichar uc,
		 Byte *ret_str)
{
    return (Size) g_unichar_to_utf8 (uc, (gchar*) ret_str);
}


}

