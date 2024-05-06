#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "khashl.h"

KHASHE_MAP_INIT(KH_LOCAL, map64_t, map64, uint64_t, int, kh_hash_uint64, kh_eq_generic)

int main(void)
{
	int absent;
	kh_ensitr_t k;
	map64_t *h;

	h = map64_init(6);

	// put 
	k = map64_put(h, 20, &absent);
	kh_ens_val(h, k) = 2;
	k = map64_put(h, 50, &absent);
	kh_ens_val(h, k) = 5;

	// get
	k = map64_get(h, 30);
	assert(kh_ens_is_end(k));
	k = map64_get(h, 20);
	assert(!kh_ens_is_end(k));

	// iterate
	kh_ens_foreach(h, k) {
		printf("h[%lu]=%d\n", (unsigned long)kh_ens_key(h, k), kh_ens_val(h, k));
	}

	map64_destroy(h);
	return 0;
}
