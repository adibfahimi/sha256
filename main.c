#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t H[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

uint32_t rotr(uint32_t a, uint32_t n) { return (a >> n) | (a << (32 - n)); }

uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }

uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t SIGMA0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }

uint32_t SIGMA1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }

uint32_t sigma0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }

uint32_t sigma1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

size_t calculate_padding(size_t original_size) {
  return ((original_size + 8 + 63) / 64) * 64;
}

char *sha256(const char *input) {
  size_t original_size = strlen(input);
  size_t data_size = calculate_padding(original_size);

  unsigned char *data = malloc(data_size);
  if (!data)
    return NULL;

  memset(data, 0, data_size);
  memcpy(data, input, original_size);
  data[original_size] = 0x80;

  size_t padding_start = original_size + 1;
  size_t padding_end = data_size - 8;
  if (padding_start < padding_end)
    memset(data + padding_start, 0, padding_end - padding_start);

  uint64_t bit_size = (uint64_t)original_size * 8;
  for (int i = 0; i < 8; i++)
    data[data_size - 8 + i] = (bit_size >> (56 - i * 8)) & 0xFF;

  uint32_t h[8];
  memcpy(h, H, sizeof(H));

  int num_blocks = data_size / 64;
  for (int block = 0; block < num_blocks; block++) {
    uint32_t m[64];
    unsigned char *block_data = data + block * 64;

    for (int i = 0; i < 16; i++)
      m[i] = (block_data[i * 4] << 24) | (block_data[i * 4 + 1] << 16) |
             (block_data[i * 4 + 2] << 8) | block_data[i * 4 + 3];

    for (int i = 16; i < 64; i++)
      m[i] = sigma1(m[i - 2]) + m[i - 7] + sigma0(m[i - 15]) + m[i - 16];

    uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
    uint32_t e = h[4], f = h[5], g = h[6], h_val = h[7];

    for (int i = 0; i < 64; i++) {
      uint32_t t1 = h_val + SIGMA1(e) + Ch(e, f, g) + K[i] + m[i];
      uint32_t t2 = SIGMA0(a) + Maj(a, b, c);
      h_val = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
    }

    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
    h[5] += f;
    h[6] += g;
    h[7] += h_val;
  }

  free(data);

  char *hash = malloc(65);
  if (!hash)
    return NULL;

  for (int i = 0; i < 8; i++)
    sprintf(hash + i * 8, "%08x", h[i]);

  hash[64] = '\0';
  return hash;
}

int main() {
  char *hash = sha256("aa");
  printf("hash: %s\n", hash);
  free(hash);
  return 0;
}
