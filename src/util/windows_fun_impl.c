#include <stddef.h>
#include <stdio.h>
#include <vadefs.h>
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

FILE *open_memstream(void *buffer, size_t *lenght);

FILE *fmemopen(void *buffer, size_t lenght, const char *type)
{
	int filedescriptor;
	char tmpfolder[MAX_PATH - 13];
	char tmpfile[MAX_PATH + 1];
	char tmpfilename[] = "TempFile_";
	if (!GetTempPath2A(sizeof(tmpfolder), tmpfolder))
		return NULL;
	if (!GetTempFileNameA(tmpfolder, tmpfilename, 0, tmpfile))
		return NULL;
	int retner = _sopen_s(&filedescriptor, tmpfile, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT, _SH_DENYRW, _S_IREAD | _S_IWRITE);
	
   // HANDLE handle = CreateFileA(tmpfile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | /*FILE_FLAG_DELETE_ON_CLOSE*/0x04000000, NULL);
   // DWORD written;
   // WriteFile(handle, asmstr, strlen(asmstr), &written, NULL);
   // int filedescriptor = _open_osfhandle((intptr_t)handle,_O_RDONLY);
   
   if (retner != 0) return NULL;
	if (filedescriptor == -1) return NULL;
	FILE * filepointer = _fdopen(filedescriptor, "wb+" /* *type plutot */);
	if (!filepointer) {
		_close(filedescriptor);
		return NULL;
	}

	/*File descriptors passed into _fdopen are owned by the returned FILE * stream.If _fdopen is successful, do not call _close on the file descriptor.Calling fclose on the returned FILE * also closes the file descriptor.*/
	fwrite(buffer, lenght, 1, filepointer);
	rewind(filepointer);
	return filepointer;
}

size_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = (char *)malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = (char *)realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}

void errx(int code, const char * message){
    fprintf(stderr, "%s: select: %s\n", message, strerror(errno));
}

LARGE_INTEGER
getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

int
clock_gettime(int X, struct timeval *tv)
{
    LARGE_INTEGER           t;
    FILETIME            f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        } else {
            offset = getFILETIMEoffset();
            frequencyToMicroseconds = 10.;
        }
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = microseconds;
    tv->tv_sec = t.QuadPart / 1000000;
    tv->tv_usec = t.QuadPart % 1000000;
    return (0);
}

/*
#ifdef _WIN32
#include "util/windows_fun_impl.c"
#endif
*/