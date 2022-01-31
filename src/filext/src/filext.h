#include <config.h>

#include <inttypes.h>
#include <stddef.h>

#ifndef FILEXT_H
#define FILEXT_H

typedef uint16_t filext_signature_t;
typedef size_t filext_offset_t;

typedef struct filext_record {
	struct {
		size_t size;
		filext_signature_t (*data)[];
	} signature;
	struct {
		size_t size;
		filext_offset_t (*data)[];
	} offsets;
	char *extension;
} filext_record_t;

typedef struct filext_records {
	size_t length;
	filext_record_t (*records)[];
} filext_table_t;

typedef struct filext_sample {
	size_t length;
	uint8_t *sample;
} filext_sample_t;

#define EXT_BYTE_ANY (uint16_t)(-1)

#define FILEXT_SIGNATURE(...)                                                  \
	{                                                                      \
		__VA_ARGS__                                                    \
	}

#define FILEXT_OFFSETS(...)                                                    \
	{                                                                      \
		__VA_ARGS__                                                    \
	}

#define FILEXT_TABLE(...)                                                      \
	(filext_table_t)                                                       \
	{                                                                      \
		.length = (sizeof((filext_record_t[]){ __VA_ARGS__ }) /        \
			   sizeof(filext_record_t)),                           \
		.records = &((filext_record_t[]){ __VA_ARGS__ })               \
	}

#define FILEXT_RECORD(__SIG__, __OFF__, __EXT__)                                \
	{                                                                       \
		.signature = { .size = (sizeof((filext_signature_t[])__SIG__) / \
					sizeof(filext_signature_t)),            \
			       .data = &((filext_signature_t[])__SIG__) },      \
		.offsets = { .size = (sizeof((filext_offset_t[])__OFF__) /      \
				      sizeof(filext_offset_t)),                 \
			     .data = &((filext_offset_t[])__OFF__) },           \
		.extension = (__EXT__)                                          \
	}

char *filext_get_filext(filext_sample_t *sample);
size_t filext_max_sample_size();

#endif
