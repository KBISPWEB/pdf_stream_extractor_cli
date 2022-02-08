#include <stdio.h> /* printing stuff to stdout */
#include <config.h>

#include <stdlib.h>
#include <libgen.h> /* basename */
#include <string.h> /* string and memory stuff */
#include <errno.h> /* errno */
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

#include "stream_scanner.h"

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

buffer_t buffer_out = NULL;

int uncompress_and_save(buffer_t buffer_in, char *path)
{
	int ret;

	z_stream infstream = { 0 }; /* zero-init so pointers are null */

	char *ext;
	char *newpath;

	buffer_out = buffer_init();

#ifdef DEBUG
	printf("DEBUG: opening \"%s\"\n", path);
	fflush(stdout);
#endif

	if (buffer_open(buffer_out, path, O_RDWR | O_CREAT | O_TRUNC)) {
		fprintf(stderr, "buffer_open: opening \"%s\" failed: ", path);
		fputs(strerror(errno), stderr);
		fputc('\n', stderr);
		fflush(stderr);
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
	//printf("DEBUG: infstream parameters: {\n\tnext_in: %lx\n\tavail_in: %ld\n\tnext_out: %lx\n\tavail_out: %ld\n}\n",
	//       infstream.next_in, infstream.avail_in, infstream.next_out,
	//       infstream.avail_out);
	//fflush(stdout);
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
	if (buffer_rewind(buffer_out))
		goto die;

	if (buffer_read(buffer_out))
		goto die;

	newpath = malloc(strlen(path) + 5);
	strcpy(newpath, path);

	if (ext = get_file_filext(buffer_get_bufptr(buffer_out),
				  buffer_get_bufsize(buffer_out))) {
		strcat(newpath, ".");
		strcat(newpath, ext);
	} else {
		strcat(newpath, ".dat");
	}

	buffer_free(buffer_out);
	buffer_out = NULL;

	// TODO: rename uncompressed data.

#ifdef OS_LINUX
	if (rename(path, newpath)) {
		free(newpath);
		goto die;
	}
#else
	errno = ENOSYS;
	return -1;
#endif

	printf("saved ./%s \n", newpath);
	fflush(stdout);

	free(newpath);

	return 0;
die:
	fprintf(stderr, "error in processing %s. (%d)", path, ret);
	if (ret == -3) {
		fprintf(stderr, ": %s", infstream.msg);
	}
	fputc('\n', stderr);
	fflush(stderr);

	inflateEnd(&infstream);

#ifdef OS_LINUX
	unlink(path);
#else
	errno = ENOSYS;
	return -1;
#endif

	return -1;
}

buffer_t buffer = NULL;

void cleanup(void)
{
	if (buffer)
		buffer_free(buffer);

	if (buffer_out)
		buffer_free(buffer_out);
}

int main(int argc, char *argv[])
{
	int ret = 0, error = 0;
	int i, stream;

	char *filename = NULL;
	char dataname[NAME_MAX];

	size_t objlen = 0;
	off_t streamstart = 0;
	off_t objend = 0;

	size_t orig_size;

	if (atexit(cleanup)) {
		fputs("The function could not be registered.\n", stderr);
		return 1;
	}

	buffer = buffer_init();

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
		while ((ret = get_obj(buffer, &objlen)) > 0) {
			// TODO: make sure stream data is OK to uncompress.
			if (strstr(buffer_get_bufptr(buffer),
				   "/Filter/FlateDecode/") != NULL) {
#ifdef DEBUG
				printf("DEBUG: found stream starting at position %ld, obj length is:%ld\n",
				       buffer_get_filepos(buffer), objlen);
				fflush(stdout);
#endif
				objend = buffer_get_filepos(buffer) + objlen;

				if ((ret = buffer_find_mem(buffer, "stream\r\n",
							   7, &streamstart,
							   1)) <= 0) {
					if ((ret = buffer_find_mem(
						     buffer, "stream\n", 7,
						     &streamstart, 1)) <= 0) {
						fputs("searching for stream start failed!\n",
						      stderr);
						continue;
					} else {
						if (buffer_seek(buffer,
								streamstart + 7,
								SEEK_SET) == -1)
							continue;
					}
				} else {
					if (buffer_seek(buffer, streamstart + 8,
							SEEK_SET) == -1)
						continue;
				}

				/* create filename for data */
				sprintf(stpcpy(stpcpy(dataname,
						      basename(filename)),
					       ".data"),
					".%d", stream);

				uncompress_and_save(buffer, dataname);

				stream++;

				if (buffer_seek(buffer,
						objend + sizeof("objend"),
						SEEK_SET) == -1) {
					ret = -1;
					break;
				}
			}
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
	return error;
}
