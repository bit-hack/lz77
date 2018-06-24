#pragma once
#include <cstdint>

void encode(const uint8_t *src, uint8_t *dst, size_t src_size, size_t dst_size,
            size_t *written);

void decode(const uint8_t *src, uint8_t *dst, size_t src_size, size_t dst_size,
            size_t *out_size);



void* encode(const uint8_t *src, size_t src_size, size_t *written);

void* decode(const uint8_t *src, size_t src_size, size_t *out_size);
