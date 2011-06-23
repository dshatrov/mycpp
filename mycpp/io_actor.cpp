#include <mycpp/io_actor.h>

namespace MyCpp {

void
IoActor::needInform (NeedCallback  callback,
		     void         *callbackData,
		     void         *_nidata)
{
    NeedInformData *nidata = static_cast <NeedInformData*> (_nidata);
    callback (nidata->need, callbackData);
}

void
IoActor::fireNeedInput (bool need)
{
    NeedInformData nid;

    nid.self = this;
    nid.need = need;

    needInputInformer->informAll (&nid);
}

void
IoActor::fireNeedOutput (bool need)
{
    NeedInformData nid;

    nid.self = this;
    nid.need = need;

    needOutputInformer->informAll (&nid);
}

void
IoActor::processEvents (Events events)
{
    if (events & EventRead)
	processInput ();

    if (events & EventWrite)
	processOutput ();	

    if (events & EventError)
	processError ();
}

IoActor::IoActor ()
{
    needInputInformer  = grab (new Informer<NeedCallback> (needInform));
    needOutputInformer = grab (new Informer<NeedCallback> (needInform));
}

}

