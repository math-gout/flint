/*
    Copyright (C) 2009 William Hart

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "flint.h"
#include "ulong_extras.h"

ulong n_mod_precomp(ulong a, ulong n, double npre)
{
    ulong quot, rem;

    quot = (ulong) ((double) a * npre);
    rem  = a - quot*n;
    if ((slong) rem < 0) /* unlikely */
       rem += n;
    return rem - (n & (((slong) (n - rem - 1)) >> (FLINT_BITS-1)));
}
