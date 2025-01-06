#include <stdlib.h>
#include <string.h>
#include "khashp.h"

#define kh_max_count(cap) (((cap)>>1) + ((cap)>>2)) /* default load factor: 75% */

#define MALLOC(type, cnt)       ((type*)malloc((cnt) * sizeof(type)))
#define CALLOC(type, cnt)       ((type*)calloc((cnt), sizeof(type)))
#define REALLOC(type, ptr, cnt) ((type*)realloc((ptr), (cnt) * sizeof(type)))

#define __kh_used(flag, i)       (flag[i>>5] >> (i&0x1fU) & 1U)
#define __kh_set_used(flag, i)   (flag[i>>5] |= 1U<<(i&0x1fU))
#define __kh_set_unused(flag, i) (flag[i>>5] &= ~(1U<<(i&0x1fU)))

#define __kh_fsize(m) ((m) < 32? 1 : (m)>>5)

static inline khint_t __kh_h2b(khint_t hash, khint_t bits) { return hash * 2654435769U >> (32 - bits); } // Fibonacci hashing

static khint_t khp_hash_fn0(const void *p, uint32_t len) // FNV-1a as generic hash function
{
	const uint8_t *s = (const uint8_t*)p;
	khint_t h = 2166136261U;
	uint32_t i;
	for (i = 0; i < len; ++i)
		h ^= s[i], h *= 16777619;
	return h;
}

static int khp_key_eq0(const void *key1, const void *key2, uint32_t key_len) // generic equality function
{
	return memcmp(key1, key2, key_len) == 0;
}

khashp_t *khp_init(uint32_t key_len, uint32_t val_len, khp_hash_fn_t fn, khp_key_eq_t eq)
{
	khashp_t *h = CALLOC(khashp_t, 1);
	h->key_len = key_len, h->val_len = val_len;
	h->hash_fn = fn? fn : khp_hash_fn0;
	h->key_eq  = eq? eq : khp_key_eq0;
	return h;
}

void khp_destroy(khashp_t *h)
{
	if (h == 0) return;
	free(h->b); free(h);
}

void khp_clear(khashp_t *h)
{
	if (h == 0 || h->used == 0) return;
	khint_t n_buckets = (khint_t)1U << h->bits;
	memset(h->used, 0, __kh_fsize(n_buckets) * sizeof(uint32_t));
	h->count = 0;
}

khint_t khp_get(const khashp_t *h, const void *key)
{
	khint_t i, last, n_buckets, mask, hash;
	if (h->b == 0) return 0;
	hash = h->hash_fn(key, h->key_len);
	n_buckets = (khint_t)1U << h->bits;
	mask = n_buckets - 1U;
	i = last = __kh_h2b(hash, h->bits);
	while (__kh_used(h->used, i) && h->key_eq(khp_get_bucket(h, i), key, h->key_len) != 0) {
		i = (i + 1U) & mask;
		if (i == last) return n_buckets;
	}
	return !__kh_used(h->used, i)? n_buckets : i;
}

int khp_resize(khashp_t *h, khint_t new_n_buckets)
{
	uint32_t *new_used = 0;
	uint8_t *tmp;
	khint_t j = 0, x = new_n_buckets, n_buckets, new_bits, new_mask;
	while ((x >>= 1) != 0) ++j;
	if (new_n_buckets & (new_n_buckets - 1)) ++j;
	new_bits = j > 2? j : 2;
	new_n_buckets = (khint_t)1U << new_bits;
	if (h->count > kh_max_count(new_n_buckets)) return 0; /* requested size is too small */
	new_used = MALLOC(uint32_t, __kh_fsize(new_n_buckets));
	memset(new_used, 0, __kh_fsize(new_n_buckets) * sizeof(uint32_t));
	if (!new_used) return -1; /* not enough memory */
	n_buckets = h->b? (khint_t)1U<<h->bits : 0U;
	if (n_buckets < new_n_buckets) { /* expand */
		uint8_t *new_b = REALLOC(uint8_t, h->b, new_n_buckets * (h->key_len + h->val_len));
		if (!new_b) { free(new_used); return -1; }
		h->b = new_b;
	} /* otherwise shrink */
	new_mask = new_n_buckets - 1;
	tmp = MALLOC(uint8_t, h->key_len + h->val_len);
	for (j = 0; j != n_buckets; ++j) {
		void *key;
		if (!__kh_used(h->used, j)) continue;
		key = khp_get_bucket(h, j);
		__kh_set_unused(h->used, j);
		while (1) { /* kick-out process; sort of like in Cuckoo hashing */
			khint_t i;
			i = __kh_h2b(h->hash_fn(key, h->key_len), new_bits);
			while (__kh_used(new_used, i)) i = (i + 1) & new_mask;
			__kh_set_used(new_used, i);
			if (i < n_buckets && __kh_used(h->used, i)) { /* kick out the existing element */
				void *keyi = khp_get_bucket(h, i);
				memcpy(tmp,  keyi, h->key_len + h->val_len);
				memcpy(keyi, key,  h->key_len + h->val_len);
				memcpy(key,  tmp,  h->key_len + h->val_len);
				__kh_set_unused(h->used, i); /* mark it as deleted in the old hash table */
			} else { /* write the element and jump out of the loop */
				memcpy(khp_get_bucket(h, i), key, h->key_len + h->val_len);
				break;
			}
		}
	}
	free(tmp);
	if (n_buckets > new_n_buckets) /* shrink the hash table */
		h->b = REALLOC(uint8_t, h->b, new_n_buckets * (h->key_len + h->val_len));
	free(h->used); /* free the working space */
	h->used = new_used, h->bits = new_bits;
	return 0;
}

