/* Copyright (c) 2020 Andi McClure.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "s_fileops.h"

static bool null_open(const char *path, t_fileops_flags flags, t_fileops_handle *handle) {
    return false;
}
static bool null_close(t_fileops_handle handle) {
    return false;
}
static bool null_stat(t_fileops_handle handle, t_fileops_stat *stat) {
    return false;
}
static int64_t null_seek(t_fileops_handle handle, int64_t offset, t_fileops_flags flags) {
    return 0;
}
static ssize_t null_read(t_fileops_handle handle, const void *buf, size_t nbyte) {
    return 0;
}
static ssize_t null_write(t_fileops_handle handle, const void *buf, size_t nbyte) {
    return 0;
}
static ssize_t null_scanf(t_fileops_handle handle, const char * restrict format, ...) {
    return 0;
}
static ssize_t null_vscanf(t_fileops_handle handle, const char * restrict format, va_list ap) {
    return 0;
}
static ssize_t null_printf(t_fileops_handle handle, const char * restrict format, ...) {
    return 0;
}
static ssize_t null_vprintf(t_fileops_handle handle, const char * restrict format, va_list ap) {
    return 0;
}
static bool null_flush(t_fileops_handle handle) {
    return false;
}

t_fileops sys_fileops_null = {null_open, null_close, null_stat, null_seek, null_read, null_write, null_scanf, null_vscanf, null_printf, null_vprintf};

#ifndef _PD_METAFILE_NO_DEFAULT

#include <fcntl.h>

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

static bool open(const char *path, t_fileops_flags flags, t_fileops_handle *handle) {
    char mode[3];
    int midx = 0;
    if (flags & FILEOPS_READ)
        mode[midx++] = 'r';
    if (flags & FILEOPS_WRITE)
        mode[midx++] = 'w';
    if (!(flags & FILEOPS_CREAT))
        mode[midx++] = 'x';
    mode[midx++] = '\0';

// IF BODY PD CODE
#ifdef _WIN32
    char namebuf[MAXPDSTRING];
    wchar_t ucs2buf[MAXPDSTRING];
    wchar_t ucs2mode[MAXPDSTRING];
    sys_bashfilename(filename, namebuf);
    u8_utf8toucs2(ucs2buf, MAXPDSTRING, namebuf, MAXPDSTRING-1);
    /* mode only uses ASCII, so no need for a full conversion, just copy it */
    mbstowcs(ucs2mode, mode, MAXPDSTRING);
    FILE * f = _wfopen(ucs2buf, ucs2mode);
#else
  char namebuf[MAXPDSTRING];
  sys_bashfilename(filename, namebuf);
  FILE *f = fopen(namebuf, mode);
#endif
  *handle = (intptr_t)(void *)f;
  return tobool(f);
}
static bool close(t_fileops_handle handle) {
    FILE *f = (FILE *)(void *)handle;
    int result = fclose(f);
    return 0 == result;
}
static bool stat(t_fileops_handle handle, t_fileops_stat *stat) {

}
static int64_t seek(t_fileops_handle handle, int64_t offset, t_fileops_flags flags) {

}
static ssize_t read(t_fileops_handle handle, const void *buf, size_t nbyte) {

}
static ssize_t write(t_fileops_handle handle, const void *buf, size_t nbyte) {

}
static ssize_t scanf(t_fileops_handle handle, const char * restrict format, ...) {

}
static ssize_t vscanf(t_fileops_handle handle, const char * restrict format, va_list ap) {

}
static ssize_t printf(t_fileops_handle handle, const char * restrict format, ...) {

}
static ssize_t vprintf(t_fileops_handle handle, const char * restrict format, va_list ap) {

}
static bool flush(t_fileops_handle handle) {

}

t_fileops sys_fileops_standard = {std_open, std_close, std_stat, std_seek, std_read, std_write, std_scanf, std_vscanf, std_printf, std_vprintf};

t_fileops sys_fileops = sys_fileops_standard;

#else

t_fileops sys_fileops = sys_fileops_null;

#endif