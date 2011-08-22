#ifndef __MYCPP__CMDLINE__H__
#define __MYCPP__CMDLINE__H__


#include <mycpp/iterator.h>


namespace MyCpp {

typedef bool (*ParseCmdlineCallback) (const char *short_name,
				      const char *long_name,
				      const char *value,
				      void       *opt_data,
				      void       *callback_data);

class CmdlineOption
{
public:
    const char *short_name,
	       *long_name;

    bool  with_value;
    void *opt_data;

    ParseCmdlineCallback opt_callback;

    CmdlineOption ()
	: short_name   (NULL),
	  long_name    (NULL),
	  with_value   (false),
	  opt_data     (NULL),
	  opt_callback (NULL)
    {
    }
};

void parseCmdline (int    *argc,
		   char ***argv,
		   Iterator<CmdlineOption&> &opt_iter,
		   ParseCmdlineCallback      callback,
		   void                     *callbackData);

}

#endif /* __MYCPP__CMDLINE__H__ */

