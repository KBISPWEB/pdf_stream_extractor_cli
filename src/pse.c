#include <stdio.h>
#include <config.h>

#include <stdlib.h>

#include <errno.h>
#include <regex.h>
#include <string.h>
#include <zlib.h>

#ifdef OS_WINDOWS
#include <fileapi.h>
#endif

#ifdef OS_LINUX
#include <unistd.h>
#endif

#include <filext.h>
#include <ioutils.h>

#define NMATCH 1
#define BUFFER_LEN 128

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

int is_pdf(FILE *fp)
{
	char signature[6];

	/* check file signature */
	if (fread(signature, sizeof(char), 6, fp) < 6)
		return 1;

	if (strncmp("%PDF-", signature, 5))
		return 1;

	return 0;
}

int scan_buffer(buf_t *buffer, regex_t *restrict preg,
		regmatch_t pmatch[restrict])
{
	int ret = 0;

	size_t orig_nmemb = buffer->nmemb;

	/* resize data frame until we find a match or run out of space */

	/* search our current frame for a regex match */
	while (regexec(preg, (char *)(buffer->ptr), NMATCH, pmatch, 0)) {
		/* REG_NOMATCH, increase buffer size. */
		if ((ret = ioutils_rbuf_frame_expand(buffer)))
			goto die;
	}

	/* matched, reset buffer size and put location directly after our match */
	buffer->nmemb = orig_nmemb;

	if ((ret = ioutils_buf_init(buffer)))
		goto die;

	if ((ret = ioutils_rbuf_frame_seek(buffer, pmatch[0].rm_eo, SEEK_CUR)))
		goto die;

die:
	return ret;
}

int uncompress_and_save()
{
	char buffer_in[BUFFER_LEN];
	char buffer_out[BUFFER_LEN];

	z_stream infstream = { 0 }; /* zero-init so pointers are null */

	infstream.next_in = (Bytef *)buffer_in; /* input char buffer */
	infstream.avail_in = (uInt)BUFFER_LEN; /* size of input buffer */
	infstream.next_out = (Bytef *)buffer_out; /* output char array */
	infstream.avail_out = (uInt)BUFFER_LEN; /* size of output */

	// TODO: uncompress raw data using zlib
	// TODO: determine filetype from uncompressed data
	// TODO: save uncompressed data with file filext matching filetype
}

int main(int argc, char **argv)
{
	int i;
	const char *filename;
	char *sp;

	regex_t preg;
	regmatch_t pmatch[NMATCH]; /* nmatch is hard-coded */

	buf_t buffer;

	if (argc <= 1) {
		fputs("You must specify a PDF file from which to extract stream data.\n",
		      stderr);
		return 1;
	}

	/* build regex */
	if (regcomp(&preg, "FlateDecode.*?stream(.*?)endstream",
		    REG_EXTENDED | REG_NEWLINE))
		exit(1);

	/* Every parameter specifies a file name */
	for (i = 1; i < argc; i++) {
		filename = argv[i];

		/* build buffer */
		ioutils_buf_init_defaults(&buffer);
		ioutils_rbuf_stream_open(&buffer, filename);

		//if (!(fp = fopen(filename, "rb"))) {
		//	fputs(strerror(errno), stderr);
		//	continue;
		//}

		if (!is_pdf(buffer.stream)) {
			fprintf(stderr, "%s doesn't look like a PDF file.",
				filename);
			continue;
		}

		while ((sp = strstr((char *)(buffer.ptr), "FlateDecode")) ==
		       NULL) {
			/* didn't find "FlateDecode" */
			/* advance as much as we can */
			ioutils_rbuf_frame_seek(
				&buffer, buffer.nmemb - sizeof("FlateDecode"),
				SEEK_CUR);
		}

		/* found "FlateDecode", starting at sp */
		/* advance up to "FlateDecode" so we have as much in frame as possible */
		ioutils_rbuf_frame_seek(&buffer, (char *)(buffer.ptr) - sp,
					SEEK_CUR);
		sp = buffer.ptr;

		scan_buffer(&buffer, &preg, pmatch);

		// TODO: this thing
		//uncompress_and_save();
	}

	regfree(&preg);
	return 0;
}
