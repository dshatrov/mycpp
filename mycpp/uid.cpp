#include <mycpp/uid.h>

namespace MyCpp {

namespace {
    UidMapper mycpp_glob_uid_mapper;
}

UidMapper&
UidMapper::getGlobUidMapper ()
{
    return mycpp_glob_uid_mapper;
}

}

