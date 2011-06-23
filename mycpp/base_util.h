#ifndef __MYCPP__BASE_UTIL_H_
#define __MYCPP__BASE_UTIL_H_

#include <cstring>

#include <mycpp/types.h>

/* Do not include this header file directly from external programs
 * and libraries. Include <mycpp/util.h> instead.
 *
 * This header resolves circular dependencies in some MyCpp header files. */

namespace MyCpp {

/* Introduced to make 'abortIfReached()' macro look like it has
 * a default argument value. */
static inline const char* _stringItself (const char *str = NULL)
{
    return str;
}

/* Macros do not obey namespaces, unfortunately... */
#define abortIfReached_str(str)					\
	    (MyCpp::_abortIfReached (__FILE__,			\
				     (unsigned long) __LINE__,	\
				     MyCpp::_stringItself (str)))

#define abortIfReached() (abortIfReached_str (""))

#define abortIf(cond)						\
	    do {						\
		if (cond)					\
		    abortIfReached ();				\
	    } while (0)

void _abortIfReached (const char    *file,
		      unsigned long  line,
		      const char    *str = NULL);

#define oopsMessage(str)					\
	    (MyCpp::_oopsMessage (__FILE__,			\
				  (unsigned long) __LINE__,	\
				  MyCpp::_stringItself (str)))

void _oopsMessage (const char    *file,
		   unsigned long  line,
		   const char    *str = NULL);

/* 07.05.26
 * I introduced libraryLock and libraryUnlock to wrap
 * non-reentrant getservbyname library call.
 * It looks like Linux does not support reentrant
 * getservbyname_r call. */

/* 08.10.28
 * One should use libraryLock() in every case of using a single-threaded
 * library. Remember that such libraries could call non-reentrant glibc
 * functions, which means that they should all be synchronized with each
 * other. libraryLock/Unlock() is the synchronization mechanism to use
 * in case of using MyCpp. */

/*m Aquires the lock that is used to protect non-reentrant library calls
 * (mainly for glibc calls). */
void libraryLock ();

/*m Releases the lock that is used to protect non-reentrant library calls
 * (mainly for glibc calls). */
void libraryUnlock ();

/* 08.01.07 Introduced helperLock() to provide a method for synchronizing
 * on-demand initialization of static data. The problem was that one can't
 * have a static Mutex object which would not require explicit
 * initialization. */
void helperLock ();
void helperUnlock ();

bool gthreadInitialized ();

// We want to disallow the use of sumOverflow() for all but a few types.
// Templates allow us to do that.
template <class T>
inline bool
sumOverflow (T left,
	     T right)
{
    abortIfReached ();
    return true;
}

template <>
inline bool
sumOverflow<Size> (Size left,
		   Size right)
{
    return left + right < left;
}

template <class T>
inline bool
checkBoundaries (T start_offset,
		 T len,
		 T size)
{
    abortIfReached ();
    return false;
}

// @start_offset - offset to the subregion
// @len          - length of the subregion
// @size         - size of the outer memory region
template <>
inline bool
checkBoundaries<Size> (Size start_offset,
		       Size len,
		       Size size)
{
    // 'start_offset' is allowed to point to the first byte
    // past the end of the buffer, provided that 'len' is 0.
    if (start_offset > size             ||
	sumOverflow (start_offset, len) ||
	start_offset + len > size)
    {
	return false;
    }

    return true;
}

template <class T>
inline void
assertBoundaries (T start_offset,
		  T len,
		  T size)
{
    abortIfReached ();
}

template <>
inline void
assertBoundaries<Size> (Size start_offset,
			Size len,
			Size size)
{
    abortIf (!checkBoundaries (start_offset, len, size));
}

// TODO Size countStrLength (const char *str);
static inline unsigned long
countStrLength (const char *str)
{
    assert (str);
    return strlen (str);
}

}

#endif /* __MYCPP__BASE_UTIL_H_ */

