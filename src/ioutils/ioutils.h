#include <stdio.h>
#include <regex.h>

#ifndef IOUTILS_H
#define IOUTILS_H

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
int ioutils_buf_init(buf_t *buffer);

/* initialize the buffer with compiled defaults */
int ioutils_buf_init_defaults(buf_t *buffer);

/* free the buffer -- this does NOT close any stream */
void ioutils_buf_free(buf_t *buffer);

/* open and load the file into our read buffer */
int ioutils_rbuf_stream_open(buf_t *buffer, const char *pathname);

/* close out the stream -- does NOT refresh or destroy the buffer */
int ioutils_rbuf_stream_close(buf_t *buffer);

/* read the next buffer-full of data into the frame */
int ioutils_rbuf_frame_page(buf_t *buffer);

/* move the read buffer frame elsewhere */
int ioutils_rbuf_frame_seek(buf_t *buffer, long offset, int whence);

/* reload the current frame */
int ioutils_rbuf_frame_reload(buf_t *buffer);

/* rewind the read buffer frame to the beginning of the file */
int ioutils_rbuf_frame_rewind(buf_t *buffer);

/* increase the read buffer frame size */
int ioutils_rbuf_frame_expand(buf_t *buffer);

/* reduce the read buffer frame size */
int ioutils_rbuf_frame_contract(buf_t *buffer);

/******************************************************************************
 * SCANNING FUNCTIONS                                                         *
 ******************************************************************************/

/* Please, be mindful:
 * these scanning regex functions have buffer limitations!
 * Use scan_reg_buffer() if you need to change the buffer parameters at runtime,
 * or override the BUFFER CONTROL constants at compile time.
 */

/* scan stdio with regex */
int scan_reg(const char *regex, size_t nmatch, regmatch_t pmatch[], int cflags,
	     int eflags);

/* scan a stream with regex */
int fscan_reg(FILE *stream, const char *regex, size_t nmatch,
	      regmatch_t pmatch[], int cflags, int eflags);

/* bring your own buffer - make sure you load in the stream! */
int scan_reg_buffer(const char *regex, size_t nmatch, regmatch_t pmatch[],
		    int cflags, int eflags, buf_t *buffer);
#endif
