/*
    Copyright (C) 2012 Fredrik Johansson

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "flint.h"
#include "ulong_extras.h"
#include "fmpz.h"

ulong
fmpz_abs_lbound_ui_2exp(slong * exp, const fmpz_t x, int bits)
{
    ulong m;
    slong shift, e, size;
    fmpz c = *x;

    if (!COEFF_IS_MPZ(c))
    {
        m = FLINT_ABS(c);
        e = 0;
    }
    else
    {
        mpz_ptr z = COEFF_TO_PTR(c);
        size = z->_mp_size;
        size = FLINT_ABS(size);
        e = (size - 1) * FLINT_BITS;

        if (size == 1)
        {
            m = z->_mp_d[0];
        }
        else   /* there are two or more limbs */
        {
            /* top limb (which must be nonzero) */
            m = z->_mp_d[size - 1];

            shift = flint_clz(m);
            shift = FLINT_BITS - shift - bits;
            e += shift;

            if (shift >= 0)
            {
                m >>= shift;
            }
            else
            {
                /* read a second limb to get an accurate value */
                ulong m2 = z->_mp_d[size - 2];
                m = (m << (-shift)) | (m2 >> (FLINT_BITS + shift));
            }

            *exp = e;
            return m;
        }
    }

    shift = flint_clz(m);
    e += FLINT_BITS - shift - bits;
    if (e >= 0)
        m >>= e;
    else
        m <<= (-e);
    *exp = e;
    return m;
}
