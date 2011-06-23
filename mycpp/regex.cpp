#include <glib.h>

#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/regex.h>

namespace MyCpp {

Regex::Regex (const char *pattern)
{
    regex = g_regex_new ((const gchar*) pattern,
			 (GRegexCompileFlags) 0,
			 (GRegexMatchFlags) 0,
			 NULL);
    abortIf (regex == NULL);
}

Regex::~Regex ()
{
    g_regex_unref ((GRegex*) regex);
}

Ref<Regex>
regexNew (const char *pattern)
{
    return grab (new Regex (pattern));
}

static void
regex_free_callback (void *mem)
{
    if (mem != NULL)
	g_free (mem);
}

Ref<String>
regexMatch (Regex *regex,
	    ConstMemoryDesc const &mdesc,
	    int match_num)
{
    GMatchInfo *match_info = NULL;

    gboolean match = g_regex_match_full ((GRegex*) regex->regex,
					 (const gchar*) mdesc.getMemory (),
					 // FIXME Overflow check
					 (gssize) mdesc.getLength (),
					 0, // start_position
					 (GRegexMatchFlags) 0,
					 &match_info,
					 NULL);
    if (!match)
	return grab (new String);

    gchar *ret_str = g_match_info_fetch (match_info, match_num);
    abortIf (ret_str == NULL);

    g_match_info_free (match_info);

    return grab (static_cast <String*> (new String_ExtAlloc (ret_str,
							     countStrLength (ret_str),
							     regex_free_callback)));
}

Ref<String>
regexReplace (Regex *regex,
	      ConstMemoryDesc const &mdesc,
	      const char *replacement)
{
    gchar *repl_res = g_regex_replace ((GRegex*) regex->regex,
				       (const gchar*) mdesc.getMemory (),
				       // FIXME Overflow check
				       (gssize) mdesc.getLength (),
				       0, // start_position
				       replacement,
				       (GRegexMatchFlags) 0,
				       NULL);

    if (repl_res == NULL)
	return grab (new String);

    return grab (static_cast <String*> (new String_ExtAlloc (repl_res,
							     countStrLength (repl_res),
							     regex_free_callback)));
}

Ref<String>
regexReplaceLiteral (Regex *regex,
		     ConstMemoryDesc const &mdesc,
		     const char *replacement)
{
    gchar *repl_res = g_regex_replace_literal ((GRegex*) regex->regex,
					       (const gchar*) mdesc.getMemory (),
					       // FIXME Overflow check
					       (gssize) mdesc.getLength (),
					       0, // start_position
					       replacement,
					       (GRegexMatchFlags) 0,
					       NULL);

    if (repl_res == NULL)
	return grab (new String);

    return grab (static_cast <String*> (new String_ExtAlloc (repl_res,
							     countStrLength (repl_res),
							     regex_free_callback)));
}

}

