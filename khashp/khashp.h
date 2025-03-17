#ifndef __AC_KHASHP_H
#define __AC_KHASHP_H

#define AC_VERSION_KHASHP_H "r35"

#include <stddef.h>
#include <stdint.h>

typedef uint32_t khint_t;
typedef khint_t (*khp_hash_fn_t)(const void *key, uint32_t key_len);
typedef int (*khp_key_eq_t)(const void *key1, const void *key2, uint32_t key_len);

typedef struct {
	uint32_t key_len, val_len; // key and value lengths in bytes
	uint16_t bits;             // the capacity of the hash table is 1<<bits
	uint16_t dup;              // whether to duplicate string keys (only used for string hash tables)
	khint_t count;             // number of elements
	khp_hash_fn_t hash_fn;     // hash function
	khp_key_eq_t key_eq;       // equality function
	uint8_t *b;                // buckets
	uint32_t *used;            // bit flag that indicates which buckets are occupied
} khashp_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KHASHP_STATIC

/**
 * Initialize a hash table
 *
 * All keys (and values) must be of the same length in bytes. For string keys,
 * see khp_str_init().
 *
 * @param key_len      length of a key in bytes
 * @param val_len      length of a value in bytes
 * @param fn           function to hash keys; NULL for FNV-1a as hash function
 * @param eq           function that tests equality of keys; NULL for memcpy comparison
 *
 * @return pointer to the hash table
 */
khashp_t *khp_init(uint32_t key_len, uint32_t val_len, khp_hash_fn_t fn, khp_key_eq_t eq);

/** Deallocate a hash table */
void khp_destroy(khashp_t *h);

/** Clear the content of a hash table */
void khp_clear(khashp_t *h);

/** Resize a hash table */
int khp_resize(khashp_t *h, khint_t new_n_buckets);

/**
 * Query a hash table
 *
 * @param h            pointer to the hash table
 * @param key          pointer to the query key
 *
 * @return offset in buckets if present, or khp_end(h) if absent
 */
khint_t khp_get(const khashp_t *h, const void *key);

/**
 * Insert a key
 *
 * @param h            pointer to the hash table
 * @param key          pointer to the key
 * @param absent       (out) 1 if the key is present; 0 otherwise
 *
 * @return offset in bucekts
 */
khint_t khp_put(khashp_t *h, const void *key, int *absent);

/**
 * Delete an element
 *
 * @param h            pointer to the hash table
 * @param i            offset in buckets which is returned by khp_get() or khp_put()
 *
 * @return 1 on success
 */
int khp_del(khashp_t *h, khint_t i);

/**
 * Get a key
 *
 * @param h            pointer to the hash table
 * @param i            offset in buckets
 * @param p            (out) pointer to the key
 */
void khp_get_key(const khashp_t *h, khint_t i, void *p);

/** Get a value */
void khp_get_val(const khashp_t *h, khint_t i, void *v);

/** Set a value */
void khp_set_val(const khashp_t *h, khint_t i, const void *v);

/**
 * Initialize a hash table with string keys
 *
 * Each occupied bucket keeps the pointer to a string. If _dup_ is 0, the
 * string content is not duplicated in the hash table and users need to
 * maintain the memory and the content of the string. If _dup_ is 1, the string
 * is duplicated with strdup() when a new key is inserted. khp_str_destroy()
 * and khp_str_del() free the duplicated string contents automatically.
 *
 * @param val_len      length of a value in bytes
 * @param dup          whether to duplicate string contents
 *
 * @return pointer to the hash table
 */
khashp_t *khp_str_init(uint32_t val_len, int dup);

/** Deallocate a string hash table */
void khp_str_destroy(khashp_t *h);

/** Get a string hash table */
khint_t khp_str_get(const khashp_t *h, const char *key);

/** Insert to a string hash table */
khint_t khp_str_put(khashp_t *h, const char *key, int *absent);

/** Delete an element from a string hash table */
int khp_str_del(khashp_t *h, khint_t i);

#endif // ~KHASHP_STATIC

/** Get the capacity of a hash table */
static inline khint_t khp_capacity(const khashp_t *h) { return h->b? 1U<<h->bits : 0U; }

/** End "iterator" */
static inline khint_t khp_end(const khashp_t *h) { return khp_capacity(h); }

/** Get the number of elements in a hash table */
static inline khint_t khp_size(const khashp_t *h) { return h->count; }

/** Test whether a bucket is occupied */
static inline int khp_exist(const khashp_t *h, khint_t x) { return h->used[x>>5] >> (x&0x1fU) & 1U; }

/** Iterate over a hash table */
#define khp_foreach(h, x) for ((x) = 0; (x) != khp_end(h); ++(x)) if (khp_exist((h), (x)))

static inline void *khp_get_bucket(const khashp_t *h, khint_t i)
{
	return &h->b[(h->key_len + h->val_len) * i];
}

#ifdef __cplusplus
}
#endif

#endif // defined(__AC_KHASHP_H)
