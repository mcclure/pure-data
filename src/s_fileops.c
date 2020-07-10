/* Copyright (c) 2020 Andi McClure.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "s_fileops.h"

#ifndef _PD_METAFILE_NO_DEFAULT

#include <fcntl.h>

t_fileops sys_fileops;

// TODO delete this?
#ifdef _LARGEFILE64_SOURCE
# define open  open64
# define lseek lseek64
# define fstat fstat64
# define stat  stat64
#endif


    /* open a file with a UTF-8 filename
    This is needed because WIN32 does not support UTF-8 filenames, only UCS2.
    Having this function prevents lots of #ifdefs all over the place.
    */
#ifdef _WIN32
static int sys_open(const char *path, int oflag, ...)
{
    int i, fd;
    char pathbuf[MAXPDSTRING];
    wchar_t ucs2path[MAXPDSTRING];
    sys_bashfilename(path, pathbuf);
    u8_utf8toucs2(ucs2path, MAXPDSTRING, pathbuf, MAXPDSTRING-1);
    /* For the create mode, Win32 does not have the same possibilities,
     * so we ignore the argument and just hard-code read/write. */
    if (oflag & O_CREAT)
        fd = _wopen(ucs2path, oflag | O_BINARY, _S_IREAD | _S_IWRITE);
    else
        fd = _wopen(ucs2path, oflag | O_BINARY);
    return fd;
}

static FILE *sys_fopen(const char *filename, const char *mode)
{
    char namebuf[MAXPDSTRING];
    wchar_t ucs2buf[MAXPDSTRING];
    wchar_t ucs2mode[MAXPDSTRING];
    sys_bashfilename(filename, namebuf);
    u8_utf8toucs2(ucs2buf, MAXPDSTRING, namebuf, MAXPDSTRING-1);
    /* mode only uses ASCII, so no need for a full conversion, just copy it */
    mbstowcs(ucs2mode, mode, MAXPDSTRING);
    return (_wfopen(ucs2buf, ucs2mode));
}
#else
#include <stdarg.h>
static int sys_open(const char *path, int oflag, ...)
{
    int i, fd;
    char pathbuf[MAXPDSTRING];
    sys_bashfilename(path, pathbuf);
    if (oflag & O_CREAT)
    {
        mode_t mode;
        int imode;
        va_list ap;
        va_start(ap, oflag);

        /* Mac compiler complains if we just set mode = va_arg ... so, even
        though we all know it's just an int, we explicitly va_arg to an int
        and then convert.
           -> http://www.mail-archive.com/bug-gnulib@gnu.org/msg14212.html
           -> http://bugs.debian.org/647345
        */

        imode = va_arg (ap, int);
        mode = (mode_t)imode;
        va_end(ap);
        fd = open(pathbuf, oflag, mode);
    }
    else
        fd = open(pathbuf, oflag);
    return fd;
}

#if 0
FILE *sys_fopen(const char *filename, const char *mode)
{
  char namebuf[MAXPDSTRING];
  sys_bashfilename(filename, namebuf);
  return fopen(namebuf, mode);
}
#endif
#endif /* _WIN32 */

   /* close a previously opened file
   this is needed on platforms where you cannot open/close resources
   across dll-boundaries, but we provide it for other platforms as well */
static int sys_close(int fd)
{
#ifdef _WIN32
    return _close(fd);  /* Bill Gates is a big fat hen */
#else
    return close(fd);
#endif
}

#if 0
int sys_fclose(FILE *stream)
{
    return fclose(stream);
}
#endif

#endif