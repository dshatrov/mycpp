#include <glib.h>

#include <mycpp/mycpp_thread_local.h>

namespace MyCpp {

static GPrivate *tlocal_gprivate = NULL;

MyCpp_ThreadLocal::MyCpp_ThreadLocal ()
{
    data_mutex_counter = 0;
}

MyCpp_ThreadLocal*
myCpp_getThreadLocal ()
{
    MyCpp_ThreadLocal *tlocal = static_cast <MyCpp_ThreadLocal*> (g_private_get (tlocal_gprivate));
    if (tlocal == NULL) {
	tlocal = new MyCpp_ThreadLocal;
	g_private_set (tlocal_gprivate, tlocal);
    }

    return tlocal;
}

static void
tlocal_destructor (gpointer _tlocal)
{
    MyCpp_ThreadLocal * const &tlocal = static_cast <MyCpp_ThreadLocal*> (_tlocal);
    if (tlocal != NULL)
	delete tlocal;
}

void
threadLocal_myCppInit ()
{
    tlocal_gprivate = g_private_new (tlocal_destructor);
}

}

