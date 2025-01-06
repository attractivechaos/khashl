#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/resource.h>
#include "khashp.h"

static double udb_cputime(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
	return r.ru_utime.tv_sec + r.ru_stime.tv_sec + 1e-6 * (r.ru_utime.tv_usec + r.ru_stime.tv_usec);
}

static long udb_peakrss(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
#ifdef __linux__
	return r.ru_maxrss * 1024;
#else
	return r.ru_maxrss;
#endif
}

uint64_t splitmix64(uint64_t *x)
{
	uint64_t z = ((*x) += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int main(int argc, char *argv[])
{
	int i, n = 30000000;
	uint64_t x = 11, sum = 0;
	khashp_t *h;
	double t;
	khint_t k;

	if (argc > 1) n = atol(argv[1]);
	t = udb_cputime();
	h = khp_init(8, 8, NULL, NULL);
	for (i = 0; i < n; ++i) {
		uint64_t z, key;
		int absent;
		z = splitmix64(&x);
		key = z % (n>>2);
		k = khp_put(h, &key, &absent);
		if (absent) khp_set_val(h, k, &z);
		else khp_del(h, k);
	}
	khp_foreach(h, k) {
		uint64_t v;
		khp_get_val(h, k, &v);
		sum += v;
	}
	printf("elements: %lu\n", (long)khp_size(h));
	printf("checksum: %llx\n", sum);
	printf("CPU time: %.3f sec\n", udb_cputime() - t);
	printf("Peak RSS: %.3f MB\n", udb_peakrss() / 1024. / 1024.);
	khp_destroy(h);
	return 0;
}
