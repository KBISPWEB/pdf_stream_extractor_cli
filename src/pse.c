#include <stdio.h> /* printing stuff to stdout */
#include <config.h>

#include <stdlib.h>

#include <libgen.h> /* basename */
#include <errno.h> /* errno */
#include <string.h> /* string and memory stuff */
#include <zlib.h> /* zlib compression and uncompression */

#ifdef OS_WINDOWS
/* no debugging in windows target right now, sorry! */
#ifdef DEBUG
#undef DEBUG
#endif
#include <fileapi.h> /* low level file stuff */
#endif

#ifdef OS_LINUX
#include <limits.h> /* filename limit */
#include <unistd.h> /* low level file stuff */
#endif

#include <filext.h> /* my file extension library */
#include <buffer.h> /* my buffer library */

filext_table_t filext_records = FILEXT_TABLE(FILEXT_RECORD(
	FILEXT_SIGNATURE(0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf),
	FILEXT_OFFSETS(0), "wmv"));

char *get_file_filext(uint8_t *data, size_t size)
{
	char *ext;
	filext_sample_t sample = { .length = size, .sample = data };

	ext = filext_get_filext(&sample);

	return ext;
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

int get_stream(buf_t *buffer)
{
	off_t start;
	int ret;

	/* make sure we have enough space here. */
	if (buffer_rbuf_frame_reset(buffer)) {
		fputs("error: buffer_rbuf_frame_reset: ", stderr);
		fputs(strerror(errno), stderr);
		fputc('\n', stderr);
		fflush(stderr);
		return -1;
	}

	/* Search for FlateDecode */
	if ((ret = advance_to_str(buffer, "FlateDecode")))
		return ret;

	/* Search for stream start */
	if ((ret = advance_to_str(buffer, "stream")))
		return ret;

	/* currently at \"stream\". I want to get to the start of the data.*/
	start = buffer->pos + 6;

	/* Search for stream end */
	if ((ret = advance_to_str(buffer, "endstream")))
		return ret;

	/* make sure the frame is the correct size */
	buffer->size = buffer->pos - start;

	if (buffer_buf_init(buffer)) {
		fputs("error: buffer_buf_init: ", stderr);
		fputs(strerror(errno), stderr);
		fputc('\n', stderr);
		fflush(stderr);
		return -1;
	}

	/* position buffer over entire stream */
	if (buffer_rbuf_frame_seek(buffer, start, SEEK_SET)) {
		fputs("error: buffer_rbuf_frame_seek: ", stderr);
		fputs(strerror(errno), stderr);
		fputc('\n', stderr);
		fflush(stderr);
		return -1;
	}

	return 0;
}

int uncompress_and_save(const char *bufp, const size_t size, const char *path)
{
	z_stream infstream = { 0 }; /* zero-init so pointers are null */

	wbuf_t buffer_out;

	buffer_buf_construct((but_t *)&buffer_out);
	buffer_buf_init_defaults((buf_t *)&buffer_out);

	buffer_wbuf_open(&buffer_out, path);

	infstream.next_in = (Bytef *)bufp; /* input char buffer */
	infstream.avail_in = (uInt)size; /* size of input buffer */
	infstream.next_out = (Bytef *)(buffer_out.ptr); /* output char array */
	infstream.avail_out = (uInt)(buffer_out.size); /* size of output */

#ifdef DEBUG
	printf("DEBUG: uncompressing stream data\n", path);
	fflush(stdout);
#endif

	inflateInit(&infstream);
	// TODO: uncompress raw data using zlib
	while (inflate(&infstream, Z_NO_FLUSH) != Z_STREAM_END) {
		// TODO: save uncompressed data
	}

	inflateEnd(&infstream);

	// TODO: determine filetype from uncompressed data

	// TODO: rename uncompressed data if path changed.

	printf("saved ./%s \n", path);
	fflush(stdout);

	buffer_buf_free((buf_t *)&buffer_out);
}

int main(int argc, char **argv)
{
	int ret = 0, error = 0;
	int i, stream;

	const char *filename;
	char dataname[NAME_MAX];

	rbuf_t buffer;
	size_t orig_size;

	if (argc <= 1) {
		fputs("You must specify a PDF file from which to extract stream data.\n",
		      stderr);
		return 1;
	}

#ifdef DEBUG
	printf("DEBUG: Initializing buffer defaults\n");
	fflush(stdout);
#endif

	buffer_buf_construct((buf_t *)&buffer);

	/* build buffer */
	if (buffer_buf_init_defaults((buf_t *)&buffer)) {
		fputs("Initialization failure: buffer_buf_init_defaults: ",
		      stderr);
		fputs(strerror(errno), stderr);
		fputc('\n', stderr);
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
		while ((ret = get_stream(&buffer)) != EOF) {
			if (ret) {
				fprintf(stderr, "When scanning scanning %s: ",
					filename);
				fputs(strerror(errno), stderr);
				fputc('\n', stderr);
				fflush(stderr);
				error++;
				break;
			}
#ifdef DEBUG
			printf("DEBUG: found stream #%d starting at position %ld with size %ld\n",
			       stream, buffer.pos, buffer.size);
			printf("DEBUG: loading entire stream into buffer...\n");
			fflush(stdout);
#endif

			/* create filename for data */
			sprintf(stpcpy(stpcpy(dataname, basename(filename)),
				       ".data"),
				".%d", stream);

			// TODO: this thing
			uncompress_and_save(buffer.ptr, buffer.size, dataname);

			stream++;
		}

	no_go:
#ifdef DEBUG
		printf("DEBUG: Closing %s\n", filename);
		fflush(stdout);
#endif

		buffer_buf_close((buf_t *)&buffer);
	}

#ifdef DEBUG
	printf("DEBUG: Cleanup.\n");
	fflush(stdout);
#endif
	buffer_buf_free((buf_t *)&buffer);
	return error;
}
