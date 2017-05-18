/* NAME: SHAAN MATHUR
   EMAIL: SHAANKARANMATHUR@GMAIL.COM
   UID: 904606576
*/

#include "hash.h"

// Implementation of the well known and used djb2 algorithm, with additional absolute value
// Source: http://www.cse.yorku.ca/~oz/hash.html
long long hash(unsigned char *str){
  long long hash = 5381;
  int c;
  
  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  long long mask = ((long long)(hash)) >> (sizeof(long long) * 8 - 1);
  return (mask ^ hash) - mask;
}
