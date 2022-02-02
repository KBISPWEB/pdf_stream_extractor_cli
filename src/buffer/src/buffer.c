#include "buffer.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef OS_LINUX
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/* defaults - you can override these when building this library */

#ifndef BUFFER_DEFAULT_SIZE
/* default frame size - default 1KiB */
#define BUFFER_DEFAULT_SIZE 1023
#endif

void buffer_init(buf_t *buffer)
{
	buffer->data_length = 0;
	buffer->buf.ptr = NULL;
	buffer->buf.size = BUFFER_DEFAULT_SIZE;
	buffer->filedes = -1;
}

int buffer_resize(buf_t *buffer, size_t size)
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

void *buffer_get_bufptr(buf_t *buffer)
{
	return buffer->buf.ptr;
}

size_t buffer_get_bufsize(buf_t *buffer)
{
	return buffer->buf.size;
}

int buffer_set_datalength(buf_t *buffer, size_t datalength)
{
	if (datalength > buffer->buf.size) {
		errno = ENOMEM;
		return -1;
	}

	buffer->datalength = datalength;

	return 0;
}

size_t buffer_get_datalength(buf_t *buffer)
{
	return buffer->datalength;
}

/**
 * open a file readonly (for rbuf functions)
 */
int buffer_open(buf_t *buffer, const char *path, int oflag)
{
#if defined(OS_LINUX)
	if ((buffer->filedes = open(path, oflag)) == -1)
		return -1;
#else
	errno = ENOSYS;
	return -1;
#endif

	if (buffer_resize(buffer, buffer->buf.size))
		return -1;

	/* ALWAYS initialize this memory */
	memset(buffer->buf.ptr, 0, buffer->buf.size);

	if (buffer_rewind(buffer))
		return -1;

	return 0;
}

/**
 * close a previously opened file
 */
int buffer_close(buf_t *buffer)
{
	free(buffer->buf.ptr);
	buffer->buf.ptr = NULL;

#if defined(OS_LINUX)
	if (close(buffer->filedes))
		return -1;
#else
	errno = ENOSYS;
	return -1;
#endif

	buffer->filedes = -1;
}

off_t buffer_get_filesize(buf_t *buffer)
{
#if defined(OS_LINUX)
	struct stat buf;

	if (fstat(buffer->file.des, &buf))
		return -1;

	return buf.st_size;
#else
	errno = ENOSYS;
	return -1;
#endif
}

/**
 * set the position of the frame within the current file. doesn't refresh buffer.
 */
off_t buffer_seek(buf_t *buffer, off_t offset, int whence)
{
#if defined(OS_LINUX)
	if ((offset = lseek(buffer->file.des, offset, whence)) == -1)
		return -1;
#else
	errno = ENOSYS;
	return -1;
#endif

	buffer->offset = offset;

	return offset;
}

/**
 * a more intuitive way of seeking to the start of the file
 */
off_t buffer_rewind(buf_t *buffer)
{
	if (buffer_seek(buffer, 0, SEEK_SET))
		return -1;

	buffer->offset = 0;

	return 0;
}

off_t buffer_get_filepos(buf_t *buffer)
{
	return buffer->offset;
}

/**
 * load the current frame at the real file offset (modifies file offset, no seek)
 * Acts like a pager
 */
int buffer_read(buf_t *buffer)
{
	ssize_t bytes_read;

#if defined(OS_LINUX)
	/* this is honestly the best way I could think of to get the correct offset */
	struct stat buf;

	if (fstat(buffer->filedes, &buf))
		return -1;

	buffer->offset = buf.st_offset;

	if ((bytes_read = read(buffer->filedes, buffer->buf.ptr,
			       buffer->buf.size)) == -1)
		return -1;
#else
	errno = ENOSYS;
	return -1;
#endif
	/* fill the rest of the buffer with zeroes */
	if (bytes_read < buffer->buf.size)
		memset(buffer->buf.ptr + bytes_read, 0,
		       buffer->buf.size - bytes_read);

	buffer->data_length = bytes_read;

	return 0;
}

/**
 * reload the current frame at the current position (doesn't modify file offset)
 */
int buffer_reload(buf_t *buffer)
{
	if (buffer_seek(buffer, -buffer->buf.size, SEEK_CUR))
		return -1;

	return buffer_read(buffer);
}

/**
 * commit the current frame at the real file offset (modifies file offset and st_size, no seek)
 * Acts like a pager
 */
int buffer_write(buf_t *buffer)
{
	// TODO: WRITE, DON'T READ.
	ssize_t bytes_wrote;

#if defined(OS_LINUX)
	if ((bytes_wrote = write(buffer->filedes, buffer->buf.ptr,
				 buffer->datalength)) == -1)
		return -1;
#else
	errno = ENOSYS;
	return -1;
#endif
	/* move remaining data to the front of the buffer, if any */
	if (bytes_wrote < buffer->buf.data_length)
		memmove(buffer->buf.ptr, buffer->buf.ptr + bytes_wrote,
			buffer->datalength - bytes_wrote);

	/* fill the rest of the buffer with zeroes */
	memset(buffer->buf.ptr + bytes_wrote, 0,
	       buffer->datalength - bytes_wrote);

	buffer->offset += bytes_wrote;

	return 0;
}
