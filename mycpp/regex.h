#ifndef __MYCPP__REGEX_H__
#define __MYCPP__REGEX_H__

#include <mycpp/ref.h>
#include <mycpp/string.h>

namespace MyCpp {

class Regex : public virtual SimplyReferenced
{
public:
    // GRegex*
    void *regex;

    Regex (const char *pattern);

    ~Regex ();
};


Ref<Regex> regexNew (const char *pattern);

Ref<String> regexMatch (Regex *regex,
			ConstMemoryDesc const &mdesc,
			int match_num = 0);

Ref<String> regexReplace (Regex *regex,
			  ConstMemoryDesc const &mdesc,
			  const char *replacement);

Ref<String> regexReplaceLiteral (Regex *regex,
				 ConstMemoryDesc const &mdesc,
				 const char *replacement);

}

#endif /* __MYCPP__REGEX_H__ */

