#include <stdio.h>
#include <config.h>

#include <stdlib.h>

#include <errno.h>
#include <regex.h>
#include <string.h>
#include <zlib.h>

#ifdef OS_WINDOWS
/* no debugging in windows target right now, sorry! */
#ifdef DEBUG
#undef DEBUG
#endif
#include <fileapi.h>
#endif

#ifdef OS_LINUX
#include <unistd.h>
#endif

#ifdef DEBUG
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <filext.h>
#include <buffer.h>

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

int is_pdf(buf_t *buffer)
{
	/* check file signature */
#ifdef DEBUG
	printf("DEBUG: File signature looks like: \"%.8s\"\n", buffer->ptr);
	fflush(stdout);
#endif

	if (strncmp("%PDF-", buffer->ptr, 5))
		return 0;

	return 1;
}

int scan_buffer(buf_t *buffer, regex_t *restrict preg,
		regmatch_t pmatch[restrict])
{
	int ret = 0;

	size_t orig_size = buffer->size;

	/* resize data frame until we find a match or run out of space */

	/* search our current frame for a regex match */
	while (regexec(preg, (char *)(buffer->ptr), NMATCH, pmatch, 0)) {
		/* REG_NOMATCH, increase buffer size. */
		if ((ret = buffer_rbuf_frame_expand(buffer)))
			goto die;
	}

	/* matched, reset buffer size and put location directly after our match */
	buffer->size = orig_size;

	if ((ret = buffer_buf_init(buffer)))
		goto die;

	if ((ret = buffer_rbuf_frame_seek(buffer, pmatch[0].rm_eo, SEEK_CUR)))
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
	int ret = 0;
	int i;
	const char *filename;
	char *sp;

	regex_t preg;
	regmatch_t pmatch[NMATCH]; /* nmatch is hard-coded */

	buf_t buffer = { 0 }; /* IMPORTANT */

	if (argc <= 1) {
		fputs("You must specify a PDF file from which to extract stream data.\n",
		      stderr);
		return 1;
	}

	/* build regex */
	if (regcomp(&preg, "FlateDecode.*?stream(.*?)endstream",
		    REG_EXTENDED | REG_NEWLINE))
		exit(1);

#ifdef DEBUG
	printf("DEBUG: Initializing buffer defaults\n");
	fflush(stdout);
#endif

	/* build buffer */
	if (buffer_buf_init_defaults(&buffer)) {
		fputs("Initialization failure.\n", stderr);
		fflush(stderr);
		exit(1);
	}

	/* Every parameter specifies a file name */
	for (i = 1; i < argc; i++) {
		filename = argv[i];

#ifdef DEBUG
		printf("DEBUG: Opening %s\n", filename);
		fflush(stdout);
#endif

		if (buffer_rbuf_open(&buffer, filename)) {
			fprintf(stderr, "error opening %s: ", filename);
			fputs("buffer_rbuf_open: ", stderr);
			fputs(strerror(errno), stderr);
			fputc('\n', stderr);
			fflush(stderr);
			ret++;
			continue;
		}

		//if (!(fp = fopen(filename, "rb"))) {
		//	fputs(strerror(errno), stderr);
		//	continue;
		//}

#ifdef DEBUG
		printf("DEBUG: Checking %s for PDF signature\n", filename);
		fflush(stdout);
#endif

		if (!is_pdf(&buffer)) {
			fprintf(stderr, "error parsing %s: ", filename);
			fputs("the file doesn't contain a valid PDF signature.\n",
			      stderr);
			fflush(stderr);
			ret++;
			goto no_go;
		}

#ifdef DEBUG
		printf("DEBUG: Searching %s for \"FlateDecode\"\n", filename);
		fflush(stdout);
#endif

		while ((sp = strstr((char *)(buffer.ptr), "FlateDecode")) ==
		       NULL) {
			/* didn't find "FlateDecode here" */

			/* advance as much as we can */
			if (buffer_rbuf_frame_seek(
				    &buffer,
				    buffer.size - sizeof("FlateDecode"),
				    SEEK_CUR)) {
				fprintf(stderr, "error parsing %s: ", filename);
				fputs("buffer_rbuf_frame_seek: ", stderr);
				fputs(strerror(errno), stderr);
				fputc('\n', stderr);
				fflush(stderr);
				ret++;
				goto no_go;
			}

			/* went past EOF, so it's not in the current file. */
			if (buffer.actual_pos > buffer.st_size) {
				fprintf(stderr, "error parsing %s: ", filename);
				fputs("the file doesn't contain any FlateDecode stream\n",
				      stderr);
				fflush(stderr);
				ret++;
				goto no_go;
			}
		}

		/* found "FlateDecode", starting at sp */
		/* advance up to "FlateDecode" so we have as much in frame as possible */
		buffer_rbuf_frame_seek(&buffer, (char *)(buffer.ptr) - sp,
				       SEEK_CUR);
		sp = buffer.ptr;

		scan_buffer(&buffer, &preg, pmatch);

		/* the match should be in pmatch[1] */
		// print for debugging
		*((char *)(buffer.ptr + pmatch[1].rm_eo + 1)) = 0;
		puts((char *)(buffer.ptr + pmatch[1].rm_so));

		// TODO: this thing
		//uncompress_and_save();
	no_go:
#ifdef DEBUG
		printf("DEBUG: Closing %s\n", filename);
		fflush(stdout);
#endif

		buffer_buf_close(&buffer);
	}

	buffer_buf_free(&buffer);
	regfree(&preg);
	return ret;
}
