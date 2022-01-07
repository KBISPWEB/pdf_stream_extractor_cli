#ifndef EXT_H
#define EXT_H

#include <inttypes.h>
#include <stddef.h>

typedef struct ext_extensions {
	size_t length;
	char **ext;
} ext_extensions_t;

typedef struct ext_signature {
	size_t length;
	struct {
		size_t length;
		size_t *e;
	} offsets;
	uint16_t *signature;
	ext_extensions_t extensions;
} ext_signature_t;

#define _SIGNATURE_A uint16_t[]
#define _OFFSET_A size_t[]
#define _EXTENSION_A char *[]

typedef struct ext_records {
	size_t length;
	ext_signature_t signatures[];
} ext_records_t;

typedef struct ext_sample {
	size_t length;
	uint8_t *sample;
} ext_sample_t;

#define EXT_BYTE_ANY (uint16_t)(-1)

ext_extensions_t *ext_get_extensions(ext_sample_t *sample);
size_t ext_max_sample_size();

#endif
