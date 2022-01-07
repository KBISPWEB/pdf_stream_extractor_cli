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

char *ext_guess_extension(ext_result_t *result, uint8_t *data, size_t length)
{
	size_t i = 0, oi = 0, ei = 0;

	ext_signature_t *signature;

	size_t offset;

	if (data == NULL) {
		i = result->i;
		oi = result->offs_i;
		ei = result->exts_i;
	}

	for (; i < ext_records.length; i++) {
		signature = &(ext_records.e[i]);

		for (; oi < signature->offsets.length; oi++) {
			offset = signature->offsets.e[oi];

			/* if offset is bigger than sample length,
			 * skip signature check
			 */
			if (offset >= length)
				continue;

			/* if length of signature is bigger than
			 * the sample, skip signature check
			 */
			if ((signature->length + offset) > length)
				continue;

			if (ext_check_data_against_signature(data + offset,
							     signature)) {
				/* signature checks out;
				 * select next extension and return
				 */
				if (ei < signature->extensions.length) {
					/* preserve current indexes */
					result->i = i;
					result->offs_i = oi;
					result->exts_i = ++ei;
					return signature->extensions.e[ei - 1];
				}
				ei = 0;
			}
		}
		oi = 0;
	}
}

size_t ext_sample_size()
{
}
