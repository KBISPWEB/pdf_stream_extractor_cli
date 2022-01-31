#include <config.h>

#include <stdio.h>

#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer {
	void *ptr; /* the working buffer */
	size_t size; /* size of each buffer element */
	size_t nmemb; /* number of elements in buffer */
	FILE *stream; /* the current working stream */

	size_t nmemb_e; /* number of elements to expand/contract by */
	size_t nmemb_m; /* maximum number of elements we can have */

	fpos_t pos;
} buf_t;

/******************************************************************************
 * BUFFER FUNCTIONS                                                           *
 ******************************************************************************/

/* initialize the buffer with structure-defined values */
int buffer_buf_init(buf_t *buffer);

/* initialize the buffer with compiled defaults */
int buffer_buf_init_defaults(buf_t *buffer);

/* free the buffer -- this does NOT close any stream */
void buffer_buf_free(buf_t *buffer);

/* open and load the file into our read buffer */
int buffer_rbuf_stream_open(buf_t *buffer, const char *pathname);

/* close out the stream -- does NOT refresh or destroy the buffer */
int buffer_rbuf_stream_close(buf_t *buffer);

/* read the next buffer-full of data into the frame */
int buffer_rbuf_frame_page(buf_t *buffer);

/* move the read buffer frame elsewhere */
int buffer_rbuf_frame_seek(buf_t *buffer, long offset, int whence);

/* reload the current frame */
int buffer_rbuf_frame_reload(buf_t *buffer);

/* rewind the read buffer frame to the beginning of the file */
int buffer_rbuf_frame_rewind(buf_t *buffer);

/* increase the read buffer frame size */
int buffer_rbuf_frame_expand(buf_t *buffer);

/* reduce the read buffer frame size */
int buffer_rbuf_frame_contract(buf_t *buffer);

#endif
