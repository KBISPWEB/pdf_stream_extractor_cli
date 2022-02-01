#include <config.h>

#include <stdio.h>

#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer {
	void *ptr; /* the working buffer */
	size_t size; /* size of the buffer in bytes */
	size_t size_e; /* number of bytes to expand/contract the buffer by */

	int filedes; /* the current working stream */
	off_t st_size; /* the total size of the file */

	off_t actual_pos; /* the real offset in the file */
	off_t pos; /* the current position of the frame (might not be the actual offset) */
} buf_t;

/**
 * initialize internal buffer based on structure's size parameter
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_buf_init(buf_t *buffer);

/**
 * initialize buf_t compile-time defaults
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_buf_init_defaults(buf_t *buffer);

/**
 * free internal buffer
 * @param buffer  [description]
 */
void buffer_buf_free(buf_t *buffer);

/**
 * close a previously opened file
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_buf_close(buf_t *buffer);

/**
 * set the position of the frame within the current file. doesn't refresh buffer.
 * @param  buffer               [description]
 * @param  offset               [description]
 * @param  whence               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_buf_frame_seek(buf_t *buffer, off_t offset, int whence);

/**
 * a more intuitive way of seeking to the start of the file
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_buf_frame_rewind(buf_t *buffer);

/**
 * open a file readonly (for rbuf functions)
 * @param  buffer               [description]
 * @param  path                 path to file
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_open(buf_t *buffer, const char *path);

/**
 * load the current frame at the real file offset (modifies file offset, no seek)
 * Acts like a pager
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_frame_load(buf_t *buffer);

/**
 * seeks and refreshes the current frame
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_frame_seek(buf_t *buffer, off_t offset, int whence);

/**
 * rewinds and refreshes the current frame
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_frame_rewind(buf_t *buffer);

/**
 * reload the current frame at the current position (doesn't modify file offset)
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_frame_reload(buf_t *buffer);

/**
 * expand the current frame by the expansion value in the struct
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_frame_expand(buf_t *buffer);

/**
 * contracts the current frame by the expansion value in the struct
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_frame_contract(buf_t *buffer);

/**
 * resets the position and size of the frame
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_rbuf_frame_reset(buf_t *buffer);

#endif
