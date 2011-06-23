#ifndef __MYCPP_IO_ACTOR_H__
#define __MYCPP_IO_ACTOR_H__

#include <mycpp/object.h>
#include <mycpp/io_subject.h>
#include <mycpp/informer.h>

namespace MyCpp {

/*c An object which is pollable by a file descriptor.
 *
 * Pollable objects can be polled using a <t>PollGroup</t>. */
class IoActor : public virtual IoSubject,
		public virtual Object
{
public:
    typedef void (*NeedCallback) (bool  need,
				  void *callbackData);

    /*e Poll event types. */
    enum Event {
	EventRead  = 1, //< Poll for read events.
	EventWrite = 2, //< Poll for write events.
	EventError = 4  //< Poll for error events.
    };

    enum {
	AllEvents  = 0x7
    };

    /*t*/
    typedef unsigned long Events;

protected:
    Ref< Informer<NeedCallback> > needInputInformer,
				  needOutputInformer;

    struct NeedInformData {
	IoActor *self; // TODO This field is now unnecessary
	bool     need;
    };

    static void needInform (NeedCallback  callback,
			    void         *callbackData,
			    void         *_nidata);

    /* Convinience methods. */
    void fireNeedInput  (bool need);
    void fireNeedOutput (bool need);

public:
    /*m Input callback.
     *
     * Should be called when there is input available for
     * the polling file descriptor. */
    virtual void processInput  () = 0;

    /*m Output callback.
     *
     * Should be called when the polling file descriptor
     * is ready for output. */
    virtual void processOutput () = 0;

    /*m Error callback.
     *
     * Should be called when an error occurs for
     * the polling file descriptor. */
    virtual void processError  () = 0;

  /* Non-virtual methods */

  /* IoSubject interface */

    void setNeedInput (bool need)
    {
	fireNeedInput (need);
    }

    void setNeedOutput (bool need)
    {
	fireNeedOutput (need);
    }

  /* (End of IoSubject interface) */

    /*m*/
    void processEvents (Events events);

    Ref< Informer<NeedCallback> > getNeedInputInformer ()
    {
	return needInputInformer;
    }

    Ref< Informer<NeedCallback> > getNeedOutputInformer ()
    {
	return needOutputInformer;
    }

    IoActor ();
};

class IoActor_Closing : public IoActor,
			public virtual Object
{
public:
    typedef void (*PreClosedCallback) (void *data);

private:
  // mt_const

    Ref< Informer<PreClosedCallback> > pre_closed_informer;

  // (end mt_const)

  // mt_mutex state_mutex

    Bool pre_closed;

  // (end mt_mutex state_mutex)

    Mutex state_mutex;

    static void preClosedInform (PreClosedCallback callback,
				 void *callback_data,
				 void *data);

    void firePreClosed ()
    {
	pre_closed_informer->informAll (NULL);
    }

protected:
    // TODO doPreClose() is subject to race conditions.
    // A state_mutex should be locked for a pollable (i.e. TcpConnection)
    // in its destructor, but that's not done in current implementations.
    //
    // Pollables are Objects anyway, so DeletionCallbacks should be used
    // instead, with proper protection from spurious events in the pollable
    // itself.
    //
    // Note: doPreClose() is necessary to ensure proper removal of the Pollable
    // from a PollGroup. It is presumed that fd (from getFd()) is necessary
    // to properly remove a Pollable, and that the fd is not available after
    // a call to doPreClose() is made.
    void ioActor_doPreClose ();

public:
    bool isPreClosed_subscribe (CallbackDesc<PreClosedCallback> const *cb,
				unsigned long flags,
				Ref<GenericInformer::Subscription> *ret_sbn);

    Ref< Informer<PreClosedCallback> > getPreClosedInformer ()
    {
	return pre_closed_informer;
    }

    IoActor_Closing ();

    ~IoActor_Closing ();
};

}

#endif /* __MYCPP_IO_ACTOR_H__ */

