#include <Windows.h>

#include <mycpp/win32.h>
#include <mycpp/native_file.h>
#include <mycpp/io.h>

namespace MyCpp {

void
win32_myCppInit ()
{
    stdoutFile = grab (new NativeFile (GetStdHandle (STD_OUTPUT_HANDLE), false /* should_close */));
    stderrFile = grab (new NativeFile (GetStdHandle (STD_ERROR_HANDLE), false /* should_close */));

    /* Prevent std*File objects from being destroyed
     * on return from main() or after a call to exit().
     * (There is no well-defined point in doing this,
     * it's just for some extra safety). */
    stdoutFile->ref ();
    stderrFile->ref ();

    outf = stdoutFile;
    errf = stderrFile;

    failmodeOutf = outf;
    failmodeErrf = errf;
}

}

