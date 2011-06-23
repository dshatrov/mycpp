#include <unistd.h>

#include <mycpp/posix.h>
#include <mycpp/io.h>
#include <mycpp/native_file.h>
#include <mycpp/cached_file.h>

namespace MyCpp {

void
posix_myCppInit ()
{
    Ref<NativeFile> stdout_nf = grab (new NativeFile (STDOUT_FILENO, false /* should_close */));
    Ref<NativeFile> stderr_nf = grab (new NativeFile (STDERR_FILENO, false /* should_close */));

    stdoutFile = grab (new CachedFile (stdout_nf, 4096 /* page_size */, 1 /* max_pages */));
    stderrFile = grab (new CachedFile (stderr_nf, 4096 /* page_size */, 1 /* max_pages */));
//    stdoutFile = stdout_nf;
//    stderrFile = stderr_nf;

    /* Prevent std*File objects from being destroyed
     * on return from main() or after a call to exit().
     * (There is no well-defined point in doing this,
     * it's just for some extra safety). */
    stdoutFile->ref ();
    stderrFile->ref ();

    outf = stdoutFile;
    errf = stderrFile;

    failmodeOutf = stdout_nf;
    failmodeErrf = stderr_nf;
}

}

