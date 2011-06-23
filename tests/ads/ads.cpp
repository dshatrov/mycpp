#include <mycpp/file_data_source.h>
#include <mycpp/native_file.h>
#include <mycpp/poll_group.h>
#include <mycpp/io.h>

using namespace MyCpp;

static void
adsSimpleRead (File *file)
{
    char buf [1024];
    unsigned long bread = 0,
		  last_read;
    while (bread < sizeof buf - 1) {
	last_read = file->read (buf + bread, sizeof buf - 1 - bread);
	if (last_read == 0)
	    break;

	bread += last_read;
    }
    buf [bread] = 0;

    errf->print ("adsSimpleRead: string: ").
	  print (buf).pendl ();
}

struct AdsPrqData {
    char          *buf;
    unsigned long  bufLen;
    PollGroup     *pollGroup;
    bool           done;
};

static char*
adsGetPage (unsigned long long pos,
	    unsigned long *len,
	    void **pageData,
	    void  *userData)
{
    AdsPrqData *prqData = (AdsPrqData*) userData;

    errf->print ("adsGetPage").pendl ();

    *len = prqData->bufLen;
    return prqData->buf;
}

static void
adsPage (char           *page,
	 unsigned long long pos,
	 unsigned long   len,
	 void           *pageData,
	 void           *userData)
{
    AdsPrqData *prqData = (AdsPrqData*) userData;

    errf->print ("adsPage: "
		 "pos = ").
	  print (pos).
	  print (", len = ").
	  print (len).
	  pendl ();
}

static void
adsRequestComplete (bool  aborted,
		    bool  eof,
		    Ref<Exception> exc,
		    void *userData)
{
    AdsPrqData *prqData = (AdsPrqData*) userData;

    errf->print ("adsRequestComplete").pendl ();

    if (aborted)
	errf->print ("adsRequestComplete: aborted").pendl ();

    if (eof)
	errf->print ("adsRequestComplete: eof").pendl ();

    if (!exc.isNull ())
	errf->print ("adsRequestComplete: exception: ").
	      print ((const char*) *exc->getMessage ()).
	      pendl ();

    prqData->done = true;
    prqData->pollGroup->trigger ();

    errf->print ("adsRequestComplete: done").pendl ();
}

static void
ads_ADS_read (NativeFile *file)
{
    errf->print ("ads_ADS_read").pendl ();

    Ref<FileDataSource> fds = grab (new FileDataSource (file));

    Ref<PollGroup> pollGroup = grab (new PollGroup ());
 //   pollGroup->addPollable (file);

    char buf [1024];

    AdsPrqData prqData;
    prqData.buf = buf;
    prqData.bufLen = sizeof buf;
    prqData.pollGroup = pollGroup;
    prqData.done = false;

    if (fds->requestPages (0,			// pos
			   sizeof buf,		// len
			   sizeof buf,		// pageSize
			   adsPage,		// pageCallback
			   adsGetPage,		// getPageCallback
			   NULL,		// putPageCallback
			   adsRequestComplete,	// requestCompleteCallback
			   &prqData,		// callbackData
			   NULL,		// refCallback
			   NULL,		// unrefCallback
			   NULL,		// refCallback
			   NULL).isNull ())	// sbnKey
    {
	errf->print ("ads_ADS_read: request was not accepted").pendl ();
	return;
    }

    for (;;) {
//	errf->print ("ads_ADS_read: loop iteration").pendl ();
//	pollGroup->iteration ();
	file->processInput ();

	if (prqData.done)
	    break;
    }

    errf->print ("ads_ADS_read: done").pendl ();
}

int main (void)
{
    myCppInit ();

    try {
	Ref<NativeFile> file = grab (new NativeFile ("test.txt"));

//	file->setAsync (false);
//	errf->print ("main: performing adsSimpleRead ()).pendl ();
//	adsSimpleRead (file);

	file->setAsync (true);
	errf->print ("main: performing ads_ADS_read ()").pendl ();
	ads_ADS_read (file);
    } catch (Exception &exc) {
	errf->print ("main: exception: ").
	      print ((const char*) *exc.getMessage ()).
	      pendl ();
    }

    return 0;
}

