#ifndef FILE_EXT_H
#define FILE_EXT_H

#include <inttypes.h>
#include <stddef.h>

typedef struct ext_record {
	size_t *offsets;
	uint16_t **signatures;
	char **extensions;
} ext_record_t;

typedef struct ext_result {
	size_t i;
	size_t offsets_i;
	size_t signatures_i;
	size_t extensions_i;
} ext_result_t;

#define EXT_OFFSET_STOP (size_t)(-1)

#define EXT_BYTE_STOP (uint16_t)(-1)
#define EXT_BYTE_ANY (uint16_t)(-2)

char *ext_guess_extension(ext_result_t *result, uint8_t *data, size_t length);

#endif
