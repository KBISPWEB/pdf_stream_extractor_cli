#include <config.h>

#include <stdio.h>

#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer {
	size_t data_length; /* size of the data in the buffer in bytes */

	struct {
		void *ptr; /* operation buffer */
		size_t size; /* size of the buffer in bytes */
	} buf;

	int filedes; /* filedes number */
	off_t offset; /* offset in file from where the start of buffer is */
} buf_t;

/**
 * construct a new buffer structure to be "nothing"
 * @param buffer  [description]
 */
void buffer_init(buf_t *buffer);

/**
 * resize internal buffer
 * @param  buffer               [description]
 * @param  size                 [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_resize(buf_t *buffer, size_t size);

/**
 * open a file readonly (for rbuf functions)
 * @param  buffer               [description]
 * @param  path                 path to file
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_open(buf_t *buffer, const char *path, int oflag);

/**
 * close a previously opened file
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_close(buf_t *buffer);

/**
 * get size of current file
 * @param  buffer               [description]
 * @return        positive number representing the size of the file, -1 on failure
 */
off_t buffer_get_filesize(buf_t *buffer);

/**
 * set the position of the frame within the current file. doesn't refresh buffer.
 * @param  buffer               [description]
 * @param  offset               [description]
 * @param  whence               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
off_t buffer_seek(buf_t *buffer, off_t offset, int whence);

/**
 * a more intuitive way of seeking to the start of the file
 * @param  buffer               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
off_t buffer_rewind(buf_t *buffer);

/**
 * get position in current file
 * @param  buffer               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
off_t buffer_get_filepos(buf_t *buffer);

/**
 * load the current frame at the real file offset (modifies file offset, no seek)
 * Acts like a pager
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_read(buf_t *buffer);

/**
 * reload the current frame at the current position (doesn't modify file offset)
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_reload(buf_t *buffer);

/**
 * commit the current frame at the real file offset (modifies file offset and st_size, no seek)
 * Acts like a pager
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_write(buf_t *buffer);

#endif
