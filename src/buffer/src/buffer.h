#include <config.h>

#include <stdio.h>

#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer {
	void *ptr; /* the working buffer */
	size_t size; /* size of the buffer in bytes */

	int filedes; /* the current working stream */
	off_t st_size; /* the total size of the file */

	off_t actual_pos; /* the real offset in the file */
	off_t pos; /* the current position of the frame */
} buf_t;

typedef buf_t rwbuf_t;

typedef struct buffer {
	void *ptr; /* the working buffer */
	size_t size; /* size of the buffer in bytes */

	const int filedes; /* the current working stream */
	const off_t st_size; /* the total size of the file */

	off_t actual_pos; /* the real offset in the file */
	off_t pos; /* the current position of the frame */
} rbuf_t;

typedef struct buffer {
	void *ptr; /* the working buffer */
	size_t size; /* size of the buffer in bytes */

	const int filedes; /* the current working stream */
	off_t st_size; /* the total size of the file */

	const off_t actual_pos; /* the real offset in the file */
	const off_t pos; /* the current position of the frame */
} wbuf_t;

/**
 * construct a new buffer structure to be "nothing"
 * @param buffer  [description]
 */
void buffer_construct(buf_t *buffer);

/**
 * initialize internal buffer based on structure's size parameter
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_init(buf_t *buffer);

/**
 * initialize buf_t compile-time defaults
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_init_defaults(buf_t *buffer);

/**
 * free internal buffer
 * @param buffer  [description]
 */
void buffer_free(buf_t *buffer);

/**
 * close a previously opened file
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_close(buf_t *buffer);

/**
 * test whether buffer has reached (or gone past) the real eof
 * @param  buffer               [description]
 * @return        0 if not EOF, 1 if EOF
 */
int buffer_eof(buf_t *buffer);

/**
 * set the position of the frame within the current file. doesn't refresh buffer.
 * @param  buffer               [description]
 * @param  offset               [description]
 * @param  whence               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_seek(buf_t *buffer, off_t offset, int whence);

/**
 * a more intuitive way of seeking to the start of the file
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rewind(buf_t *buffer);

/**
 * open a file readonly (for rbuf functions)
 * @param  buffer               [description]
 * @param  path                 path to file
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_openr(rbuf_t *buffer, const char *path);

/**
 * load the current frame at the real file offset (modifies file offset, no seek)
 * Acts like a pager
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_readr(rbuf_t *buffer);

/**
 * seeks and refreshes the current frame
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_seekr(rbuf_t *buffer, off_t offset, int whence);

/**
 * rewinds and refreshes the current frame
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rewindr(rbuf_t *buffer);

/**
 * reload the current frame at the current position (doesn't modify file offset)
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_reloadr(rbuf_t *buffer);

/**
 * resets the position and size of the frame
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_resetr(rbuf_t *buffer);

/**
 * open a file writeonly (for wbuf functions)
 * @param  buffer               [description]
 * @param  path                 path to file (doesn't have to exist)
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_openw(wbuf_t *buffer, const char *path);

/**
 * commit the current frame at the real file offset (modifies file offset and st_size, no seek)
 * Acts like a pager
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_writew(wbuf_t *buffer);

#endif
