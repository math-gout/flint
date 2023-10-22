/*
    Copyright (C) 2009 William Hart

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "flint.h"
#include "ulong_extras.h"
#include "fmpz.h"

int
main(void)
{
    int i, result;
    FLINT_TEST_INIT(state);

    flint_printf("divexact....");
    fflush(stdout);

    for (i = 0; i < 10000 * flint_test_multiplier(); i++)
    {
        fmpz_t a, b, c;
        mpz_t d, e, f, g;
        int aliasing;

        fmpz_init(a);
        fmpz_init(b);
        fmpz_init(c);

        mpz_init(d);
        mpz_init(e);
        mpz_init(f);
        mpz_init(g);

        fmpz_randtest(a, state, 200);
        fmpz_randtest_not_zero(b, state, 200);
        fmpz_mul(c, a, b);

        fmpz_get_mpz(d, b);
        fmpz_get_mpz(e, c);

        aliasing = n_randint(state, 4);

        if (aliasing == 0)
        {
            fmpz_divexact(a, c, b);
        }
        else if (aliasing == 1)
        {
            fmpz_set(c, b);
            mpz_set(e, d);
            fmpz_divexact(a, c, c);
        }
        else if (aliasing == 2)
        {
            fmpz_set(a, c);
            fmpz_divexact(a, a, b);
        }
        else
        {
            fmpz_set(a, b);
            fmpz_divexact(a, c, a);
        }

        mpz_divexact(f, e, d);

        fmpz_get_mpz(g, a);

        result = (mpz_cmp(f, g) == 0) && _fmpz_is_canonical(a);
        if (!result)
        {
            flint_printf("FAIL %d:\n");
            gmp_printf("d = %Zd, e = %Zd, f = %Zd, g = %Zd\n", d, e, f, g);
            fflush(stdout);
            flint_abort();
        }

        fmpz_clear(a);
        fmpz_clear(b);
        fmpz_clear(c);

        mpz_clear(d);
        mpz_clear(e);
        mpz_clear(f);
        mpz_clear(g);
    }

    FLINT_TEST_CLEANUP(state);

    flint_printf("PASS\n");
    return 0;
}
