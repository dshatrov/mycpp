#include <mycpp/iterator.h>
#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/cmdline.h>

#define DEBUG(a) ;

namespace MyCpp {

void
parseCmdline (int    *argc,
	      char ***argv,
	      Iterator<CmdlineOption&> &opt_iter,
	      ParseCmdlineCallback      callback,
	      void                     *callbackData)
{
    if (argc == NULL ||
	argv == NULL)
    {
	return;
    }

    if (*argc <= 1)
	return;

    unsigned long nopts_consumed = 0;
    unsigned long cur_opt = 1,
		  cur_opt_shifted = 1;
    while (cur_opt < (unsigned long) *argc) {
	DEBUG (
	    errf->print ("MyCpp.parseCmdline: ")
		 .print ("cur_opt: ")
		 .print (cur_opt)
		 .print (", cur_opt_shifted: ")
		 .print (cur_opt_shifted)
		 .pendl ();

	    errf->print ("MyCpp.parseCmdline: option: \"")
		 .print ((*argv) [cur_opt_shifted])
		 .print ("\"")
		 .pendl ();
	)

	bool long_option;
	if ((*argv) [cur_opt_shifted] [0] == '-') {
	    if ((*argv) [cur_opt_shifted] [1] == '-') {
		DEBUG (
		    errf->print ("MyCpp.parseCmdline: long option").pendl ();
		)

		long_option = true;
	    } else {
		DEBUG (
		    errf->print ("MyCpp.parseCmdline: short option").pendl ();
		)

		long_option = false;
	    }
	} else {
	    DEBUG (
		errf->print ("MyCpp.parseCmdline: not an option").pendl ();
	    )

	    cur_opt ++;
	    cur_opt_shifted ++;
	    continue;
	}

	unsigned long argv_shift = 0;

	opt_iter.reset ();
	while (!opt_iter.done ()) {
	    CmdlineOption &opt = opt_iter.next ();

	    bool match = false;
	    if (long_option) {
		if (opt.long_name == NULL)
		    continue;

		if (compareStrings ((*argv) [cur_opt_shifted] + 2,
				    opt.long_name))
		{
		    DEBUG (
			errf->print ("MyCpp.parseCmdline: long option match").pendl ();
		    )

		    match = true;
		}
	    } else {
		if (opt.short_name == NULL)
		    continue;

		if (compareStrings ((*argv) [cur_opt_shifted] + 1,
				    opt.short_name))
		{
		    match = true;
		}
	    }

	    if (!match)
		continue;

	    nopts_consumed ++;

	    argv_shift = 1;

	    const char *value = NULL;
	    if (opt.with_value &&
		cur_opt < (unsigned long) *argc - 1)
	    {
		value = (*argv) [cur_opt_shifted + 1];
		nopts_consumed ++;
		argv_shift ++;
	    }

	    unsigned long i,
			  j = 0;
	    for (i = cur_opt_shifted;
		 i < (unsigned long) *argc - argv_shift;
		 i++)
	    {
		DEBUG (
		    errf->print ("MyCpp.parseCmdline: shifting ")
			 .print ("argv [")
			 .print (cur_opt_shifted + j + argv_shift)
			 .print ("] to argv [")
			 .print (cur_opt_shifted + j)
			 .print ("]")
			 .pendl ();
		)

		(*argv) [cur_opt_shifted + j] =
			(*argv) [cur_opt_shifted + j + argv_shift];
		j ++;
	    }

	    if (opt.opt_callback != NULL) {
		bool cont;
		cont = opt.opt_callback (opt.short_name,
					 opt.long_name,
					 value,
					 opt.opt_data,
					 callbackData);
		if (!cont)
		    goto _stop_parsing;
	    }

	    if (callback != NULL) {
		bool cont;
		cont = callback (opt.short_name,
				 opt.long_name,
				 value,
				 opt.opt_data,
				 callbackData);
		if (!cont)
		    goto _stop_parsing;
	    }

	    break;
	}

	if (argv_shift == 0) {
	    cur_opt ++;
	    cur_opt_shifted ++;
	} else
	    cur_opt += argv_shift;
    }

_stop_parsing:
    *argc -= nopts_consumed;
}

}

