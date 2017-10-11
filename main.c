#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/stat.h>
#include <byteswap.h>

extern void sha256_transform_ssse3(uint32_t* digest, const char* data, uint64_t rounds);
extern void sha256_transform_avx(uint32_t* digest, const char* data, uint64_t rounds);
extern void sha256_transform_rorx(uint32_t* digest, const char* data, uint64_t rounds);

#define SHA256_H0   0x6a09e667UL
#define SHA256_H1   0xbb67ae85UL
#define SHA256_H2   0x3c6ef372UL
#define SHA256_H3   0xa54ff53aUL
#define SHA256_H4   0x510e527fUL
#define SHA256_H5   0x9b05688cUL
#define SHA256_H6   0x1f83d9abUL
#define SHA256_H7   0x5be0cd19UL

#define SHA256_DIGEST_SIZE  32
#define SHA256_BLOCK_SIZE   64

typedef struct
{
  uint32_t state[8];
  uint64_t count;
  uint8_t buffer[64];
} sha256_state;

int main(int argc, char** argv)
{
  struct stat st;
  uint64_t file_size = 0;
  if(stat(argv[1], &st) == 0)
  {
    file_size = st.st_size;
  }
  else
  {
    printf("Unable to stat file\n");
    return 1;
  }

  FILE* file = fopen(argv[1], "r");

  char* file_buf = malloc(file_size);
  uint64_t bytes_read = fread(file_buf, sizeof(char), file_size, file);

  uint32_t digest_out[8];

  sha256_state sha_state;

  sha256_state* ctx = &sha_state;

  ctx->state[0] = SHA256_H0;
  ctx->state[1] = SHA256_H1;
  ctx->state[2] = SHA256_H2;
  ctx->state[3] = SHA256_H3;
  ctx->state[4] = SHA256_H4;
  ctx->state[5] = SHA256_H5;
  ctx->state[6] = SHA256_H6;
  ctx->state[7] = SHA256_H7;
  ctx->count = 0;

  uint8_t* data = file_buf;
  uint32_t len = file_size;
  uint32_t partial = ctx->count % SHA256_BLOCK_SIZE;

  // Update
  ctx->count += len;

  if((partial + len) >= SHA256_BLOCK_SIZE)
  {
    int blocks;

    if(partial)
    {
      int p = SHA256_BLOCK_SIZE - partial;

      memcpy(ctx->buffer + partial, data, p);
      data += p;
      len -= p;

      //sha256_transform_avx(ctx->state, ctx->buffer, 1);
      sha256_transform_rorx(ctx->state, ctx->buffer, 1);
    }

    blocks = len / SHA256_BLOCK_SIZE;
    len %= SHA256_BLOCK_SIZE;

    if(blocks)
    {
      //sha256_transform_avx(ctx->state, data, blocks);
      sha256_transform_rorx(ctx->state, data, blocks);
      data += blocks * SHA256_BLOCK_SIZE;
    }
    partial = 0;
  }
  
  if(len)
    memcpy(ctx->buffer + partial, data, len);
  // End Update

  // Do-Finalize
  const int offset = SHA256_BLOCK_SIZE - sizeof(uint64_t);
  uint64_t* bits = (uint64_t*)(ctx->buffer + offset);
  partial = ctx->count % SHA256_BLOCK_SIZE;

  ctx->buffer[partial++] = 0x80;
  if(partial > offset)
  {
    memset(ctx->buffer + partial, 0x0, SHA256_BLOCK_SIZE - partial);
    partial = 0;

    //sha256_transform_avx(ctx->state, ctx->buffer, 1);
    sha256_transform_rorx(ctx->state, ctx->buffer, 1);
  }

  memset(ctx->buffer + partial, 0x0, offset - partial);
  *bits = bswap_64(ctx->count << 3);
  //sha256_transform_avx(ctx->state, ctx->buffer, 1);
  sha256_transform_rorx(ctx->state, ctx->buffer, 1);
  // End-Finalize

  // Finish
  uint32_t digest_size = SHA256_DIGEST_SIZE;
  uint32_t* digest = (uint32_t*)digest_out;
  int i;

  for(i = 0; digest_size > 0; i++, digest_size -= sizeof(uint32_t))
  {
    *digest++ = ctx->state[i];
  }

  for(int i = 0; i < 8; i++)
  {
    printf("%08x", digest_out[i]);
  }
  printf("  %s\n", argv[1]);

  free(file_buf);

  return 0;
}
