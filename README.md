## Table of Contents

- [Introduction](#intro)
- [Usage](#use)
  - [Integer keys](#int)
  - [String keys](#str)
  - [Custom keys](#custom)
- [Algorithm](#algo)
- [Ensemble of hash tables](#ensemble)
  - [Rationale](#rationale)
  - [Use ensemble](#use-ens)
- [Performance](#perf)

## <a name="intro"></a>Introduction

Khashl is a single-header macro-based generic hash table library in C. It is an
improved version of [khash][khash] from [klib][klib] and is one of the faster
hash table implementations in C/C++. Klib also has a copy of khashl for
historical reason. This repo provides more [examples][ex] and better
documentation.

## <a name="use"></a>Usage

### <a name="int"></a>Integer keys

Here is a small example for integer keys:
```c
#include <stdio.h>
#include <stdint.h>
#include "khashl.h"
// Instantiate
KHASHL_MAP_INIT(KH_LOCAL, map32_t, map32, uint32_t, int, kh_hash_uint32, kh_eq_generic)

int main(void) {
    int absent;
    khint_t k;
    map32_t *h = map32_init();
    k = map32_put(h, 20, &absent); // get iterator to the new bucket
    kh_val(h, k) = 2; // set value
    k = map32_get(h, 30); // query the hash table
    if (k < kh_end(h)) printf("found key '30'\n");
    kh_foreach(h, k) { // iterate
        printf("h[%u]=%d\n", kh_key(h, k), kh_val(h, k));
    }
    map32_destroy(h); // free
    return 0;
}
```

To use khashl, you need to instantiate functions specific to your types with
```c
KHASHL_MAP_INIT(scope, table_type, prefix, key_type, val_type, hash_func, eq_func)
```
where:
 * `scope` is the scope of instantiated functions. It can be empty for global
   visibility or `KH_LOCAL`.
 * `table_type` is the type of the hash table. It can be any symbol that has not
   been used.
 * `prefix` is the prefix of instantiated functions (see below)
 * `key_type` is the key type
 * `val_type` is the value type
 * `hash_func` is the hash function. Khashl provides hash functions for 32-bit
   integers, 64-bit integers and strings. See [khashl.h][khashl.h] for details.
 * `eq_func` is the equality function. For primitive types, use the
   `kh_eq_generic()` macro; for strings, use `kh_eq_str()`.

After instantiation, you will be able to use the following functions:
 * `table_type *prefix_init(void)`: initialize an empty hash table.
 * `khint_t prefix_put(table_type*, key_type, int*)`: put a key into the
   table. It returns the position in the table. The last parameter tells you
   whether the key is new.
 * `khint_t prefix_get(table_type*, key_type)`: query a key. It returns
   the position of the key if the key is present; otherwise the function
   returns `kh_end(table)`.
 * `prefix_del(table_type*, khint)`: delete the key at a postion.
 * `prefix_destroy(table_type*)`: deallocate the entire table.

In khashl, a position is like an iterator. `prefix_get()` and `prefix_put()`
return iterators. Khashl additionally provides the following macros:
 * `key_type kh_key(table, pos)`: access or modify keys. It can be an L-value.
   Don't modify the content of keys. If the `key_type` is a pointer, you may
   change the value of the pointer but not the content the pointer points to.
 * `val_type kh_val(table, pos)`: access or modify values.
 * `kh_size(table)`: return the size of the table
 * `kh_end(table)`: return the capacity of the table
 * `kh_exist(table, pos)`: test whether the bucket at `pos` is empty. `pos`
   must be smaller than `kh_end(table)`.
 * `kh_foreach(table, pos) { }`: iterate a table. `pos` should be defined at the
   `khint_t` type before this macro. Note that because `prefix_put()` and
   `prefix_del()` may change the content of the hash table, please do not call
   these two functions inside a foreach loop.

### <a name="str"></a>String keys

It is important to note that khashl only keeps the pointers to strings. You are
responsible for managing the memory allocated to the strings.

Here is an example for counting the number of distinct words on the commnand
line:
```c
// To run this program: `./this_prog abc bc abc a bc`
#include <stdio.h>
#include <string.h>
#include "khashl.h"
KHASHL_SET_INIT(KH_LOCAL, strmap_t, strmap, const char*, kh_hash_str, kh_eq_str)

int main(int argc, char *argv[])
{
    strmap_t *h;
    int i, absent;
    h = strmap_init();
    for (i = 1; i < argc; ++i)
        strmap_put(h, argv[i], &absent);
    printf("# of distinct words: %d\n", kh_size(h));
    strmap_destroy(h);
    return 0;
}
```
In this example, the string contents are already stored in the `argv[]` array.
You don't need to worry about memory management. The following demonstrates
how to insert string pointers and their contents into a hash table.
```c
// To run this program: `echo a bc a cd bc|./this_prog`
#include <stdio.h>
#include <string.h>
#include "khashl.h"
KHASHL_MAP_INIT(KH_LOCAL, strmap_t, strmap, const char*, int, kh_hash_str, kh_eq_str)

int main(int argc, char *argv[])
{
    char s[4096]; // max string length: 4095 characters
    strmap_t *h;
    khint_t k;
    h = strmap_init();
    while (scanf("%s", s) > 0) {
        int absent;
        k = strmap_put(h, s, &absent);
        if (absent) kh_key(h, k) = strdup(s), kh_val(h, k) = 0;
        // else, the key is not touched; we do nothing
        ++kh_val(h, k);
    }
    printf("# of distinct words: %d\n", kh_size(h));
    // IMPORTANT: free memory allocated by strdup() above
    kh_foreach(h, k) {
        printf("%s: %d\n", kh_key(h, k), kh_val(h, k));
        free((char*)kh_key(h, k));
    }
    strmap_destroy(h);
    return 0;
}
```

### <a name="custom"></a>Custom keys

You can put C `struct` into a hash table as long as you provide a hash function
and an equality function. You can use macro functions.

## <a name="algo"></a>Algorithm

Khashl uses linear probing and power-of-2 capacity. It applies [Fibonacci
hashing][fib-hash] to protect against bad hash functions and implements
[deletion without tombstones][no-tombstone]. Khashl uses one bit per bucket
to indicate whether a bucket is empty. It has minimal memory overhead though
this comes at the cost of one extra cache miss per query. Khashl does not use
SIMD.

## <a name="ensemble"></a>Ensemble of hash tables

Khashl uses 32-bit hashes, which means it cannot directly store more than 4
billion keys. Nonetheless, it has a special way to handle billions of keys:
ensemble of hash tables.

### <a name="rationale"></a>Rationale

Suppose a hash table consists of `n` smaller sub hash tables. A key `x` is
located in sub-table `hash(x) % n`. Because it is rare for all sub-tables to
rehash at the same time, the peak memory can be reduced. You can find more
explanation in [this blog][ensemble]. In my opinion, **using an ensemble of
hash tables it the best strategy for huge hash tables**.

We can implement a hash table ensemble in the user space for any libraries. I
have been using the idea since 2015. Nonetheless, it is more convenient to
hide the details behind the library code such that users can use familiar hash
table APIs. [phmap][phmap] is perhaps the first library to do this. Now khashl
has this functionality as well.

### <a name="use-ens"></a>Use ensemble

The [integer example above](#int) becomes:
```c
#include <stdio.h>
#include <stdint.h>
#include "khashl.h"
// use "KHASHE" for instantiation
KHASHE_MAP_INIT(KH_LOCAL, map32_t, map32, uint32_t, int, kh_hash_uint32, kh_eq_generic)
int main(void) {
    int absent;
    kh_ensitr_t k; // use kh_ensitr_t instead of khint_t
    map32_t *h = map32_init(6); // use 2**6=64 sub hash tables
    k = map32_put(h, 20, &absent); // get iterator to the new bucket
    kh_ens_val(h, k) = 2; // use kh_ens_val() instead of kh_val()
    k = map32_get(h, 30); // query the hash table
    if (!kh_ens_is_end(k)) printf("found key '30'\n"); // use kh_ens_is_end()
    kh_ens_foreach(h, k) { // use kh_ens_foreach() instead of kh_foreach()
        printf("h[%u]=%d\n", kh_ens_key(h, k), kh_ens_val(h, k));
    }
    map32_destroy(h);
    return 0;
}
```
You will have to change most macros and iteration:
 * `khint_t` &#8594; `kh_ensitr_t` (iterator type)
 * `kh_key()` &#8594; `kh_ens_key()`
 * `kh_val()` &#8594; `kh_ens_val()`
 * `kh_foreach()` &#8594; `kh_ens_foreach()`
 * `prefix_init(void)` &#8594; `prefix_init(int b)`, which enables `2**b` sub hash tables.
 * `k == kh_end(h)` &#8594; `kh_ens_is_end(k)` for testing the presence of a key

## <a name="perf"></a>Performance

<img align="left" src="https://i.ibb.co/Nrxr81J/240509-fast.png"/>

For details, see [udb3][udb3].

[klib]: https://github.com/attractivechaos/klib
[khash]: https://github.com/attractivechaos/klib/blob/master/khash.h
[ex]: https://github.com/attractivechaos/khashl/tree/main/examples
[khashl.h]: https://github.com/attractivechaos/khashl/blob/main/khashl.h
[fib-hash]: https://probablydance.com/2018/06/16/fibonacci-hashing-the-optimization-that-the-world-forgot-or-a-better-alternative-to-integer-modulo/
[no-tombstone]: https://attractivechaos.wordpress.com/2019/12/28/deletion-from-hash-tables-without-tombstones/
[ensemble]: https://greg7mdp.github.io/parallel-hashmap/
[phmap]: https://github.com/greg7mdp/parallel-hashmap
[udb3]: https://github.com/attractivechaos/udb3/
