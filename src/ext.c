#include "ext.h"

#include <stdio.h>

extern ext_records_t ext_records;

/* HEY! SEGMENTATION FAULT IS TOTALLY ABLE TO HAPPEN HERE!
 * DON'T USE THIS WITHOUT MAKING SURE THE LENGTHS OF THESE ARRAYS ARE THE SAME,
 * NOT INCLUDING NULL BYTE!
 */
int ext_check_data_against_signature(uint8_t *data, ext_signature_t *signature)
{
	size_t i;
	uint16_t byte;

	for (i = 0; i < signature->length; i++) {
		if (signature->signature[i] == EXT_BYTE_ANY)
			continue;
		if (data[i] != (char)(signature->signature[i]))
			return -1;
	}

	return 0;
}

char *ext_guess_extension(ext_result_t *result, ext_sample_t *sample)
{
	size_t i = 0, oi = 0, ei = 0;

	static ext_sample_t *__sample;

	ext_signature_t *signature;

	size_t offset;

	if (sample == NULL) {
		i = result->i;
		oi = result->offs_i;
		ei = result->exts_i;
	} else {
		__sample = sample;
	}

	for (; i < ext_records.length; i++) {
		signature = &(ext_records.e[i]);

		for (; oi < signature->offsets.length; oi++) {
			offset = signature->offsets.e[oi];

			/* if offset is bigger than sample length,
			 * skip signature check
			 */
			if (offset >= __sample->length)
				continue;

			/* if length of signature is bigger than
			 * the sample, skip signature check
			 */
			if ((signature->length + offset) > __sample->length)
				continue;

			if (ext_check_data_against_signature(
				    __sample->sample + offset, signature)) {
				/* signature checks out;
				 * select next extension and return
				 */
				if (ei < signature->extensions.length) {
					/* preserve current indexes */
					result->i = i;
					result->offs_i = oi;
					result->exts_i = ei + 1;
					return signature->extensions.e[ei];
				}
				ei = 0;
			}
		}
		oi = 0;
	}

	return NULL;
}

size_t ext_max_sample_size()
{
	size_t i = 0, oi = 0, max = 0, len = 0;

	ext_signature_t *signature;

	for (; i < ext_records.length; i++) {
		signature = &(ext_records.e[i]);

		for (; oi < signature->offsets.length; oi++) {
			len = signature->length + signature->offsets.e[oi];
			if (len > max)
				max = len;
		}
	}

	return max;
}
