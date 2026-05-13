#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

static char source_dir[PATH_MAX];

/* =========================
   FULL PATH
========================= */
static void get_fullpath(char *fpath, const char *path)
{
    strcpy(fpath, source_dir);

    strncat(
        fpath,
        path,
        PATH_MAX - strlen(fpath) - 1
    );
}

/* =========================
   GENERATE tujuan.txt
========================= */
static void generate_tujuan(char *buffer)
{
    buffer[0] = '\0';

    for (int i = 1; i <= 7; i++) {

        char filepath[PATH_MAX];

        strcpy(filepath, source_dir);

        char temp[20];

        sprintf(temp, "/%d.txt", i);

        strncat(
            filepath,
            temp,
            PATH_MAX - strlen(filepath) - 1
        );

        FILE *fp = fopen(filepath, "r");

        if (!fp)
            continue;

        char line[1024];

        while (fgets(line, sizeof(line), fp)) {

            if (strncmp(line, "KOORD:", 6) == 0) {

                char *frag = line + 6;

                while (*frag == ' ')
                    frag++;

                frag[strcspn(frag, "\n")] = 0;

                strcat(buffer, frag);
            }
        }

        fclose(fp);
    }

    strcat(buffer, "\n");
}

/* =========================
   GETATTR
========================= */
static int xmp_getattr(
    const char *path,
    struct stat *stbuf,
    struct fuse_file_info *fi
)
{
    (void) fi;

    memset(stbuf, 0, sizeof(struct stat));

    /* virtual file */
    if (strcmp(path, "/tujuan.txt") == 0) {

        char content[4096];

        generate_tujuan(content);

        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(content);

        return 0;
    }

    char fpath[PATH_MAX];

    get_fullpath(fpath, path);

    if (lstat(fpath, stbuf) == -1)
        return -errno;

    return 0;
}

/* =========================
   READDIR
========================= */
static int xmp_readdir(
    const char *path,
    void *buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info *fi,
    enum fuse_readdir_flags flags
)
{
    (void) offset;
    (void) fi;
    (void) flags;

    char fpath[PATH_MAX];

    get_fullpath(fpath, path);

    DIR *dp = opendir(fpath);

    if (dp == NULL)
        return -errno;

    struct dirent *de;

    while ((de = readdir(dp)) != NULL) {

        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (filler(buf, de->d_name, &st, 0, 0))
            break;
    }

    closedir(dp);

    /* tambah virtual file */
    if (strcmp(path, "/") == 0) {

        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_mode = S_IFREG | 0644;
        st.st_nlink = 1;

        filler(buf, "tujuan.txt", &st, 0, 0);
    }

    return 0;
}

/* =========================
   OPEN
========================= */
static int xmp_open(
    const char *path,
    struct fuse_file_info *fi
)
{
    /* virtual file */
    if (strcmp(path, "/tujuan.txt") == 0)
        return 0;

    char fpath[PATH_MAX];

    get_fullpath(fpath, path);

    int fd = open(fpath, fi->flags);

    if (fd == -1)
        return -errno;

    close(fd);

    return 0;
}

/* =========================
   READ
========================= */
static int xmp_read(
    const char *path,
    char *buf,
    size_t size,
    off_t offset,
    struct fuse_file_info *fi
)
{
    (void) fi;

    /* virtual file */
    if (strcmp(path, "/tujuan.txt") == 0) {

        char content[4096];

        generate_tujuan(content);

        size_t len = strlen(content);

        if (offset < len) {

            if (offset + size > len)
                size = len - offset;

            memcpy(buf, content + offset, size);

        } else {
            size = 0;
        }

        return size;
    }

    char fpath[PATH_MAX];

    get_fullpath(fpath, path);

    int fd = open(fpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);

    if (res == -1)
        res = -errno;

    close(fd);

    return res;
}

/* =========================
   UNLINK
========================= */
static int xmp_unlink(const char *path)
{
    /* cegah hapus virtual file */
    if (strcmp(path, "/tujuan.txt") == 0)
        return -EACCES;

    char fpath[PATH_MAX];

    get_fullpath(fpath, path);

    int res = unlink(fpath);

    if (res == -1)
        return -errno;

    return 0;
}

/* =========================
   OPERATIONS
========================= */
static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open    = xmp_open,
    .read    = xmp_read,
    .unlink  = xmp_unlink,
};

/* =========================
   MAIN
========================= */
int main(int argc, char *argv[])
{
    if (argc < 3) {

        fprintf(
            stderr,
            "Usage: %s <source_dir> <mount_dir>\n",
            argv[0]
        );

        return 1;
    }

    realpath(argv[1], source_dir);

    char *fuse_argv[2];

    fuse_argv[0] = argv[0];
    fuse_argv[1] = argv[2];

    return fuse_main(
        2,
        fuse_argv,
        &xmp_oper,
        NULL
    );
}
