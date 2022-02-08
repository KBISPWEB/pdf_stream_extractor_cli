#include "buffer.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

//#ifdef OS_LINUX
#include <sys/stat.h>
#include <unistd.h>
//#endif

/* defaults - you can override these when building this library */

#ifndef BUFFER_DEFAULT_SIZE
/* default frame size - default 1KiB */
#define BUFFER_DEFAULT_SIZE 1023
#endif

struct buffer {
	size_t datalength; /* size of the data in the buffer in bytes */

	struct {
		void *ptr; /* operation buffer */
		size_t size; /* size of the buffer in bytes */
	} buf;

	int filedes; /* filedes number */
	off_t offset; /* offset in file from where the start of buffer is */
	int read;
};

buffer_t buffer_init()
{
	buffer_t buffer = malloc(sizeof(struct buffer));

	buffer->datalength = 0;
	buffer->buf.ptr = NULL;
	buffer->buf.size = BUFFER_DEFAULT_SIZE;
	buffer->filedes = -1;
	buffer->read = 0;

	return buffer;
}

void buffer_free(buffer_t buffer)
{
	if (!buffer)
		return;

	if (buffer->filedes != -1)
		buffer_close(buffer);

	free(buffer);
}

int buffer_resize(buffer_t buffer, size_t size)
{
	void *tmp;

	tmp = realloc(buffer->buf.ptr, size);

	if ((tmp == NULL) && (size != 0)) {
		errno = ENOMEM;
		return -1;
	}

	buffer->buf.ptr = tmp;
	buffer->buf.size = size;

	return 0;
}

void *buffer_get_bufptr(buffer_t buffer)
{
	return buffer->buf.ptr;
}

size_t buffer_get_bufsize(buffer_t buffer)
{
	return buffer->buf.size;
}

int buffer_set_datalength(buffer_t buffer, size_t datalength)
{
	if (datalength > buffer->buf.size) {
		errno = ENOMEM;
		return -1;
	}

	buffer->datalength = datalength;

	return 0;
}

size_t buffer_get_datalength(buffer_t buffer)
{
	return buffer->datalength;
}

/**
 * open a file readonly (for rbuf functions)
 */
int buffer_open(buffer_t buffer, const char *path, int oflag)
{
	if ((buffer->filedes = open(path, oflag, S_IRUSR | S_IWUSR)) == -1)
		return -1;

	if (buffer_resize(buffer, buffer->buf.size))
		return -1;

	/* ALWAYS initialize this memory */
	memset(buffer->buf.ptr, 0, buffer->buf.size);

	return 0;
}

/**
 * close a previously opened file
 */
int buffer_close(buffer_t buffer)
{
	free(buffer->buf.ptr);
	buffer->buf.ptr = NULL;

	if (close(buffer->filedes))
		return -1;

	buffer->filedes = -1;
}

off_t buffer_get_filesize(buffer_t buffer)
{
	struct stat buf;

	if (fstat(buffer->filedes, &buf))
		return -1;

	return buf.st_size;
}

/**
 * set the position of the frame within the current file. doesn't refresh buffer.
 */
off_t buffer_seek(buffer_t buffer, off_t offset, int whence)
{
	if (whence == SEEK_CUR) {
		offset = buffer->offset + offset;
		whence = SEEK_SET;
	}

	if ((offset = lseek(buffer->filedes, offset, whence)) == -1)
		return -1;

	buffer->offset = offset;

	return offset;
}

/**
 * a more intuitive way of seeking to the start of the file
 */
off_t buffer_rewind(buffer_t buffer)
{
	if (buffer_seek(buffer, 0, SEEK_SET) < 0) {
		fprintf(stderr, "buffer_seek: ");
		return -1;
	}

	buffer->offset = 0;

	return 0;
}

off_t buffer_get_filepos(buffer_t buffer)
{
	return buffer->offset;
}

/**
 * load the current frame at the real file offset (modifies file offset, no seek)
 * Acts like a pager
 */
int buffer_read(buffer_t buffer)
{
	ssize_t bytes_read;

	if (buffer_seek(buffer, buffer->datalength, SEEK_CUR) < 0) {
		fprintf(stderr, "buffer_seek: ");
		return -1;
	}

	/* fill all of the buffer with zeroes */
	memset(buffer->buf.ptr, 0, buffer->buf.size);

	if ((bytes_read = read(buffer->filedes, buffer->buf.ptr,
			       buffer->buf.size)) == -1)
		return -1;

	return (buffer->datalength = bytes_read);
}

/**
 * reload the current frame at the current position (doesn't modify file offset)
 */
int buffer_reload(buffer_t buffer)
{
	buffer->datalength = 0;
	return buffer_read(buffer);
}

/**
 * commit the current frame at the real file offset (modifies file offset and st_size, no seek)
 * Acts like a pager
 */
int buffer_write(buffer_t buffer)
{
	// TODO: WRITE, DON'T READ.
	ssize_t bytes_wrote;

	if (buffer_seek(buffer, 0, SEEK_CUR) == -1)
		return -1;

	if ((bytes_wrote = write(buffer->filedes, buffer->buf.ptr,
				 buffer->datalength)) == -1)
		return -1;

	/* move remaining data to the front of the buffer, if any */
	if (bytes_wrote < buffer->datalength)
		memmove(buffer->buf.ptr, buffer->buf.ptr + bytes_wrote,
			buffer->datalength - bytes_wrote);

	/* fill the rest of the buffer with zeroes */
	memset(buffer->buf.ptr + bytes_wrote, 0,
	       buffer->datalength - bytes_wrote);

	buffer->offset += bytes_wrote;

	return 0;
}
