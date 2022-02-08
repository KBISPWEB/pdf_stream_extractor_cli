#include <config.h>

#include <stdlib.h>
#include <string.h> /* string and memory stuff */

#include "stream_scanner.h"

const void *memmem(const void *s1, const void *s2, size_t n, const size_t size)
{
	size_t offset = 0;
	void *sp;

	while (memcmp(s1 + offset, s2, n) != 0) {
		if ((sp = memchr(s1 + offset + 1, *((char *)s2), n - 1)) !=
		    NULL) {
			offset = sp - s1;
		} else {
			offset += n;
		}
		if ((offset + n) > size)
			return NULL;
	}

	return s1 + offset;
}

/* 0 on OK, 1 on not found, -1 on failure */
int buffer_find_mem(buffer_t buffer, const char *substr, const size_t size,
		    off_t *offset, int reset)
{
	const void *sp;

	const off_t original_offset = buffer_get_filepos(buffer);
	const off_t filesize = buffer_get_filesize(buffer);
	const void *bufptr = buffer_get_bufptr(buffer);
	const size_t bufsize = buffer_get_bufsize(buffer);

	int ret = 1;

	*offset = original_offset;

	/* don't even bother if there's no data */
	if (*offset > filesize)
		return 0;

	if (buffer_reload(buffer) <= 0) {
		fprintf(stderr, "error: buffer_find_mem: buffer_reload\n");
		return -1;
	}

	while ((sp = memmem(bufptr, substr, size, bufsize)) == NULL) {
		/* advance as much as we can */
		if ((*offset = buffer_seek(buffer, bufsize - size, SEEK_CUR)) ==
		    -1) {
			reset = 1;
			ret = -1;
			goto die;
		}

		if (buffer_read(buffer) <= 0) {
			reset = 1;
			ret = -1;
			goto die;
		}

		/* went past EOF, so it's not in the current file. */
		if (*offset > filesize) {
			reset = 1;
			ret = 0;
			goto die;
		}
	}

	/* found what we were looking for */
	*offset += (sp - bufptr);

die:
	if (reset) {
		if (buffer_seek(buffer, original_offset, SEEK_SET) == -1)
			return -1;
	} else {
		if (buffer_seek(buffer, *offset, SEEK_SET) == -1)
			return -1;
	}

	if (buffer_read(buffer) <= 0)
		return -1;

	return ret;
}

/* 1 on OK, 0 on not found, -1 on failure */
int get_obj(buffer_t buffer, size_t *objlen)
{
	off_t objstart = 0, objend = 0;
	int ret;

	/* find an obj line... */
	if ((ret = buffer_find_mem(buffer, "obj", 3, &objstart, 0)) <= 0)
		return ret;

	/* ...get the start of it... */
	if ((ret = buffer_find_mem(buffer, "<<", 2, &objstart, 0)) <= 0)
		return ret;

	objstart += 2;

	/* ...get the end of it... */
	if ((ret = buffer_find_mem(buffer, "endobj", 6, &objend, 1)) <= 0)
		return ret;

	*objlen = (objend - objstart);

	// TODO: fix segfault.
	if (*objlen < buffer_get_bufsize(buffer))
		*((char *)buffer_get_bufptr(buffer) + *objlen) = 0;

	return 1;
}
