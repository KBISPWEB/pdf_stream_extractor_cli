#include "filext.h"

#include <stdio.h>

extern filext_table_t filext_records;

/* HEY! SEGMENTATION FAULT IS TOTALLY ABLE TO HAPPEN HERE!
 * DON'T USE THIS WITHOUT MAKING SURE THE LENGTHS OF THESE ARRAYS ARE THE SAME,
 * NOT INCLUDING NULL BYTE!
 */
int filext_check_data_against_signature(uint8_t *data, filext_record_t *record)
{
	size_t i;
	uint16_t byte;

	for (i = 0; i < record->signature.size; i++) {
		if ((*record->signature.data)[i] == EXT_BYTE_ANY)
			continue;

		if (data[i] != (uint8_t)(*(record->signature.data))[i])
			return 0;
	}

	return 1;
}

char *filext_get_filext(filext_sample_t *sample)
{
	static size_t i;
	static size_t oi;
	static filext_sample_t *__sample;

	filext_record_t *record;

	size_t offset;

	if (sample != NULL) {
		__sample = sample;
		i = 0;
		oi = 0;
	}

	for (; i < filext_records.length; i++) {
		record = &((*filext_records.records)[i]);

		for (; oi < record->offsets.size; oi++) {
			offset = (*record->offsets.data)[oi];

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

			if (filext_check_data_against_signature(
				    __sample->sample + offset, record)) {
				oi++;
				return record->extension;
			}
		}
		oi = 0;
	}

	return NULL;
}

size_t filext_max_sample_size()
{
	size_t i = 0, oi = 0, max = 0, len = 0;

	filext_record_t *record;

	for (; i < filext_records.length; i++) {
		record = &((*filext_records.records)[i]);

		for (; oi < record->offsets.size; oi++) {
			len = record->signature.size +
			      (*record->offsets.data)[oi];
			if (len > max)
				max = len;
		}
	}

	return max;
}
