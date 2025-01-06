#include <stdio.h>
#include <string.h>
#include "khashp.h"

int main(int argc, char *argv[])
{
	char s[4096]; // max string length: 4095 characters
	khashp_t *h = khp_str_init(sizeof(int32_t), 1);
	while (scanf("%s", s) > 0) {
		int absent;
		khint_t k = khp_str_put(h, s, &absent);
		int32_t c = 0;
		if (!absent) khp_get_val(h, k, &c);
		++c;
		khp_set_val(h, k, &c);
	}
	printf("# of distinct words: %d\n", khp_size(h));
	khp_str_destroy(h);
	return 0;
}
