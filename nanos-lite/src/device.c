#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  _yield();
  for (size_t i = 0; i < len; ++i) {
    _putc(((uint8_t *)buf)[i]);
  }
  return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  _yield();
  int key = read_key();
  if (key != _KEY_NONE) {
    int down = 0;
    if (key & 0x8000) {
      key ^= 0x8000;
      down = 1;
    }
    return sprintf(buf, "k%c %s\n", down ? 'd' : 'u', keyname[key]);
  } else {
    return sprintf(buf, "t %d\n", uptime());
  }
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return snprintf(buf, len, "%s", dispinfo + offset);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  _yield();
  offset /= 4;
  draw_rect((uint32_t *)buf, offset % screen_width(), offset / screen_width(),
            len / 4, 1);
  return len;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
  draw_sync();
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
}
