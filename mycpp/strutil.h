#ifndef __MYCPP__STRUTIL_H__
#define __MYCPP__STRUTIL_H__

// TODO Move this to mem_util.h

#include <mycpp/memory_desc.h>

namespace MyCpp {

bool compareStrings (const char *str1,
		     const char *str2);

ComparisonResult orderStrings (const char *str1,
			       const char *str2);

ComparisonResult compareByteArrays (ConstMemoryDesc const &buf1,
				    ConstMemoryDesc const &buf2);

ComparisonResult compareByteArrayToString (ConstMemoryDesc const &buf,
					   const char *str);

bool stringHasSuffix (ConstMemoryDesc  str,
		      ConstMemoryDesc  suffix,
		      ConstMemoryDesc *ret_str);

bool stringHasSuffix (const char *str,
		      const char *suffix,
		      ConstMemoryDesc *ret_str);

/* DEPRECATED
 *

ComparisonResult compareByteArrays (const unsigned char *str1,
				    unsigned long        str1_len,
				    const unsigned char *str2,
				    unsigned long        str2_len);

ComparisonResult compareByteArrayToString (const unsigned char *str1,
					   unsigned long        str1_len,
					   const char          *str2);
*/

/* Simplified strtoul introduced to parse myrelay messages
 * in a consistent way. */
unsigned long strToUlong (const char *str);

long strToLong (const char *str);

double strToDouble (const char *str);

bool ullongToHexStr (unsigned long long  ull,
		     char          *str,
		     unsigned long  len,
		     unsigned long *offs);

/* If str is NULL, then len has no effect, and the function
 * is guaranteed to return true. */
bool ullongToStr (unsigned long long ull,
		  char          *str,
		  unsigned long  len,
		  unsigned long *offs);

bool ulongToStr (unsigned long  ul,
		 char          *str,
		 unsigned long  len,
		 unsigned long *offs);

bool llongToStr (long long      ll,
		 char          *str,
		 unsigned long  len,
		 unsigned long *offs);

bool longToStr (long           l,
		char          *str,
		unsigned long  len,
		unsigned long *offs);

bool doubleToStr (double         dbl,
		  char          *str,
		  unsigned long  len,
		  unsigned long *offs,
		  unsigned long  prec);

#if 0
Banning printf* functions

unsigned long snprintf (char          *buf,
			unsigned long  len,
			const char    *format,
			...);

unsigned long vasnprintf (char          *buf,
			  unsigned long  len,
			  const char    *format,
			  va_list        ap);

// Создаёт новую строку по формату, "на лету" определяя нужный объём памяти
// После использования строку нужно освободить (delete)
// FIXME Нужно ввести метод принудительного освобождения специально для
// таких строк, чтобы не зависеть то перегружаемого оператора delete
char* message (const char *fmt, ...);
char* vmessage (const char *fmt, va_list ap);

// Опрелеление длины будущей строки
// -1, если строка слишком длинная (см. переполнение в коде vformatLenght)
int formatLength (const char *fmt, ...);
int vformatLength (const char *fmt, va_list ap);
#endif

}

#endif /* __MYCPP__STRUTIL_H__ */

