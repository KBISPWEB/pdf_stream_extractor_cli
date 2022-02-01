#include <stdio.h>
#include <config.h>

#include <stdlib.h>

#include <errno.h>
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
	if (strncmp("%PDF-", buffer->ptr, 5))
		return 0;

	return 1;
}

const void *memmem(const void *s1, const void *s2, size_t n, const size_t size)
{
	size_t offset = 0;
	void *sp;

	while (memcmp(s1 + offset, s2, n) != 0) {
		if ((sp = memchr(s1 + offset + 1, *((char *)s2), n - 1)) !=
		    NULL) {
			offset = sp - s1;
		} else {
			offset += n;
		}
		if ((offset + n) > size)
			return NULL;
	}

	return s1 + offset;
}

int advance_to_str(buf_t *buffer, const char *substr)
{
	const void *sp;
	size_t size = strlen(substr);

	while ((sp = memmem(buffer->ptr, substr, size, buffer->size)) == NULL) {
		/* advance as much as we can */
		if (buffer_rbuf_frame_seek(buffer, buffer->size - size,
					   SEEK_CUR)) {
			fputs("error: buffer_rbuf_frame_seek: ", stderr);
			return -1;
		}

		/* went past EOF, so it's not in the current file. */
		if (buffer_buf_eof(buffer))
			return EOF;
	}

	/* found what we were looking for. advance up to it */
	if (buffer_rbuf_frame_seek(buffer, sp - buffer->ptr, SEEK_CUR)) {
		fputs("error: buffer_rbuf_frame_seek: ", stderr);
		return -1;
	}

	return 0;
}

struct match {
	off_t start;
	off_t end;
};

int get_stream(buf_t *buffer, struct match *pmatch)
{
	int ret;

	/* Search for FlateDecode */
	if ((ret = advance_to_str(buffer, "FlateDecode")))
		return ret;

	/* Search for stream start */
	if ((ret = advance_to_str(buffer, "stream")))
		return ret;

	/* currently at \"stream\". I want to get to the start of the data.*/
	pmatch->start = buffer->pos + 6;

	/* Search for stream end */
	if ((ret = advance_to_str(buffer, "endstream")))
		return ret;

	pmatch->end = buffer->pos;

	/* seek past stream end */
	if (buffer_rbuf_frame_seek(buffer, 9, SEEK_CUR)) {
		fputs("error: buffer_rbuf_frame_seek: ", stderr);
		return -1;
	}

	return 0;
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
	int ret = 0, error = 0;
	int i, stream;
	const char *filename;

	struct match pmatch;
	buf_t buffer = { 0 }; /* IMPORTANT */

	if (argc <= 1) {
		fputs("You must specify a PDF file from which to extract stream data.\n",
		      stderr);
		return 1;
	}

#ifdef DEBUG
	printf("DEBUG: Initializing buffer defaults\n");
	fflush(stdout);
#endif

	/* build buffer */
	if (buffer_buf_init_defaults(&buffer)) {
		fputs("Initialization failure.\n", stderr);
		fflush(stderr);
		return 1;
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
			error++;
			continue;
		}

#ifdef DEBUG
		printf("DEBUG: Checking %s for PDF signature\n", filename);
		printf("DEBUG: File signature looks like: \"%.8s\"\n",
		       buffer.ptr);
		fflush(stdout);
#endif

		if (!is_pdf(&buffer)) {
			fprintf(stderr,
				"error: When validating %s: ", filename);
			fputs("The file doesn't contain a valid PDF signature.\n",
			      stderr);
			fflush(stderr);
			error++;
			goto no_go;
		}

		// TODO: Do this until there are no more streams.
		stream = 0;
		errno = 0;
		while ((ret = get_stream(&buffer, &pmatch)) != EOF) {
			if (ret) {
				fprintf(stderr, "When scanning scanning %s: ",
					filename);
				fputs(strerror(errno), stderr);
				fputc('\n', stderr);
				fflush(stderr);
				error++;
				goto no_go;
			}
#ifdef DEBUG
			printf("DEBUG: found stream #%d starting at position %ld with size %ld\n",
			       stream, pmatch.start, pmatch.end - pmatch.start);
			fflush(stdout);
#endif
			stream++;
		}

		// TODO: this thing
		//uncompress_and_save();
	no_go:
#ifdef DEBUG
		printf("DEBUG: Closing %s\n", filename);
		fflush(stdout);
#endif

		buffer_buf_close(&buffer);
	}

#ifdef DEBUG
	printf("DEBUG: Cleanup.\n");
	fflush(stdout);
#endif
	buffer_buf_free(&buffer);
	return error;
}
