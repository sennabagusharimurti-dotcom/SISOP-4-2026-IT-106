# SISOP-4-2026-IT-106
# Laporan Resmi Modul 4 — File System FUSE


# Deskripsi Soal

Pada praktikum ini dibuat sebuah filesystem berbasis FUSE bernama `kenz_rescue.c`.

Filesystem menerima dua argumen:

```bash
./kenz_rescue <source_directory> <mount_directory>
```

Filesystem bekerja sebagai:

* Passthrough filesystem
* Seluruh file pada source directory muncul identik pada mount directory
* Menambahkan virtual file bernama `tujuan.txt`
* Isi `tujuan.txt` dibuat secara on-the-fly dengan menggabungkan seluruh fragment `KOORD:` dari file `1.txt` hingga `7.txt`

---

# Struktur Direktori

```text
.
├── kenz_rescue
├── kenz_rescue.c
├── amba_files/
│   ├── 1.txt
│   ├── 2.txt
│   ├── 3.txt
│   ├── 4.txt
│   ├── 5.txt
│   ├── 6.txt
│   └── 7.txt
└── mnt/
```

---

# Penjelasan Program

## 1. Header dan Library

```c
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
```

Digunakan library FUSE3 untuk membuat userspace filesystem.

---

## 2. Variabel Source Directory

```c
static char source_dir[PATH_MAX];
```

Digunakan untuk menyimpan path source directory.

---

## 3. Function get_fullpath()

```c
static void get_fullpath(char *fpath, const char *path)
{
    strcpy(fpath, source_dir);

    strncat(
        fpath,
        path,
        PATH_MAX - strlen(fpath) - 1
    );
}
```

Function ini digunakan untuk menggabungkan path mount dengan source directory asli.

Contoh:

```text
/1.txt
```

menjadi:

```text
/home/user/amba_files/1.txt
```

---

## 4. Function generate_tujuan()

```c
static void generate_tujuan(char *buffer)
```

Function ini digunakan untuk:

* Membaca file `1.txt` hingga `7.txt`
* Mencari baris yang diawali `KOORD:`
* Menggabungkan seluruh fragment koordinat
* Membentuk isi virtual file `tujuan.txt`

Isi file dibuat secara on-the-fly dan tidak disimpan permanen.

---

## 5. Function getattr

```c
static int xmp_getattr()
```

Digunakan untuk:

* Mengambil metadata file asli
* Membuat metadata virtual file `tujuan.txt`

Jika path adalah `/tujuan.txt`, maka filesystem membuat virtual file.

---

## 6. Function readdir

```c
static int xmp_readdir()
```

Digunakan untuk membaca isi direktori.

Function ini:

* Menampilkan seluruh file asli
* Menambahkan virtual file `tujuan.txt`

---

## 7. Function open

```c
static int xmp_open()
```

Digunakan untuk membuka file.

Pada virtual file `tujuan.txt`, filesystem langsung mengizinkan akses.

---

## 8. Function read

```c
static int xmp_read()
```

Digunakan untuk membaca isi file.

Jika file adalah:

```text
/tujuan.txt
```

maka isi file dibuat menggunakan:

```c
generate_tujuan()
```

Sedangkan file lain dibaca langsung dari source directory.

---

## 9. Function unlink

```c
static int xmp_unlink()
```

Digunakan untuk menghapus file.

Filesystem menolak penghapusan:

```text
tujuan.txt
```

karena merupakan virtual file.

---

## 10. fuse_operations

```c
static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open    = xmp_open,
    .read    = xmp_read,
    .unlink  = xmp_unlink,
};
```

Berisi daftar operasi filesystem yang digunakan oleh FUSE.

---

# Cara Compile

```bash
gcc kenz_rescue.c -o kenz_rescue $(pkg-config fuse3 --cflags --libs)
```

---

# Cara Menjalankan

## Membuat mount directory

```bash
mkdir -p mnt
```

## Menjalankan filesystem

```bash
./kenz_rescue amba_files mnt
```

---

# Testing

## 1. Melihat isi mount directory

```bash
ls mnt
```

Output:

```text
1.txt
2.txt
3.txt
4.txt
5.txt
6.txt
7.txt
tujuan.txt
```

---

## 2. Menguji passthrough filesystem

```bash
for i in 1 2 3 4 5 6 7; do
    diff mnt/$i.txt amba_files/$i.txt && echo "$i.txt OK"
done
```

Output:

```text
1.txt OK
2.txt OK
3.txt OK
4.txt OK
5.txt OK
6.txt OK
7.txt OK
```

Hal ini membuktikan bahwa seluruh file identik dengan source directory.

---

## 3. Membaca virtual file

```bash
cat mnt/tujuan.txt
```

Output berupa gabungan seluruh fragment koordinat.

---

## 4. Menghapus file

```bash
rm mnt/2.txt
```

File asli pada source directory ikut terhapus.

---

# Kendala

## 1. fuse3 tidak ditemukan

Solusi:

```bash
sudo apt install fuse3 libfuse3-dev pkg-config
```

---

## 2. Permission denied saat mount

Solusi:

```bash
sudo chown $USER:$USER mnt
```

atau mengaktifkan:

```text
user_allow_other
```

pada:

```text
/etc/fuse.conf
```

---

## 3. Undefined reference to fuse_main_real

Solusi compile:

```bash
gcc kenz_rescue.c -o kenz_rescue $(pkg-config fuse3 --cflags --libs)
```

---

# Kesimpulan

Pada praktikum ini berhasil dibuat filesystem berbasis FUSE menggunakan bahasa C.

Filesystem mampu:

* Melakukan passthrough filesystem
* Menampilkan file source secara identik
* Membuat virtual file
* Menghasilkan isi file secara dinamis (on-the-fly)
* Mengimplementasikan operasi dasar FUSE seperti:

  * getattr
  * readdir
  * open
  * read
  * unlink

Dengan demikian seluruh requirement soal berhasil dipenuhi.
