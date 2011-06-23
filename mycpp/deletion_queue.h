#ifndef __MYCPP__DELETION_QUEUE_H__
#define __MYCPP__DELETION_QUEUE_H__

#include <mycpp/simply_referenced.h>

namespace MyCpp {

void deletionQueue_append (SimplyReferenced *obj);

void deletionQueue_process ();

bool deletionQueue_isEmpty ();

void deletionQueue_myCppInit ();

}

#endif /* __MYCPP__DELETION_QUEUE_H__ */

