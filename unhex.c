#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#define __MSVCRT_VERSION__ 0x0700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include "utils.c"

char hex_convert (const char c)
{
  return (c & 15) + (c >> 6) * 9;
}

char hex_to_u8 (const char hex[2])
{
  char v = 0;

  v |= (hex_convert (hex[1]) << 0);
  v |= (hex_convert (hex[0]) << 4);

  return (v);
}

bool is_valid_hex_char (const char c)
{
  if ((c >= '0') && (c <= '9'))
    return true;
  if ((c >= 'A') && (c <= 'F'))
    return true;
  if ((c >= 'a') && (c <= 'f'))
    return true;

  return false;
}

bool is_valid_hex_string (const char *s, const size_t len)
{
  for (size_t i = 0; i < len; i++)
  {
    const char c = s[i];

    if (is_valid_hex_char (c) == false)
      return false;
  }

  return true;
}

bool is_hexify (const char *buf, const size_t len)
{
  if (len < 6)
    return false;               // $HEX[] = 6

  // length of the hex string must be a multiple of 2
  // and the length of "$HEX[]" is 6 (also an even length)
  // Therefore the overall length must be an even number:

  if ((len & 1) == 1)
    return false;

  if (buf[0] != '$')
    return (false);
  if (buf[1] != 'H')
    return (false);
  if (buf[2] != 'E')
    return (false);
  if (buf[3] != 'X')
    return (false);
  if (buf[4] != '[')
    return (false);
  if (buf[len - 1] != ']')
    return (false);

  if (is_valid_hex_string (buf + 5, len - 6) == false)
    return false;

  return true;
}

size_t unhexify (const char *in_buf, const size_t in_len, char *out_buf, const size_t out_sz)
{
  size_t i, j;

  for (i = 0, j = 5; j < in_len - 1; i += 1, j += 2)
  {
    const char c = hex_to_u8 (&in_buf[j]);

    out_buf[i] = c;
  }

  memset (out_buf + i, 0, out_sz - i);

  return (i);
}

char *trim_nl (char *s, int len)
{
  char *p = s + len - 1;
  char *q = s;

  while (len)
  {
    if (*p != '\n' && *p != '\r')
      break;

    *p-- = 0;

    len--;
  }

  while (len)
  {
    if (*q != '\n' && *q != '\r')
      break;

    q++;

    len--;
  }

  return q;
}

int main (int argc, char *argv[])
{
#ifdef _WINDOWS
  _setmode (_fileno (stdin), _O_BINARY);
#endif

  bool strip_newlines = false;
  if (argc == 2 && !strcmp (argv[1], "-n"))
  {
    strip_newlines = true;
  }

  char line_buf[BUFSIZ];
  char unhex[BUFSIZ];

  int line_len;

  while ((line_len = fgetl (stdin, BUFSIZ, line_buf)) != -1)
  {
    if (line_len == 0)
      continue;

    char *p;
    if (is_hexify (line_buf, line_len))
    {
      const size_t unhex_len = unhexify (line_buf, line_len, unhex, sizeof (unhex));

      unhex[unhex_len] = 0;
      p = unhex;
      line_len = unhex_len;
    }
    else
    {
      p = line_buf;
    }

    if (strip_newlines)
    {
      p = trim_nl (p, line_len);
    }

    puts (p);
  }

  return 0;
}
