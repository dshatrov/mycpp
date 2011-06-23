#ifndef __MYCPP__ACTION_BOSS_H__
#define __MYCPP__ACTION_BOSS_H__

#include <mycpp/object.h>
#include <mycpp/informer.h>

namespace MyCpp {

class ActionBoss : public virtual Object
{
public:
    typedef void (*MayDoCallback) (void *callback_data);

protected:
    Ref< Informer<MayDoCallback> > mayDoInformer;

    void fireMayDo ()
    {
	mayDoInformer->informAll (this);
    }

    static void mayDoInform (MayDoCallback  callback,
			     void          *callbackData,
			     void          * /* _self */)
    {
	callback (callbackData);
    }

public:
    virtual bool mayDo () = 0;

    virtual bool mayDo_subscribe (CallbackDesc<MayDoCallback> const &cb,
				  unsigned long flags,
				  Ref<GenericInformer::Subscription> *ret_sbn) = 0;

    Ref< Informer<MayDoCallback> > getMayDoInformer ()
    {
	return mayDoInformer;
    }

    ActionBoss ()
	: mayDoInformer (grab (new Informer<MayDoCallback> (mayDoInform)))
    {
    }
};

}

#endif /* __MYCPP__ACTION_BOSS_H__ */

