#ifndef __strbuf_h
#define __strbuf_h

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct _strbuf {
	char *buf;
	char *end;
	char *ptr;
} strbuf_t;

static __inline
void strbuf_init(char *buffer, size_t max_buffer, strbuf_t *strbuf)
{
	strbuf->buf = strbuf->ptr = buffer;
	strbuf->end = strbuf->ptr + max_buffer;
}

static __inline
int strbuf_append_char(strbuf_t *strbuf, char ch)
{
	if (strbuf->ptr == strbuf->end)
		return (-1);

	*(strbuf->ptr) = ch;
	strbuf->ptr++;

	return (0);
}

static __inline
int strbuf_terminate(strbuf_t *strbuf)
{
	if (strbuf->ptr == strbuf->end)
		return (-1);

	*(strbuf->ptr) = 0;

	return (0);
}

static __inline
int strbuf_append(strbuf_t *strbuf, char const *string)
{
	int len;

	if (strbuf->ptr == strbuf->end)
		return (-1);

	len = strlen(string);
	if (len >= strbuf->end - strbuf->ptr)
	  len = strbuf->end - strbuf->ptr;

	strcpy(strbuf->ptr, string);
	strbuf->ptr += len;

	return (0);
}

static __inline
int strbuf_append_formatv(strbuf_t *strbuf, char const *format, va_list ap)
{
	if (strbuf->ptr == strbuf->end)
		return (-1);

	strbuf->ptr += vsnprintf(strbuf->ptr, strbuf->end - strbuf->ptr, format, ap);
	return (0);
}

static __inline
int strbuf_append_format(strbuf_t *strbuf, char const *format, ...)
{
	int     rc;
	va_list ap;

	va_start (ap, format);
	rc = strbuf_append_formatv(strbuf, format, ap);
	va_end (ap);

	return (rc);
}

static __inline
size_t strbuf_length(strbuf_t const *strbuf)
{
  return (strbuf->ptr - strbuf->buf);
}

#endif  /* !__strbuf_h */
