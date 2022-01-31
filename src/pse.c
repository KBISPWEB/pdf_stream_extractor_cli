#include <stdio.h>
#include <config.h>

#ifdef OS_WINDOWS
#include <fileapi.h>
#endif

#ifdef OS_LINUX
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <zlib.h>

#include <filext.h>
#include <ioutils.h>

//filext_table_t filext_table = {
//	1, /* number of records */
//	/* start of records array */
//	&(filext_record_t[]){
//		/* start of record */
//		{
//			.signature = { 7, /* size of signature in bytes */
//				       /* signature data */
//				       &(filext_signature_t[]){
//					       0x30, 0x26, 0xb2, 0x75, 0x8e,
//					       0x66, 0xcf } },
//			.offsets = { 1, /* number of unique offsets */
//				     /* offsets array */
//				     &(filext_offset_t[]){ 0 } },
//			.extension = "wmv" /* file extension */
//		} /* end of record */
//	} /* end of records array */
//};

filext_table_t filext_records = FILEXT_TABLE(FILEXT_RECORD(
	FILEXT_SIGNATURE(0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf),
	FILEXT_OFFSETS(0), "wmv"));

char *get_file_filext(uint8_t *data, size_t size)
{
	char *ext;
	filext_sample_t sample = { .length = size, .sample = data };

	ext = filext_get_filext(&sample);

	if (ext != NULL) {
		return ext;
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
		// TODO: save uncompressed data with file filext matching filetype
	}

	return 0;
}
