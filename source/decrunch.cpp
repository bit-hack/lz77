#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "../liblz/lz.h"

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

int main(const int argc, const char **args) {

  if (argc < 2) {
    printf("usage: in_file [out_file]\n");
    return 1;
  }

  const std::string out_file =
      (argc > 2) ? args[1] : (std::string(args[1]) + ".dec");

  file_t f;
  if (!read(args[1], f)) {
    printf("unable to open file: '%s'\n", args[1]);
    return 1;
  }

  FILE *fd = fopen(out_file.c_str(), "wb");
  if (!fd) {
    printf("unable to open file: '%s'\n", out_file.c_str());
    return 1;
  }

  // decode mode
  size_t written = 0;
  const uint8_t *data = (uint8_t *)decode(f._data, f._size, &written);
  if (fwrite(data, 1, written, fd) != written) {
    printf("error writing to file\n");
    return 1;
  }

  // status
  printf("%d in %d out\n", int(f._size), int(written));

  fclose(fd);
  return 0;
}
