#ifndef KHASHP_H
#define KHASHP_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t khint_t;
typedef khint_t (*khp_hash_fn_t)(const void *key, uint32_t key_len);
typedef int (*khp_key_eq_t)(const void *key1, const void *key2, uint32_t key_len);

typedef struct {
	uint32_t key_len, val_len;
	uint32_t bits;
	khint_t count;
	khp_hash_fn_t hash_fn;
	khp_key_eq_t key_eq;
	uint8_t *b;
	uint32_t *used;
} khashp_t;

#ifdef __cplusplus
extern "C" {
#endif

khashp_t *khp_init(uint32_t key_len, uint32_t val_len, khp_hash_fn_t fn, khp_key_eq_t eq);
void khp_destroy(khashp_t *h);
void khp_clear(khashp_t *h);
int khp_resize(khashp_t *h, khint_t new_n_buckets);

khint_t khp_get(const khashp_t *h, const void *key);
khint_t khp_put(khashp_t *h, const void *key, int *absent);
int khp_del(khashp_t *h, khint_t i);

void khp_get_key(const khashp_t *h, khint_t i, void *p);
void khp_get_val(const khashp_t *h, khint_t i, void *v);
void khp_set_val(const khashp_t *h, khint_t i, const void *v);

#define khp_capacity(h) ((h)->b? 1U<<(h)->bits : 0U)
#define khp_end(h) khp_capacity(h)
#define khp_size(h) ((h)->count)
#define khp_exist(h, x) ((h)->used[(x)>>5] >> ((x)&0x1fU) & 1U)
#define khp_foreach(h, x) for ((x) = 0; (x) != khp_end(h); ++(x)) if (khp_exist((h), (x)))

khashp_t *khp_str_init(uint32_t val_len);
void khp_str_destroy(khashp_t *h);
khint_t khp_str_get(const khashp_t *h, const char *key);
khint_t khp_str_put(khashp_t *h, const char *key, int *absent);
int khp_str_del(khashp_t *h, khint_t i);

static inline void *khp_get_bucket(const khashp_t *h, khint_t i)
{
	return &h->b[(h->key_len + h->val_len) * i];
}

#ifdef __cplusplus
}
#endif

#endif
