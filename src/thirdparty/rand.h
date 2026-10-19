#ifndef RAND_H
#define RAND_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define RAND_CHUNK_LEN 5

typedef struct
{
  uint64_t state;
} rand_t;

static inline void rand_init(rand_t* r)
{
  r->state = (uint64_t)(uintptr_t)r ^ (uint64_t)time(NULL);
  if (r->state == 0)
    r->state = 0xdeadbeefcafe1337ULL;
  r->state ^= r->state << 13;
  r->state ^= r->state >> 7;
  r->state ^= r->state << 17;
}

static inline uint64_t rand_next(rand_t* r)
{
  r->state ^= r->state << 13;
  r->state ^= r->state >> 7;
  r->state ^= r->state << 17;
  return r->state;
}

static inline void rand_chunk(rand_t* r, char* out)
{
  static const char charset[] = 
    "abcdefghijklmnopqrstuvwxyz0123456789";
  static const char letters[] = "abcdefghijklmnopqrstuvwxyz";

  out[0] = '.';
  uint64_t val = rand_next(r);
  out[1] = letters[val % (sizeof(letters) - 1)];
  val /= sizeof(letters) - 1;
  for (int i = 2; i <= RAND_CHUNK_LEN; i++) {
    out[i] = charset[val % 36];
    val /= 36;
  }
  out[RAND_CHUNK_LEN + 1] = '\0';
}

static inline void rand_string_id(rand_t* r, char* out) 
{
  static const char charset[] = 
    "abcdefghijklmnopqrstuvwxyz0123456789";
  static const char letters[] = "abcdefghijklmnopqrstuvwxyz";

  uint64_t val = rand_next(r);
  out[0] = letters[val % (sizeof(letters) - 1)];
  val /= sizeof(letters) - 1;
  for (int i = 1; i <= RAND_CHUNK_LEN; i++) {
    out[i] = charset[val % 36];
    val /= 36;
  }

  out[RAND_CHUNK_LEN + 1] = '\0';
}

static inline void rand_string_id_gen(void* ctx, char* out)
{
  rand_string_id((rand_t*)ctx, out);
}

static inline void rand_chunk_gen(void* ctx, char* out)
{
  rand_chunk((rand_t*)ctx, out);
}

#endif // RAND_H
