#include <stdio.h>
#include "config.h"

#ifdef PSEC_OS_WINDOWS
#include <fileapi.h>
#endif

#ifdef PSEC_OS_LINUX
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <zlib.h>

#include "ext/ext.h"
#include "ioutils/ioutils.h"

ext_records_t ext_records = {
	1,
	{ {
		.length = 7,
		.signature = (_SIGNATURE_A){ 0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66,
					     0xcf },
		.offsets = { 1, (_OFFSET_A){ 0 } },
		.extensions = { 3, (_EXTENSION_A){ "asf", "wma", "wmv" } },
	} }
};

char *get_file_extension(uint8_t *data, size_t size)
{
	ext_extensions_t *extensions;
	ext_sample_t sample = { .length = size, .sample = data };

	extensions = ext_get_extensions(&sample);

	if (extensions != NULL) {
		return extensions->ext[0];
	}

	return "unknown";
}

int main(int argc, char **argv)
{
	int i;
	FILE *fp;

#define BUFFER_LEN 128
	char buffer_in[BUFFER_LEN];
	char buffer_out[BUFFER_LEN];

	z_stream infstream = { 0 }; /* zero-init so pointers are null */

	infstream.next_in = (Bytef *)buffer_in; /* input char buffer */
	infstream.avail_in = (uInt)BUFFER_LEN; /* size of input buffer */
	infstream.next_out = (Bytef *)buffer_out; /* output char array */
	infstream.avail_out = (uInt)BUFFER_LEN; /* size of output */

	if (argc <= 1) {
		fputs("You must specify a PDF file from which to extract stream data.\n",
		      stderr);
		return 1;
	}

	/* Every parameter specifies a file name */
	for (i = 1; i < argc; i++) {
		if (!(fp = fopen(argv[i], "rb"))) {
			fputs(strerror(errno), stderr);
			return 1;
		}

		/* check file signature */
		if (fread(buffer_in, sizeof(char), 10, fp) < 10) {
			fprintf(stderr, "%s is too short to be a PDF file.",
				argv[i]);
			return 1;
		}

		if (strncmp("%PDF-", buffer_in, 5)) {
			fprintf(stderr,
				"%s is not a PDF file. The file signature doesn't match.",
				argv[i]);
			return 1;
		}

		/* build regex */
		// Regex rx = new Regex(@".*?FlateDecode.*?stream(?<data>.*?)endstream", RegexOptions.Singleline);

		/* build buffer */

		// TODO: find beginning of streamdata
		// GroupCollection groups = match.Groups;
		// groups["data"].Value

		/* use fscan_reg_buffer() to get streamdata stuff */

		// TODO: uncompress raw data using zlib
		// TODO: determine filetype from uncompressed data
		// TODO: save uncompressed data with file extension matching filetype
	}

	return 0;
}
