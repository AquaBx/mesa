#include <sys/stat.h>

struct FTW {
    int base;
    int level;
};

int nftw(const char *dirpath,
        int (*fn) (const char *fpath, const struct stat *sb,
                   int typeflag, struct FTW *ftwbuf),
        int nopenfd, int flags);