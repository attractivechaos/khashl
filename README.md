## Introduction

Khashl is a single-header macro-based generic hash table library in C. It is an
improved version of [khash][khash] from [klib][klib]. Klib also has a copy of
khashl for historical reason. This separate repo provides more [examples][ex].

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
    k = map32_put(h, 20, &absent);
    kh_val(h, k) = 2;
    k = map32_get(h, 30);
	if (k == kh_end(h)) printf("found key '30'\n");
    kh_foreach(h, k) {
        printf("h[%u]=%d\n", kh_key(h, k), kh_val(h, k));
    }
    map32_destroy(h);
    return 0;
}
```

[klib]: https://github.com/attractivechaos/klib
[khash]: https://github.com/attractivechaos/klib/blob/master/khash.h
[ex]: https://github.com/attractivechaos/khashl/tree/main/examples
