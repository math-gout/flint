/*
    Copyright (C) 2012 Sebastian Pancratz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

/*
    Benchmarks for the p-adic multiplication routine.

    We consider the set-up with p = 17, N = 2^i, i = 0, ..., 19,
    and multiply a = 3^{3 N}, b = 5^{3 N} mod p^N.
 */

#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "flint.h"
#include "fmpz.h"
#include "padic.h"

int
main(void)
{
    long l, len = 20;
    long runs[] = {
        10000000, 10000000, 10000000, 10000000, 1000000,
        1000000, 1000000, 1000000, 100000, 100000,
        100000, 10000, 10000, 1000, 1000,
        100, 100, 10, 10, 10
    };
    long N[] = {
        1, 2, 4, 8, 16,
        32, 64, 128, 256, 512,
        1024, WORD(1) << 11, WORD(1) << 12, WORD(1) << 13, WORD(1) << 14,
        WORD(1) << 15, WORD(1) << 16, WORD(1) << 17, WORD(1) << 18, WORD(1) << 19
    };
    long T[20] = {0};

    flint_printf("Benchmark for p-adic multiplication.\n");
    fflush(stdout);

for (l = 0; l < len; l++)
{
    long n = N[l], r;
    clock_t c0, c1;
    long double cputime;

    fmpz_t p;
    padic_ctx_t ctx;
    padic_t a, b, z;

    FLINT_TEST_INIT(state);

    fmpz_init_set_ui(p, 17);

    padic_ctx_init(ctx, p, n, n, PADIC_VAL_UNIT);

    padic_init(a);
    padic_init(b);
    padic_init(z);

    {
        fmpz_t f = {WORD(3)}, g = {WORD(5)}, pow;

        fmpz_init(pow);
        fmpz_pow_ui(pow, p, n);
        fmpz_pow_ui(padic_unit(a), f, 3 * n);
        fmpz_pow_ui(padic_unit(b), g, 3 * n);
        fmpz_mod(padic_unit(a), padic_unit(a), pow);
        fmpz_mod(padic_unit(b), padic_unit(b), pow);
        fmpz_clear(pow);
    }

    c0 = clock();
    for (r = runs[l]; (r); r--)
    {
        padic_mul(z, a, b, ctx);
        padic_zero(z);
    }
    c1 = clock();

    cputime = (long double) (c1 - c0) / (long double) CLOCKS_PER_SEC;

    T[l] = (slong) (cputime * (1000000000 / runs[l]));

    flint_printf("%2ld, %4XYXYXYXY, %8ld, %wd\n",
        l, cputime, runs[l], T[l]);

    padic_clear(a);
    padic_clear(b);
    padic_clear(z);

    fmpz_clear(p);
    padic_ctx_clear(ctx);
    flint_rand_clear(state);
}

    flint_printf("Output as a list:\n");
    for (l = 0; l < len; l++)
        flint_printf("%wd, ", T[l]);
    flint_printf("\n");

    return 0;
}
