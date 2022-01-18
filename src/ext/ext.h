#ifndef EXT_H
#define EXT_H

#include <inttypes.h>
#include <stddef.h>

typedef struct ext_record {
	struct {
		size_t size;
		uint16_t *data;
	} signature;
	struct {
		size_t size;
		size_t *data;
	} offsets;
	char *extension;
} ext_record_t;

#define _SIGNATURE_A uint16_t[]
#define _OFFSET_A size_t[]
#define _EXTENSION_A char *[]

typedef struct ext_records {
	size_t length;
	ext_record_t records[];
} ext_table_t;

typedef struct ext_sample {
	size_t length;
	uint8_t *sample;
} ext_sample_t;

#define EXT_BYTE_ANY (uint16_t)(-1)

char *ext_get_extension(ext_sample_t *sample);
size_t ext_max_sample_size();

#endif
