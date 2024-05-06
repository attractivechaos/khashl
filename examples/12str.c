#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "khashl.h"

KHASHL_MAP_INIT(KH_LOCAL, strmap_t, strmap, const char*, int, kh_hash_str, kh_eq_str)

int main(void)
{
	int absent;
	khint_t k;
	strmap_t *h;

	h = strmap_init();

	// put 
	k = strmap_put(h, strdup("abc"), &absent);
	kh_val(h, k) = 2;
	k = strmap_put(h, strdup("def"), &absent);
	kh_val(h, k) = 5;
	k = strmap_put(h, "ghi", &absent);
	if (absent) {
		kh_key(h, k) = strdup("ghi");
		kh_val(h, k) = 7;
	}

	// get
	k = strmap_get(h, "xyz");
	assert(k == kh_end(h));
	k = strmap_get(h, "abc");
	assert(k < kh_end(h));

	// iterate
	kh_foreach(h, k) {
		printf("h[%s]=%d\n", kh_key(h, k), kh_val(h, k));
		free((void*)kh_key(h, k)); // free memory allocated by strdup()
	}

	strmap_destroy(h);
	return 0;
}
