/*
    Copyright (C) 2009, 2010, 2015 William Hart

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "flint.h"
#include "ulong_extras.h"

/*
   Method of Niels Moller and Torbjorn Granlund see paper:
   Improved Division by Invariant Integers: (algorithm 4)
   https://gmplib.org/~tege/division-paper.pdf
*/

#define r_shift(in, c) (((c) == FLINT_BITS) ? WORD(0) : ((in) >> (c)))

ulong
n_mod2_preinv(ulong a, ulong n, ulong ninv)
{
    ulong norm, q1, q0, r;

    FLINT_ASSERT(n != 0);

    norm = flint_clz(n);
    n <<= norm;

    {
        const ulong u1 = r_shift(a, FLINT_BITS - norm);
        const ulong u0 = (a << norm);

        umul_ppmm(q1, q0, ninv, u1);
        add_ssaaaa(q1, q0, q1, q0, u1, u0);

        r = u0 - (q1 + 1) * n;

        if (r > q0)
            r += n;

        return (r < n) ? (r >> norm) : ((r - n) >> norm);
    }
}
