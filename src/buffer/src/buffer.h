#include <config.h>

#include <stdio.h>

/* O_RDONLY, O_WRONLY, O_RDWR, etc. */
#include <fcntl.h>

#if defined(OS_WINDOWS)
#elif defined(OS_LINUX)
#define _O_RDONLY O_RDONLY
#define _O_RDWR O_RDWR
#define _O_WRONLY O_WRONLY
#else
#error "This library will not build."
#endif

#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer *buffer_t;

/**
 * construct a new buffer structure to be "nothing"
 * @param buffer  [description]
 */
buffer_t buffer_init();

void buffer_free(buffer_t buffer);

/**
 * resize internal buffer
 * @param  buffer               [description]
 * @param  size                 [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_resize(buffer_t buffer, size_t size);

int buffer_get_filedes(buffer_t buffer);
off_t buffer_get_filesize(buffer_t buffer);
void *buffer_get_bufptr(buffer_t buffer);
size_t buffer_get_bufsize(buffer_t buffer);
int buffer_set_datalength(buffer_t buffer, size_t datalength);
size_t buffer_get_datalength(buffer_t buffer);

/**
 * open a file readonly (for rbuf functions)
 * @param  buffer               [description]
 * @param  path                 path to file
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_open(buffer_t buffer, const char *path, int oflag);

/**
 * close a previously opened file
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_close(buffer_t buffer);

/**
 * get position in current file
 * @param  buffer               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
off_t buffer_get_offset(buffer_t buffer);

/**
 * set the position of the frame within the current file. doesn't refresh buffer.
 * @param  buffer               [description]
 * @param  offset               [description]
 * @param  whence               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
int buffer_seek(buffer_t buffer, off_t offset, int whence);

/**
 * a more intuitive way of seeking to the start of the file
 * @param  buffer               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
int buffer_rewind(buffer_t buffer);

/**
 * reload the current frame at the current position (doesn't modify file offset)
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_reload(buffer_t buffer);

/**
 * commit the current frame at the real file offset (modifies file offset and st_size, no seek)
 * Acts like a pager
 * @param  buffer               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int buffer_write(buffer_t buffer);

#endif
