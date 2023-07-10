/*
    Copyright (C) 2023 Jean Kieffer

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "acb_theta.h"

int main(void)
{
    slong iter;
    flint_rand_t state;

    flint_printf("siegel_cocycle....");
    fflush(stdout);

    flint_randinit(state);

    for (iter = 0; iter < 10000 * 0.1 * flint_test_multiplier(); iter++)
    {
        slong g = 1 + n_randint(state, 10);
        fmpz_mat_t m1, m2, m3;
        acb_mat_t tau1, tau2;
        acb_mat_t c1, c2, c3, t;
        slong prec = 100 + n_randint(state, 200);
        slong mag_bits = n_randint(state, 10);

        fmpz_mat_init(m1, 2 * g, 2 * g);
        fmpz_mat_init(m2, 2 * g, 2 * g);
        fmpz_mat_init(m3, 2 * g, 2 * g);
        acb_mat_init(tau1, g, g);
        acb_mat_init(tau2, g, g);
        acb_mat_init(c1, g, g);
        acb_mat_init(c2, g, g);
        acb_mat_init(c3, g, g);
        acb_mat_init(t, g, g);

        acb_siegel_randtest(tau1, state, prec, mag_bits);
        acb_siegel_randtest(tau2, state, prec, mag_bits);
        sp2gz_randtest(m1, state, mag_bits);
        sp2gz_randtest(m2, state, mag_bits);

        /* Test: chain rule */
        acb_siegel_cocycle(c1, m1, tau1, prec);
        acb_siegel_transform(tau2, m1, tau1, prec);
        acb_siegel_cocycle(c2, m2, tau2, prec);
        fmpz_mat_mul(m3, m2, m1);
        acb_siegel_cocycle(c3, m3, tau1, prec);
        acb_mat_mul(t, c2, c1, prec);

        if (!acb_mat_overlaps(t, c3))
        {            
            flint_printf("FAIL\n\n");
            acb_mat_printd(c3, 10);
            flint_printf("\n");
            acb_mat_printf(t, 10);
            flint_printf("\n");
            flint_abort();
        }

        fmpz_mat_clear(m1);
        fmpz_mat_clear(m2);
        fmpz_mat_clear(m3);
        acb_mat_clear(tau1);
        acb_mat_clear(tau2);
        acb_mat_clear(c1);
        acb_mat_clear(c2);
        acb_mat_clear(c3);
        acb_mat_clear(t);
    }

    flint_randclear(state);
    flint_cleanup();
    flint_printf("PASS\n");
    return 0;
}
