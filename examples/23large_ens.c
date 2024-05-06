#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "khashl.h"

KHASHE_MAP_INIT(KH_LOCAL, map64_t, map64, uint64_t, uint64_t, kh_hash_uint64, kh_eq_generic)

uint64_t splitmix64(uint64_t *x)
{
	uint64_t z = ((*x) += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int main(int argc, char *argv[])
{
	int i, n = 10000000;
	uint64_t x = 11, sum = 0;
	map64_t *h;
	kh_ensitr_t k;
	if (argc > 1) n = atol(argv[1]);
	h = map64_init(6);
	for (i = 0; i < n; ++i) {
		uint64_t z, key;
		int absent;
		z = splitmix64(&x);
		key = z % (n>>2);
		k = map64_put(h, key, &absent);
		if (absent) kh_ens_val(h, k) = z;
		else map64_del(h, k);
	}
	kh_ens_foreach(h, k) {
		sum += kh_ens_val(h, k);
	}
	printf("%llx\n", sum);
	map64_destroy(h);
	return 0;
}
