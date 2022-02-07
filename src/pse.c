#include <stdio.h> /* printing stuff to stdout */
#include <config.h>

#include <stdlib.h>

#include <ctype.h> /* isgraph */
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

int is_pdf(buffer_t buffer)
{
	/* check file signature */
	if (strncmp("%PDF-", buffer_get_bufptr(buffer), 5))
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
int goto_str(buffer_t buffer, const char *substr, size_t size)
{
	off_t offset = buffer_get_filepos(buffer);
	off_t filesize = buffer_get_filesize(buffer);
	void *bufptr = buffer_get_bufptr(buffer);
	size_t bufsize = buffer_get_bufsize(buffer);

	const void *sp;

	/* don't even bother if there's no data */
	if (offset > filesize)
		return 0;

	while ((sp = memmem(bufptr, substr, size, bufsize)) == NULL) {
		/* advance as much as we can */
		if ((offset = buffer_seek(buffer, bufsize - size, SEEK_CUR)) ==
		    -1)
			return -1;

		/* went past EOF, so it's not in the current file. */
		if (offset > filesize)
			return 0;

		if (buffer_read(buffer))
			return -1;
	}

	/* found what we were looking for. advance up to it */
	if ((offset = buffer_seek(buffer, sp - bufptr, SEEK_CUR)) == -1)
		return -1;

	if (buffer_read(buffer))
		return -1;

	return 1;
}

/* returns 1 if found, 0 if not found, and -1 on failure */
int get_stream(buffer_t buffer)
{
	off_t start = 0;
	int ret;

	/* go to FlateDecode FIRST */
	if ((ret = goto_str(buffer, "/Filter/FlateDecode", 19)) <= 0)
		return ret;

	/* go to stream start once we know we have a FlateDecode */
	/* Assume the pdf file is in the correct format. */
	if ((ret = goto_str(buffer, "stream\r\n", 8)) <= 0) {
		if ((ret = goto_str(buffer, "stream\n", 7)) <= 0)
			return ret;

		/* currently at \"stream\". I want to get to the start of the data.*/
		if (buffer_seek(buffer, 7, SEEK_CUR) == -1)
			return -1;
	} else {
		if (buffer_seek(buffer, 8, SEEK_CUR) == -1)
			return -1;
	}

	/* load data into buffer */
	if (buffer_read(buffer))
		return -1;

#ifdef DEBUG
	printf("DEBUG: found stream starting at position %ld\n",
	       buffer_get_filepos(buffer));
	fflush(stdout);
#endif

	return 1;
}

int uncompress_and_save(buffer_t buffer_in, const char *path)
{
	int ret;

	z_stream infstream = { 0 }; /* zero-init so pointers are null */

	buffer_t buffer_out = buffer_init();

	if (buffer_open(buffer_out, path, O_WRONLY | O_CREAT | O_TRUNC)) {
		fprintf(stderr, "call to buffer_open failed!\n");
		fputs(strerror(errno), stderr);
		fputc('\n', stderr);
		fflush(stderr);
		buffer_free(buffer_out);
		return -1;
	}

	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;

	/* input char buffer */
	infstream.next_in = (Bytef *)buffer_get_bufptr(buffer_in);
	/* size of input buffer */
	infstream.avail_in = (uInt)buffer_get_datalength(buffer_in);
	/* output char array */
	infstream.next_out = (Bytef *)buffer_get_bufptr(buffer_out);
	/* size of output */
	infstream.avail_out = (uInt)buffer_get_bufsize(buffer_out);

#ifdef DEBUG
	printf("DEBUG: infstream parameters: {\n\tnext_in: %lx\n\tavail_in: %ld\n\tnext_out: %lx\n\tavail_out: %ld\n}\n",
	       infstream.next_in, infstream.avail_in, infstream.next_out,
	       infstream.avail_out);
	fflush(stdout);
#endif

#ifdef DEBUG
	printf("DEBUG: uncompressing stream data\n", path);
	fflush(stdout);
#endif

	if ((ret = inflateInit(&infstream)) < 0)
		goto die;

	// TODO: uncompress raw data using zlib
	do {
		if ((ret = inflate(&infstream, Z_BLOCK)) < 0)
			goto die;

		/* can continue */

		switch (ret) {
		case Z_BUF_ERROR:
			/* no progress was made */
			__attribute__((fallthrough));
		case Z_OK:
			/* progress was made */
			// TODO: save uncompressed data
			/* save data, clear output buffer */
			if (infstream.avail_out == 0) {
				/* update next_out and avail_out */
				if (buffer_set_datalength(
					    buffer_out,
					    infstream.next_out -
						    (Bytef *)buffer_get_bufptr(
							    buffer_out)))
					goto die;

				if (buffer_write(buffer_out))
					goto die;

				infstream.next_out =
					(Bytef *)buffer_get_bufptr(buffer_out) +
					infstream.avail_out;
				infstream.avail_out =
					(uInt)buffer_get_bufsize(buffer_out);
				/* we probably recovered */
				ret = Z_OK;
			}
			if (infstream.avail_in == 0) {
				/* update next_in and avail_in */
				if (buffer_read(buffer_in))
					goto die;

				infstream.next_in =
					(Bytef *)buffer_get_bufptr(buffer_in);
				infstream.avail_in =
					(uInt)buffer_get_datalength(buffer_in);
				/* we probably recovered */
				ret = Z_OK;
			}
			/* couldn't recover from Z_BUF_ERROR */
			if (ret != Z_OK)
				goto die;
			break;
		default:
			/* stream was completely uncompressed. flush buffers */
			if (buffer_set_datalength(
				    buffer_out,
				    infstream.next_out -
					    (Bytef *)buffer_get_bufptr(
						    buffer_out)))
				goto die;

			if (buffer_write(buffer_out))
				goto die;

			break;
		}
	} while (ret == Z_OK);

	inflateEnd(&infstream);

	// TODO: determine filetype from uncompressed data

	// TODO: rename uncompressed data if path changed.

	printf("saved ./%s \n", path);
	fflush(stdout);

	buffer_free(buffer_out);

	return 0;
die:
	fprintf(stderr, "error in processing %s. (%d)", path, ret);
	if (ret == -3) {
		fprintf(stderr, ": %s", infstream.msg);
	}
	fputc('\n', stderr);
	fflush(stderr);

	inflateEnd(&infstream);
	buffer_free(buffer_out);

	return -1;
}

int main(int argc, char **argv)
{
	int ret = 0, error = 0;
	int i, stream;

	const char *filename;
	char dataname[NAME_MAX];

	buffer_t buffer = buffer_init();
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

	/* Every parameter specifies a file name */
	for (i = 1; i < argc; i++) {
		filename = argv[i];

#ifdef DEBUG
		printf("DEBUG: Opening %s\n", filename);
		fflush(stdout);
#endif

		if (buffer_open(buffer, filename, O_RDONLY)) {
			fprintf(stderr, "error: opening %s: ", filename);
			fputs(strerror(errno), stderr);
			fputc('\n', stderr);
			fflush(stderr);
			error++;
			continue;
		}

		if (buffer_read(buffer)) {
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
		       buffer_get_bufptr(buffer));
		fflush(stdout);
#endif

		if (!is_pdf(buffer)) {
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
		while ((ret = get_stream(buffer)) > 0) {
			/* create filename for data */
			sprintf(stpcpy(stpcpy(dataname, basename(filename)),
				       ".data"),
				".%d", stream);

			// TODO: this thing
			uncompress_and_save(buffer, dataname);

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

		buffer_close(buffer);
	}

#ifdef DEBUG
	printf("DEBUG: Cleanup.\n");
	fflush(stdout);
#endif
	buffer_free(buffer);
	return error;
}
