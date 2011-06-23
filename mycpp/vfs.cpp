#include <mycpp/util.h>
#include <mycpp/vfs.h>
#include <mycpp/vfs_posix.h>

namespace MyCpp {

Ref<Vfs>
Vfs::defaultLocalVfs (const char *root_path)
{
#ifndef PLATFORM_WIN32
    return grab (static_cast <Vfs*> (new VfsPosix (root_path)));
#else
    // TODO
    abortIfReached ();
    return NULL;
#endif
}

}

