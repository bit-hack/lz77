#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

struct entry_t {
  bool match;
  // lz match
  uint32_t offset, length;
  // raw data
  uint8_t data;
};

struct match_t {
  const uint8_t *pos;
  uint32_t len;
};

// return next power of two
static size_t _npot(size_t size) {
  size -= size ? 1 : 0;
  size |= size >> 16;
  size |= size >> 8;
  size |= size >> 4;
  size |= size >> 2;
  size |= size >> 1;
  return size + 1;
}

// write variable length quantity
static void _vlq_write(uint8_t *&out, uint32_t val) {
  std::array<uint8_t, 16> data;
  int32_t i = 0;
  for (; val; ++i) {
    data[i] = val & 0x7f;
    val >>= 7;
  }
  do {
    --i;
    *(out++) = data[i] | (i ? 0x80 : 0x00);
  } while (i > 0);
}

// read variable length quantity
static uint32_t _vlq_read(const uint8_t *&out) {
  uint32_t val = 0;
  for (;;) {
    const uint8_t c = *(out++);
    val = (val << 7) | (c & 0x7f);
    if ((c & 0x80) == 0) {
      break;
    }
  }
  return val;
}

// find longest match in window
static match_t _match(const uint8_t *start, const uint8_t *end,
                      const uint8_t *data, const uint8_t *data_end) {

  // XXX: this could very quickly become a bottle neck and requires
  //      some acceleration structure I feel.

  match_t out = {nullptr, 0};
  for (const uint8_t *x = start; x < end; ++x) {
    const uint32_t mod = (end - x);
    // keep matching as much as we can
    for (uint32_t i = 0;; ++i) {
      // note: we can modulo 'x[i]' for a kind of RLE encoding
      if (x[i % mod] != data[i])
        break;
      if (data + i >= data_end)
        break;
      if (i >= out.len) {
        out.len = i + 1;
        out.pos = x;
      }
    }
  }
  return out;
}

// flush the entry list
static void _flush_list(std::vector<entry_t> &list, uint8_t *&dst,
                        const uint8_t *end) {
  int32_t count = 0;
  int32_t size = list.size();
  if (list.front().match) {
    for (const auto &i : list) {
      if (count == 0) {
        count = std::min<int>(0x7f, size);
        *(dst++) = uint8_t(count) | 0x80;
        size -= count;
      }
      _vlq_write(dst, i.offset);
      _vlq_write(dst, i.length);
      --count;
    }
  } else {
    for (const auto &i : list) {
      if (count == 0) {
        count = std::min<int>(0x7f, size);
        *(dst++) = uint8_t(count) | 0x00;
        size -= count;
      }
      *(dst++) = i.data;
      --count;
    }
  }
  list.clear();
}

// push an entry into the entry list flushing if needed
static void _push_entry(std::vector<entry_t> &list, const entry_t &entry,
                        uint8_t *&dst, const uint8_t *end) {
  if (list.empty() || (list.front().match == entry.match)) {
    list.emplace_back(entry);
    return;
  }
  _flush_list(list, dst, end);
  list.emplace_back(entry);
}

void encode(const uint8_t *src, uint8_t *dst, size_t src_size, size_t dst_size,
            size_t *written) {

  // vector of output entries
  std::vector<entry_t> entry;
  const uint8_t *end = dst + dst_size;
  const uint8_t *base = dst;

  // parse input stream into entries vector
  for (const uint8_t *base = src; size_t(src - base) < src_size;) {
    // find max match in 64k window
    const uint8_t *match_start = std::max(base, src - 0xffff);
    // find longest match in dictionary
    const match_t m = _match(match_start, src, src, base + src_size);
    // match offset from current write location
    const uint32_t offset = m.pos ? src - m.pos : 0;
    // if we have a profitable match
    // note: remember it is costly to split runs
    if (m.len >= 4) {
      _push_entry(entry, entry_t{true, offset, m.len, 0}, dst, end);
      // increment source pointer
      src += m.len;
    } else {
      _push_entry(entry, entry_t{false, 0, 0, *src}, dst, end);
      // increment source pointer
      ++src;
    }
  }
  _flush_list(entry, dst, end);
  // number of bytes written
  if (written) {
    *written = dst - base;
  }
}

void decode(const uint8_t *src, uint8_t *dst, size_t src_size, size_t dst_size,
            size_t *out_size) {
  const uint8_t *base = dst;
  const uint8_t *end = src + src_size;
  while (src < end) {
    const uint8_t marker = *(src++);
    const uint8_t count = marker & 0x7f;
    if (marker & 0x80) {
      for (uint32_t i = 0; i < count; ++i) {
        const uint32_t offset = _vlq_read(src);
        const uint32_t length = _vlq_read(src);
        assert(dst - offset >= base);
        for (uint32_t j = 0; j < length; ++j) {
          dst[j] = dst[j - offset];
        }
        dst += length;
      }
    } else {
      for (uint32_t i = 0; i < count; ++i) {
        *(dst++) = *(src++);
      }
    }
  }
}

// decode into a dynamicaly allocated memory buffer
void *decode(const uint8_t *src, size_t src_size, size_t *out_size) {
  // initial allocation
  size_t size = 256;
  uint8_t *base = (uint8_t *)malloc(size);
  assert(base);

  int32_t dst = 0;

  const uint8_t *end = src + src_size;
  while (src < end) {
    const uint8_t marker = *(src++);
    const uint8_t count = marker & 0x7f;
    if (marker & 0x80) {
      for (uint32_t i = 0; i < count; ++i) {
        // read match data
        const int32_t offset = _vlq_read(src);
        const int32_t length = _vlq_read(src);
        assert(dst - offset >= 0);
        // realloc if needed
        if ((dst + length) >= size) {
          base = (uint8_t *)realloc(base, size = _npot(dst + length));
          assert(base);
        }
        // insert matches
        for (uint32_t j = 0; j < length; ++j) {
          base[dst + j] = base[dst + j - offset];
        }
        dst += length;
      }
    } else {
      // realloc if needed
      if ((dst + count) >= size) {
        base = (uint8_t *)realloc(base, size = _npot(dst + count));
        assert(base);
      }
      // insert raw bytes
      for (uint32_t i = 0; i < count; ++i) {
        base[dst++] = *(src++);
      }
    }
  }
  if (out_size) {
    *out_size = dst;
  }
  return base;
}


void* encode(const uint8_t *src, size_t src_size, size_t *written) {
  return nullptr;
}
