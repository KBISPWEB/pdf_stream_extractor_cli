#ifndef STREAM_SCANNER_H
#define STREAM_SCANNER_H

#include <buffer.h>

/**
 * get pointers to objstream and relevant data.
 * @param  buffer                  [description]
 * @param  objlen                  OUT. object length, object has been null-terminated.
 * @return           1 if stream is found, 0 if stream isn't found, -1 on failure.
 */
int get_stream(buffer_t buffer, size_t *objlen);

int buffer_find_mem(buffer_t buffer, const char *substr, const size_t size,
		    off_t *offset, int reset);

#endif
