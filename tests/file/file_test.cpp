#include <cstdlib>
#include <mycpp/mycpp.h>
#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/buffer.h>
#include <mycpp/native_file.h>
#include <mycpp/cached_file.h>

using namespace MyCpp;

int main (void)
try {
    myCppInit ();

    Ref<NativeFile> file = grab (new NativeFile ("test.txt",
						 OpenFlags::Create | OpenFlags::Truncate,
						 AccessMode::ReadWrite));

    Ref<CachedFile> cached = grab (new CachedFile (file, 256, 256));

    cached->out ("Hello, World!").oendl ().oendl ();

    Ref<Buffer> buf = grab (new Buffer (1024));
    cached->seek (0, SeekOrigin::SeekBeg);

    Size nread;
    IOResult res = cached->read (buf->getMemoryDesc (0, buf->getSize ()), &nread);
    abortIf (res != IOResultNormal);
    abortIf (nread > buf->getSize ());

    errf->print ("read: ").print (buf->getMemoryDesc (0, nread)).pendl ();

    cached->seek (0, SeekOrigin::SeekBeg);

    Size nwritten;
    cached->write (buf->getMemoryDesc (0, nread), &nwritten);
    abortIf (nwritten != nread);
/*    cached->write (buf->getMemoryDesc (0, nread), &nwritten);
    abortIf (nwritten != nread);
*/
    cached->close (true /* flush_data */);

    return 0;
} catch (Exception &exc) {
    printException (errf, exc);
    return EXIT_FAILURE;
}

