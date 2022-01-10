#include <errno.h>
#include <stdlib.h>

#include "ioutils.h"

/******************************************************************************
 * BUFFER CONTROL                                                             *
 ******************************************************************************/

/* defaults - you can override these when building this library */
#ifndef IOUTILS_BUFFER_SIZE
/* size of data - default 1 byte */
#define IOUTILS_BUFFER_SIZE 1
#endif

#ifndef IOUTILS_BUFFER_CHUNK
/* size of chunk - default 1KiB */
#define IOUTILS_BUFFER_CHUNK 1024
#endif

#ifndef IOUTILS_BUFFER_INITIAL
/* initial number of chunks - default 1 (1KiB) */
#define IOUTILS_BUFFER_INITIAL 1
#endif

#ifndef IOUTILS_BUFFER_MAX
/* maximum number of chunks - default 1024 (1MiB) */
#define IOUTILS_BUFFER_MAX 1024
#endif

/******************************************************************************
 * BUFFER FUNCTIONS                                                           *
 ******************************************************************************/

int ioutils_buf_init(buf_t *buffer)
{
	void *tmp;
	size_t size = buffer->nmemb * buffer->size;

	tmp = realloc(buffer->ptr, size);

	if ((tmp == NULL) && (size != 0)) {
		errno = ENOMEM;
		return -1;
	}

	buffer->ptr = tmp;

	return 0;
}

int ioutils_buf_init_defaults(buf_t *buffer)
{
	buffer->size = IOUTILS_BUFFER_SIZE;
	buffer->nmemb = IOUTILS_BUFFER_INITIAL * IOUTILS_BUFFER_CHUNK;
	buffer->nmemb_e = IOUTILS_BUFFER_CHUNK;
	buffer->nmemb_m = IOUTILS_BUFFER_MAX;

	buffer->stream = NULL;

	return ioutils_buf_init(buffer);
}

void ioutils_buf_free(buf_t *buffer)
{
	free(buffer->ptr);
	buffer->nmemb = 0;
}

int ioutils_rbuf_stream_open(buf_t *buffer, const char *pathname)
{
	if ((buffer->stream = fopen(pathname, "rb")) == NULL)
		return -1;

	return ioutils_rbuf_frame_rewind(buffer);
}

int ioutils_rbuf_stream_close(buf_t *buffer)
{
	return fclose(buffer->stream);
}

int ioutils_rbuf_frame_readnext(buf_t *buffer)
{
	if (fread(buffer->ptr, buffer->size, buffer->nmemb, buffer->stream) !=
	    buffer->nmemb) {
		if (ferror(buffer->stream)) {
			errno = EIO;
			return -1;
		}
	}

	return 0;
}

int ioutils_rbuf_frame_page(buf_t *buffer)
{
	if (fgetpos(buffer->stream, &(buffer->pos)))
		return -1;

	return ioutils_rbuf_frame_readnext(buffer);
}

int ioutils_rbuf_frame_seek(buf_t *buffer, long offset, int whence)
{
	if (fsetpos(buffer->stream, &(buffer->pos)))
		return -1;

	if (fseek(buffer->stream, offset, whence))
		return -1;

	return ioutils_rbuf_frame_readnext(buffer);
}

int ioutils_rbuf_frame_reload(buf_t *buffer)
{
	if (fsetpos(buffer->stream, &(buffer->pos)))
		return -1;

	return ioutils_rbuf_frame_readnext(buffer);
}

int ioutils_rbuf_frame_rewind(buf_t *buffer)
{
	rewind(buffer->stream);

	if (fgetpos(buffer->stream, &(buffer->pos)))
		return -1;

	return ioutils_rbuf_frame_reload(buffer);
}

int ioutils_rbuf_frame_expand(buf_t *buffer)
{
	int ret = 0;
	size_t new_nmemb;

	new_nmemb = buffer->nmemb + buffer->nmemb_e;

	/* only allowed to increase in size */
	if (new_nmemb < buffer->nmemb) {
		errno = EOVERFLOW;
		return -1;
	}

	if (new_nmemb > buffer->nmemb_m) {
		errno = ERANGE;
		return -1;
	}

	buffer->nmemb = new_nmemb;

	if ((ret = ioutils_buf_init(buffer)))
		return ret;

	if ((ret = ioutils_rbuf_frame_reload(buffer)))
		return ret;

	return 0;
}

int ioutils_rbuf_frame_contract(buf_t *buffer)
{
	int ret = 0;

	size_t new_nmemb;

	new_nmemb = buffer->nmemb - buffer->nmemb_e;

	/* only allowed to decrease in size */
	if (new_nmemb > buffer->nmemb) {
		errno = EOVERFLOW;
		return -1;
	}

	buffer->nmemb = new_nmemb;

	if ((ret = ioutils_buf_init(buffer))) /* initialize pointers */
		return ret;

	if ((ret = ioutils_rbuf_frame_reload(buffer))) /* reload from stream */
		return ret;

	return 0;
}

/******************************************************************************
 * SCANNING FUNCTIONS                                                         *
 ******************************************************************************/

int scan_reg(regex_t *preg, size_t nmatch, regmatch_t pmatch[], int eflags)
{
	return fscan_reg(stdin, preg, nmatch, pmatch, eflags);
}

int fscan_reg(FILE *stream, regex_t *preg, size_t nmatch, regmatch_t pmatch[],
	      int eflags)
{
	int ret;
	buf_t buffer = { 0 };

	ret = fscan_reg_buffer(stream, preg, nmatch, pmatch, eflags, &buffer);

	ioutils_buf_free(&buffer);

	return ret;
}

int scan_reg_buffer(regex_t *preg, size_t nmatch, regmatch_t pmatch[],
		    int eflags, buf_t *buffer)
{
	return fscan_reg_buffer(stdin, preg, nmatch, pmatch, eflags, buffer);
}

int fscan_reg_buffer(FILE *stream, regex_t *preg, size_t nmatch,
		     regmatch_t pmatch[], int eflags, buf_t *buffer)
{
	/* resize data frame until we find a match or run out of space */
	/* regexec(preg, string, nmatch, pmatch, eflags); */

	// TODO
}
