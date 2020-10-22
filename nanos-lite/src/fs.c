#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t fbsync_write(const void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    {"stdin", 0, 0, invalid_read, invalid_write},
    {"stdout", 0, 0, invalid_read, serial_write},
    {"stderr", 0, 0, invalid_read, serial_write},
    {"/dev/events", 0, 0, events_read, invalid_write},
    {"/dev/fb", 0, 0, invalid_read, fb_write},
    {"/dev/fbsync", 0, 0, invalid_read, fbsync_write},
    {"/proc/dispinfo", 128, 0, dispinfo_read, invalid_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int fd = fs_open("/dev/fb", 0, 0);
  file_table[fd].size = screen_width() * screen_height() * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
  size_t fd = 3;
  for (; fd < NR_FILES; ++fd) {
    if (pathname != NULL && strcmp(file_table[fd].name, pathname) == 0) {
      file_table[fd].open_offset = 0;
      break;
    }
  }
  if (fd >= NR_FILES) {
    panic("File %s does not exist", pathname);
  }
  return fd;
}

size_t ramdisk_read(void *, size_t, size_t);
size_t ramdisk_write(const void *, size_t, size_t);

size_t fs_read(int fd, void *buf, size_t len) {
  if (file_table[fd].size != 0 &&
      file_table[fd].open_offset + len > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset;
  }
  if (file_table[fd].read) {
    size_t count = file_table[fd].read(buf, file_table[fd].open_offset, len);
    file_table[fd].open_offset += count;
    return count;
  }
  ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
               len);
  file_table[fd].open_offset += len;
  return len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  if (file_table[fd].size != 0 &&
      file_table[fd].open_offset + len > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset;
  }
  if (file_table[fd].write) {
    size_t count = file_table[fd].write(buf, file_table[fd].open_offset, len);
    file_table[fd].open_offset += count;
    return count;
  }
  ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
                len);
  file_table[fd].open_offset += len;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  switch (whence) {
    case SEEK_SET:
      file_table[fd].open_offset = offset;
      break;

    case SEEK_CUR:
      file_table[fd].open_offset += offset;
      break;

    case SEEK_END:
      file_table[fd].open_offset = file_table[fd].size + offset;
      break;

    default:
      panic("Unknown whence: %d", whence);
  }
  return file_table[fd].open_offset;
}

int fs_close(int fd) {
  file_table[fd].open_offset = 0;
  return 0;
}
