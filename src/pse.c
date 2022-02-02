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

/* returns 1 if found, 0 if not found, and -1 on failure */
int find_str(buf_t *buffer, const char *substr, size_t size)
{
	off_t offset;
	off_t filesize = buffer_get_filesize(buffer);

	const void *sp;

	while ((sp = memmem(buffer->ptr, substr, size, buffer->size)) == NULL) {
		/* advance as much as we can */
		if ((offset = buffer_seek(buffer, buffer->size - size,
					  SEEK_CUR)) == -1)
			return -1;

		/* went past EOF, so it's not in the current file. */
		if (offset > filesize)
			return 0;
	}

	/* found what we were looking for. advance up to it */
	if ((offset = buffer_seek(buffer, sp - buffer->ptr, SEEK_CUR)) == -1)
		return -1;

	return 1;
}

/* returns 1 if found, 0 if not found, and -1 on failure */
int get_stream(buf_t *buffer)
{
	off_t offset = 0;
	int ret;

	/* make sure we have enough space here. LCM(11, 6, 9) * 2 = 396 */
	if (buffer_resize(buffer, 396);
		return -1;

	/* Search for FlateDecode */
	if ((ret = find_str(buffer, "FlateDecode", 11)) <= 0)
		return ret;

	/* Search for stream start */
	if ((ret = find_str(buffer, "stream", 6)) <= 0)
		return ret;

	/* currently at \"stream\". I want to get to the start of the data.*/
	start = buffer_get_filepos(buffer) + 6;

	/* Search for stream end */
	if ((ret = find_str(buffer, "endstream", 9)) <= 0)
		return ret;

	/* make sure the frame is the correct size */
	if (buffer_resize(buffer, buffer_get_filepos(buffer) - start))
		return -1;

#ifdef DEBUG
	printf("DEBUG: found stream #%d starting at position %ld with size %ld\n",
	       stream, buffer.pos, buffer.size);
	printf("DEBUG: loading entire stream into buffer...\n");
	fflush(stdout);
#endif

	/* position buffer over entire stream */
	if (buffer_seek(buffer, start, SEEK_SET))
		return -1;

	return 1;
}

int uncompress_and_save(buf_t *buffer_in, const char *path)
{
	z_stream infstream = { 0 }; /* zero-init so pointers are null */

	buf_t buffer_out;

	buffer_init(&buffer_out);

	buffer_open(&buffer_out, path, O_WRONLY);

	/* input char buffer */
	infstream.next_in = (Bytef *)buffer_get_bufptr(buffer_in);
	/* size of input buffer */
	infstream.avail_in = (uInt)buffer_get_datalength(buffer_in);
	/* output char array */
	infstream.next_out = (Bytef *)buffer_get_bufptr(buffer_out);
	/* size of output */
	infstream.avail_out = (uInt)buffer_get_bufsize(buffer_out);

#ifdef DEBUG
	printf("DEBUG: uncompressing stream data\n", path);
	fflush(stdout);
#endif

	inflateInit(&infstream);

	// TODO: uncompress raw data using zlib
	while (inflate(&infstream, Z_NO_FLUSH) != Z_STREAM_END) {
		// TODO: save uncompressed data
		/* save data, clear output buffer */
		if (infstream.avail_out == 0) {
			buffer_set_datalength(buffer_out, );
			buffer_write(buffer_out);
			infstream.avail_out =
				(uInt)buffer_get_bufsize(buffer_out);
			infstream.avail_in =
				(uInt)buffer_get_datalength(buffer_in);
		}
		if (infstream.avail_in == 0) {
		}
	}

	inflateEnd(&infstream);

	// TODO: determine filetype from uncompressed data

	// TODO: rename uncompressed data if path changed.

	printf("saved ./%s \n", path);
	fflush(stdout);

	buffer_free(&buffer_out);
}

int main(int argc, char **argv)
{
	int ret = 0, error = 0;
	int i, stream;

	const char *filename;
	char dataname[NAME_MAX];

	buf_t buffer;
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

	buffer_init(&buffer);

	/* Every parameter specifies a file name */
	for (i = 1; i < argc; i++) {
		filename = argv[i];

#ifdef DEBUG
		printf("DEBUG: Opening %s\n", filename);
		fflush(stdout);
#endif

		if (buffer_open(&buffer, filename, O_RDONLY)) {
			fprintf(stderr, "error: opening %s: ", filename);
			fputs(strerror(errno), stderr);
			fputc('\n', stderr);
			fflush(stderr);
			error++;
			continue;
		}

		if (buffer_read(&buffer)) {
			fprintf(stderr, "error: reading %s: ", filename);
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
			fprintf(stderr, "error: validating %s: ", filename);
			fputs("The file doesn't contain a valid PDF signature.\n",
			      stderr);
			fflush(stderr);
			error++;
			goto no_go;
		}

		// TODO: Do this until there are no more streams.
		stream = 0;
		errno = 0;
		while ((ret = get_stream(&buffer)) == 0) {
			/* create filename for data */
			sprintf(stpcpy(stpcpy(dataname, basename(filename)),
				       ".data"),
				".%d", stream);

			// TODO: this thing
			uncompress_and_save(buffer.ptr, buffer.size, dataname);

			stream++;
		}
		if (ret < 0) {
			fprintf(stderr, "error: scanning %s: ", filename);
			fputs(strerror(errno), stderr);
			fputc('\n', stderr);
			fflush(stderr);
			error++;
			break;
		}

	no_go:
#ifdef DEBUG
		printf("DEBUG: Closing %s\n", filename);
		fflush(stdout);
#endif

		buffer_close(&buffer);
	}

#ifdef DEBUG
	printf("DEBUG: Cleanup.\n");
	fflush(stdout);
#endif
	return error;
}
