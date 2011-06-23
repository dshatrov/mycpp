#ifndef __MYCPP__CMDLINE__H__
#define __MYCPP__CMDLINE__H__


#include <mycpp/iterator.h>


namespace MyCpp {

typedef bool (*ParseCmdlineCallback) (const char *short_name,
				      const char *long_name,
				      const char *value,
				      void       *opt_data,
				      void       *callback_data);

struct CmdlineOption
{
    const char *short_name,
	       *long_name;

    bool  with_value;
    void *opt_data;

    ParseCmdlineCallback opt_callback;
};

void parseCmdline (int    *argc,
		   char ***argv,
		   Iterator<CmdlineOption&> &opt_iter,
		   ParseCmdlineCallback      callback,
		   void                     *callbackData);

}

#endif /* __MYCPP__CMDLINE__H__ */

