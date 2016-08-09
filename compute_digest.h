#ifndef _COMPUTE_DIGEST_H
#define _COMPUTE_DIGEST_H
#include <mhash.h>

#define DIGESTTYPE MHASH_SHA256
#define DIGESTLEN (mhash_get_block_size(DIGESTTYPE))
#define DIGESTSTRLEN (2*DIGESTLEN)

/* compute the hash digest.
	 store the result (hex string) in hashstr: an array of at least DIGESTSTRLEN chars
	 return the size of the file in case of success, -1 in case of error */
ssize_t compute_digest(const char *path, char *hashstr);

/* copytemp_hash copies the file whose path is 'inpath' in a temporary file whose path is
	 created by mkstemp using 'outtemplate' */
/* During the copy, copytemp_hash computes the hash and stores the (hex) result in hashstr */
ssize_t copytemp_digest(const char *inpath, char *outtemplate, char *hashstr); 

#endif /* _COMPUTE_DIGEST_H */

