#include "file_ext.h"
/*
typedef struct ext_result {
	size_t i;
	size_t offsets_i;
	size_t signatures_i;
	size_t extensions_i;
} ext_result_t;
*/
extern ext_record_t file_ext_records[];

/* HEY! SEGMENTATION FAULT IS TOTALLY ABLE TO HAPPEN HERE!
 * DON'T USE THIS WITHOUT MAKING SURE THE LENGTHS OF THESE ARRAYS ARE THE SAME,
 * NOT INCLUDING NULL BYTE!
 */
size_t get_signature_length(uint16_t *signature)
{
	size_t i = 0;

	if (!signature)
		return 0;

	while ((byte = signature[i++]) != EXT_BYTE_STOP)
		;

	return --i;
}

int check_data_against_signature(uint8_t *data, uint16_t *signature)
{
	size_t i = 0;
	uint16_t byte;

	if ((!data) || (!signature))
		return -1;

	while ((byte = signature[i++]) != EXT_BYTE_STOP) {
		if (byte == EXT_BYTE_ANY)
			continue;
		if (data[i] != (char)byte)
			return -1;
	}

	return 0;
}

ext_result_t *ext_guess_extension(ext_result_t *result, uint8_t *data,
				  size_t length)
{
	ext_record_t *recordp;
	size_t offset;
	uint16_t *signature;
	char *extension;

	/* *recordp == 0 literally means .offsets is NULL */
	while (*(recordp = &(file_ext_records[result->i++]))) {
		while ((offset = recordp->offsets[result->offsets_i++]) !=
		       EXT_OFFSET_STOP) {
			if (offset < length) {
				while ((signature =
						recordp->signatures
							[result->signatures_i++])) {
					/* make sure we have enough space */
					if ((get_signature_length + offset) <
					    length) {
						/* if signature at offset matches, loop through
						 * available extensions!
				 	 	*/
						if ((check_data_against_signature(
							    &(data[offset]),
							    signature)) &&
						    (extension =
							     recordp->extensions
								     [result->extensions_i++])) {
							/* We like this option. return OK */
							/* preserve current indexes */
							result->i--;
							result->offsets_i--;
							result->signatures_i--;
							return result;
						}
					}
					result->extensions_i = 0;
				}
			}
			result->signatures_i = 0;
		}
		result->offsets_i = 0;
	}

	/* we literally went through everything and there wasn't a good option.
	 * return NOT OK
	 */
	return NULL;
}

char *get_extension_from_result(ext_result_t *result)
{
	if ((!result) || (result->extensions_i == 0))
		return NULL;

	return file_ext_records[result->i].extensions[result->extensions_i - 1];
}
