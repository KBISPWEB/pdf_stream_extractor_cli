#ifndef EXT_H
#define EXT_H

#include <inttypes.h>
#include <stddef.h>

typedef struct ext_signature {
	size_t length;
	struct {
		size_t length;
		size_t *e;
	} offsets;
	struct {
		size_t length;
		char **e;
	} extensions;
	uint16_t *signature;
} ext_signature_t;

#define _SIGNATURE_A(X) (uint16_t[])(X)
#define _OFFSET_A(X) (size_t[])(X)
#define _EXTENSION_A(X) (char *[])(X)

typedef struct ext_records {
	size_t length;
	ext_signature_t e[];
} ext_records_t;

typedef struct ext_result {
	size_t i;
	size_t offs_i;
	size_t exts_i;
} ext_result_t;

#define EXT_BYTE_ANY (uint16_t)(-1)

char *ext_guess_extension(ext_result_t *result, uint8_t *data, size_t length);
size_t ext_sample_size();

#endif
