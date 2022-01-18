#include "ext.h"

#include <stdio.h>

extern ext_table_t ext_records;

/* HEY! SEGMENTATION FAULT IS TOTALLY ABLE TO HAPPEN HERE!
 * DON'T USE THIS WITHOUT MAKING SURE THE LENGTHS OF THESE ARRAYS ARE THE SAME,
 * NOT INCLUDING NULL BYTE!
 */
int ext_check_data_against_signature(uint8_t *data, ext_record_t *record)
{
	size_t i;
	uint16_t byte;

	for (i = 0; i < record->signature.size; i++) {
		if (record->signature.data[i] == EXT_BYTE_ANY)
			continue;
		if (data[i] != (char)(record->signature.data[i]))
			return -1;
	}

	return 0;
}

char *ext_get_extension(ext_sample_t *sample)
{
	static size_t i;
	static size_t oi;
	static ext_sample_t *__sample;

	ext_record_t *record;

	size_t offset;

	if (sample != NULL) {
		__sample = sample;
		i = 0;
		oi = 0;
	}

	for (; i < ext_records.length; i++) {
		record = &(ext_records.records[i]);

		for (; oi < record->offsets.size; oi++) {
			offset = record->offsets.data[oi];

			/* if offset is bigger than sample length,
			 * skip signature check
			 */
			if (offset >= __sample->length)
				continue;

			/* if length of signature is bigger than
			 * the sample, skip signature check
			 */
			if ((record->signature.size + offset) >
			    __sample->length)
				continue;

			if (ext_check_data_against_signature(
				    __sample->sample + offset, record)) {
				oi++;
				return record->extension;
			}
		}
		oi = 0;
	}

	return NULL;
}

size_t ext_max_sample_size()
{
	size_t i = 0, oi = 0, max = 0, len = 0;

	ext_record_t *record;

	for (; i < ext_records.length; i++) {
		record = &(ext_records.records[i]);

		for (; oi < record->offsets.size; oi++) {
			len = record->signature.size + record->offsets.data[oi];
			if (len > max)
				max = len;
		}
	}

	return max;
}
