#include "buffer.h"

#include <errno.h>
#include <stdlib.h>

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

#ifndef BUFFER_DEFAULT_EXPAND
/* default frame expansion - default 1KiB */
#define BUFFER_DEFAULT_EXPAND 1023
#endif

/**
 * initialize internal buffer based on structure's size parameter
 */
int buffer_buf_init(buf_t *buffer)
{
	void *tmp;

	tmp = realloc(buffer->ptr, buffer->size + 1);

	if ((tmp == NULL) && (buffer->size != 0)) {
		errno = ENOMEM;
		return -1;
	}

	buffer->ptr = tmp;

	*((char *)(buffer->ptr + buffer->size - 1)) = 0; /* null-terminate */

	return 0;
}

/**
 * initialize buf_t compile-time defaults
 */
int buffer_buf_init_defaults(buf_t *buffer)
{
	buffer->size = BUFFER_DEFAULT_SIZE;
	buffer->size_e = BUFFER_DEFAULT_EXPAND;

	return buffer_buf_init(buffer);
}

/**
 * free internal buffer
 */
void buffer_buf_free(buf_t *buffer)
{
	free(buffer->ptr);
	buffer->ptr = NULL;
	buffer->size = 0;
}

/**
 * close a previously opened file
 */
int buffer_buf_close(buf_t *buffer)
{
#if defined(OS_LINUX)
	return close(buffer->filedes);
#else
	errno = ENOSYS;
	return -1;
#endif
}

int buffer_buf_eof(buf_t *buffer)
{
	return (buffer->actual_pos > buffer->st_size);
}

/**
 * set the position of the frame within the current file. doesn't refresh buffer.
 */
int buffer_buf_frame_seek(buf_t *buffer, off_t offset, int whence)
{
	off_t new_pos;

	switch (whence) {
	case SEEK_SET:
		new_pos = offset;
		break;
	case SEEK_CUR:
		new_pos = buffer->pos + offset;
		break;
	case SEEK_END:
		new_pos = buffer->st_size - offset;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

#if defined(OS_LINUX)
	if ((new_pos = lseek(buffer->filedes, new_pos, SEEK_SET)) == -1)
		return -1;
#else
	errno = ENOSYS;
	return -1;
#endif

	buffer->actual_pos = buffer->pos = new_pos;

	return 0;
}

/**
 * a more intuitive way of seeking to the start of the file
 */
int buffer_buf_frame_rewind(buf_t *buffer)
{
	return buffer_buf_frame_seek(buffer, 0, SEEK_SET);
}

/**
 * open a file readonly (for rbuf functions)
 */
int buffer_rbuf_open(buf_t *buffer, const char *path)
{
#if defined(OS_LINUX)
	struct stat buf;

	if ((buffer->filedes = open(path, O_RDONLY)) == -1)
		return -1;

	if (stat(path, &buf))
		return -1;

	buffer->st_size = buf.st_size;
#else
	errno = ENOSYS;
	return -1;
#endif

	return buffer_rbuf_frame_rewind(buffer);
}

/**
 * load the current frame at the real file offset (modifies file offset, no seek)
 * Acts like a pager
 */
int buffer_rbuf_frame_load(buf_t *buffer)
{
	ssize_t bytes_read;

	buffer->pos = buffer->actual_pos;

#if defined(OS_LINUX)
	if ((bytes_read = read(buffer->filedes, buffer->ptr, buffer->size)) ==
	    -1)
		return -1;
#else
	errno = ENOSYS;
	return -1;
#endif
	if (bytes_read < buffer->size) {
		*((char *)(buffer->ptr + bytes_read)) = 0;
		// maybe set an EOF flag?
	}

	buffer->actual_pos = buffer->pos + bytes_read;

	return 0;
}

/**
 * seeks and refreshes the current frame
 */
int buffer_rbuf_frame_seek(buf_t *buffer, off_t offset, int whence)
{
	if (buffer_buf_frame_seek(buffer, offset, whence))
		return -1;

	return buffer_rbuf_frame_load(buffer);
}

/**
 * rewinds and refreshes the current frame
 */
int buffer_rbuf_frame_rewind(buf_t *buffer)
{
	if (buffer_buf_frame_rewind(buffer))
		return -1;

	return buffer_rbuf_frame_load(buffer);
}

/**
 * reload the current frame at the current position (doesn't modify file offset)
 */
int buffer_rbuf_frame_reload(buf_t *buffer)
{
	return buffer_rbuf_frame_seek(buffer, buffer->pos, SEEK_SET);
}

/**
 * expand the current frame by the expansion value in the struct
 */
int buffer_rbuf_frame_expand(buf_t *buffer)
{
	size_t new_size;

	new_size = buffer->size + buffer->size_e;

	/* only allowed to increase in size */
	if (new_size < buffer->size) {
		errno = EOVERFLOW;
		return -1;
	}

	buffer->size = new_size;

	if (buffer_buf_init(buffer))
		return -1;

	if (buffer_rbuf_frame_reload(buffer))
		return -1;

	return 0;
}

/**
 * contracts the current frame by the expansion value in the struct
 */
int buffer_rbuf_frame_contract(buf_t *buffer)
{
	size_t new_size;

	new_size = buffer->size + buffer->size_e;

	/* only allowed to decrease in size */
	if (new_size > buffer->size) {
		errno = EOVERFLOW;
		return -1;
	}

	buffer->size = new_size;

	if (buffer_buf_init(buffer))
		return -1;

	if (buffer_rbuf_frame_reload(buffer))
		return -1;

	return 0;
}

/**
 * resets the position and size of the frame
 */
int buffer_rbuf_frame_reset(buf_t *buffer)
{
	if (buffer_buf_init_defaults(buffer))
		return -1;

	if (buffer_rbuf_frame_load(buffer))
		return -1;

	return 0;
}
