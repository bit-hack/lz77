#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lz.h"

struct file_t {
  uint8_t *_data;
  size_t _size;
};

bool read(const char *path, file_t &f) {
  FILE *fd = fopen(path, "rb");
  fseek(fd, 0, SEEK_END);
  f._size = ftell(fd);
  f._data = (uint8_t *)malloc(f._size);
  if (!f._data) {
    return false;
  }
  rewind(fd);
  fread(f._data, 1, f._size, fd);
  fclose(fd);
  return true;
}

uint32_t rand32() { return rand() ^ (rand() << 12); }

int main(const int argc, const char **args) {

  if (argc < 4) {
    printf("usage: [d/e] in_file out_file\n");
    return 1;
  }

  file_t f;
  if (!read(args[2], f)) {
    printf("unable to open file: '%s'", args[2]);
    return 1;
  }

  FILE *fd = fopen(args[3], "wb");
  if (!fd) {
    printf("unable to open file: '%s'", args[3]);
    return 1;
  }

  const char mode = *args[1];

  // encode mode
  if (mode == 'e') {
    uint8_t *mem = (uint8_t *)malloc(f._size * 2);
    assert(mem);
    size_t written = 0;
    encode(f._data, mem, f._size, f._size * 2, &written);
    fwrite(mem, 1, written, fd);
  }

  // decode mode
  if (mode == 'd') {
    size_t written = 0;
    const uint8_t *data = (uint8_t *)decode(f._data, f._size, &written);
    fwrite(data, 1, written, fd);
  }

  fclose(fd);
  return 0;
}
