#ifndef __MYCPP__CLONEABLE_H__
#define __MYCPP__CLONEABLE_H__

#include <mycpp/simply_referenced.h>

namespace MyCpp {

template <class T>
class Cloneable
{
public:
    virtual T* clone () const = 0;
};

}

#endif /* __MYCPP__CLONEABLE_H__ */