khint_t khp_put(khashp_t *h, const void *key, int *absent)
{
	khint_t n_buckets, i, last, mask, hash;
	n_buckets = h->b? (khint_t)1U<<h->bits : 0U;
	*absent = -1;
	if (h->count >= kh_max_count(n_buckets)) { /* rehashing */
		if (khp_resize(h, n_buckets + 1U) < 0)
			return n_buckets;
		n_buckets = (khint_t)1U<<h->bits;
	} /* TODO: to implement automatically shrinking; resize() already support shrinking */
	mask = n_buckets - 1;
	hash = h->hash_fn(key, h->key_len);
	i = last = __kh_h2b(hash, h->bits);
	while (__kh_used(h->used, i) && !h->key_eq(khp_get_bucket(h, i), key, h->key_len)) {
		i = (i + 1U) & mask;
		if (i == last) break;
	}
	if (!__kh_used(h->used, i)) { /* not present at all */
		memcpy(khp_get_bucket(h, i), key, h->key_len);
		__kh_set_used(h->used, i);
		++h->count;
		*absent = 1;
	} else *absent = 0; /* Don't touch h->b[i] if present */
	return i;
}

int khp_del(khashp_t *h, khint_t i)
{
	khint_t j = i, k, mask, n_buckets;
	if (h->b == 0) return 0;
	n_buckets = (khint_t)1U<<h->bits;
	mask = n_buckets - 1U;
	while (1) {
		j = (j + 1U) & mask;
		if (j == i || !__kh_used(h->used, j)) break; /* j==i only when the table is completely full */
		k = __kh_h2b(h->hash_fn(khp_get_bucket(h, j), h->key_len), h->bits);
		if ((j > i && (k <= i || k > j)) || (j < i && (k <= i && k > j)))
			memcpy(khp_get_bucket(h, i), khp_get_bucket(h, j), h->key_len + h->val_len), i = j;
	}
	__kh_set_unused(h->used, i);
	--h->count;
	return 1;
}

void khp_get_val(const khashp_t *h, khint_t i, void *v)
{
	uint8_t *p = (uint8_t*)khp_get_bucket(h, i) + h->key_len;
	if (h->val_len > 0) memcpy(v, p, h->val_len);
}

void khp_set_val(const khashp_t *h, khint_t i, const void *v)
{
	uint8_t *p = (uint8_t*)khp_get_bucket(h, i) + h->key_len;
	if (h->val_len > 0) memcpy(p, v, h->val_len);
}

void khp_get_key(const khashp_t *h, khint_t i, void *p)
{
	memcpy(p, khp_get_bucket(h, i), h->key_len);
}

/*********************
 * String hash table *
 *********************/

static khint_t khp_str_hash_fn(const void *s, uint32_t key_len) // FNV-1a
{
	const uint8_t *p;
	memcpy(&p, s, key_len); // get the address to the string
	khint_t h = 2166136261U;
	for (; *p; ++p)
		h ^= *p, h *= 16777619;
	return h;
}

static int kh_str_key_eq(const void *s1, const void *s2, uint32_t key_len)
{
	const char *p1, *p2;
	memcpy(&p1, s1, key_len);
	memcpy(&p2, s2, key_len);
	return strcmp(p1, p2) == 0;
}

khashp_t *khp_str_init(uint32_t val_len, int dup)
{
	khashp_t *h = khp_init(sizeof(void*), val_len, khp_str_hash_fn, kh_str_key_eq);
	h->dup = !!dup;
	return h;
}

void khp_str_destroy(khashp_t *h)
{
	if (h->dup) {
		khint_t k;
		khp_foreach(h, k) {
			char *p;
			khp_get_key(h, k, &p);
			free(p); // free
		}
	}
	khp_destroy(h);
}

khint_t khp_str_get(const khashp_t *h, const char *key)
{
	return khp_get(h, &key);
}

khint_t khp_str_put(khashp_t *h, const char *key, int *absent)
{
	khint_t k = khp_put(h, &key, absent);
	if (*absent) {
		if (h->dup) {
			size_t len = strlen(key);
			char *q = MALLOC(char, len + 1);
			memcpy(q, key, len + 1);
			memcpy(khp_get_bucket(h, k), &q, h->key_len); // the bucket keeps the address to the string
		} else {
			memcpy(khp_get_bucket(h, k), &key, h->key_len);
		}
	}
	return k;
}

int khp_str_del(khashp_t *h, khint_t i)
{
	if (h->b == 0) return 0;
	if (h->dup) {
		char *p;
		khp_get_key(h, i, &p);
		free(p);
	}
	return khp_del(h, i);
}
