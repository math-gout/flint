/*
    Copyright (C) 2010, 2011, 2018, 2024 Fredrik Johansson
    Copyright (C) 2016 Aaditya Thakkar

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "fmpz.h"
#include "fmpz_mat.h"

#if FLINT_USES_BLAS
# include "longlong.h"
#endif

void _fmpz_mat_mul_small_1(fmpz_mat_t C, const fmpz_mat_t A, const fmpz_mat_t B)
{
    slong ar, br, bc;
    slong i, j, k;

    ar = fmpz_mat_nrows(A);
    br = fmpz_mat_nrows(B);
    bc = fmpz_mat_ncols(B);

    fmpz_mat_zero(C);

    for (i = 0; i < ar; i++)
    {
        for (j = 0; j < bc; j++)
        {
            slong s = 0;

            for (k = 0; k < br; k++)
                s += *fmpz_mat_entry(A, i, k) * (*fmpz_mat_entry(B, k, j));

            *fmpz_mat_entry(C, i, j) = s;
        }
    }
}

void _fmpz_mat_mul_small_2a(fmpz_mat_t C, const fmpz_mat_t A, const fmpz_mat_t B)
{
    slong ar, br, bc;
    slong i, j, k;

    ar = fmpz_mat_nrows(A);
    br = fmpz_mat_nrows(B);
    bc = fmpz_mat_ncols(B);

    for (i = 0; i < ar; i++)
    {
        for (j = 0; j < bc; j++)
        {
            ulong hi, lo, shi, slo;
            slong x, y;

            shi = slo = 0;

            for (k = 0; k < br; k++)
            {
                x = *fmpz_mat_entry(A, i, k);
                y = *fmpz_mat_entry(B, k, j);

                smul_ppmm(hi, lo, x, y);
                add_ssaaaa(shi, slo, shi, slo, hi, lo);
            }

            fmpz_set_signed_uiui(fmpz_mat_entry(C, i, j), shi, slo);
        }
    }
}

void _fmpz_mat_mul_small_2b(fmpz_mat_t C, const fmpz_mat_t A, const fmpz_mat_t B)
{
    slong ar, br, bc;
    slong i, j, k;

    ar = fmpz_mat_nrows(A);
    br = fmpz_mat_nrows(B);
    bc = fmpz_mat_ncols(B);

    for (i = 0; i < ar; i++)
    {
        for (j = 0; j < bc; j++)
        {
            ulong hi, lo, shi, smid, slo;
            slong x, y;

            shi = smid = slo = 0;

            for (k = 0; k < br; k++)
            {
                x = *fmpz_mat_entry(A, i, k);
                y = *fmpz_mat_entry(B, k, j);

                smul_ppmm(hi, lo, x, y);
                add_sssaaaaaa(shi, smid, slo, shi, smid, slo,
                              FLINT_SIGN_EXT(hi), hi, lo);
            }

            fmpz_set_signed_uiuiui(fmpz_mat_entry(C, i, j), shi, smid, slo);
        }
    }
}

void
fmpz_mat_mul(fmpz_mat_t C, const fmpz_mat_t A, const fmpz_mat_t B)
{
    slong ar, br, bc;
    slong abits, bbits;
    flint_bitcnt_t cbits;
    slong i, j, dim, limit;
    int sign;

    FLINT_ASSERT(C->r == A->r);
    FLINT_ASSERT(C->c == B->c);
    FLINT_ASSERT(A->c == B->r);

    ar = fmpz_mat_nrows(A);
    br = fmpz_mat_nrows(B);
    bc = fmpz_mat_ncols(B);

    if (ar == 0 || br == 0 || bc == 0)
    {
        fmpz_mat_zero(C);
        return;
    }

    if (C == A || C == B)
    {
        fmpz_mat_t T;
        fmpz_mat_init(T, ar, bc);
        fmpz_mat_mul(T, A, B);
        fmpz_mat_swap_entrywise(C, T);
        fmpz_mat_clear(T);
        return;
    }

    if (br == 1)
    {
        fmpz_mat_mul_classical(C, A, B);
        return;
    }

    /* todo: use Strassen, Waksman or FFT when entries are huge
       and the matrix is not structured */
    if (br == 2)
    {
        for (i = 0; i < ar; i++)
            for (j = 0; j < bc; j++)
                fmpz_fmma(fmpz_mat_entry(C, i, j),
                    fmpz_mat_entry(A, i, 0), fmpz_mat_entry(B, 0, j),
                    fmpz_mat_entry(A, i, 1), fmpz_mat_entry(B, 1, j));
        return;
    }

    dim = FLINT_MIN(ar, bc);
    dim = FLINT_MIN(dim, br);

    /* TODO: for space reasons maybe just call strassen here if dim > 10000 */

    abits = fmpz_mat_max_bits(A);
    bbits = fmpz_mat_max_bits(B);

    sign = 0;
    if (abits < 0)
    {
        sign = 1;
        abits = -abits;
    }

    if (bbits < 0)
    {
        sign = 1;
        bbits = -bbits;
    }

    if (abits == 0 || bbits == 0)
    {
        fmpz_mat_zero(C);
        return;
    }

    cbits = abits + bbits + FLINT_BIT_COUNT(br);

#if FLINT_USES_BLAS && FLINT_BITS == 64
    if (dim > 50)
    {
        if (cbits <= 53)
            limit = 0;
        else if (cbits <= 64)
            limit = 1000;
        else if (cbits <= 128)
            limit = 1300;
        else if (cbits <= 256)
            limit = 250;
        else
            limit = 200 + 8*FLINT_BIT_COUNT(cbits);

        if (dim > limit && _fmpz_mat_mul_blas(C, A, abits, B, bbits, sign, cbits))
            return;
    }
#endif

    if (abits <= SMALL_FMPZ_BITCOUNT_MAX && bbits <= SMALL_FMPZ_BITCOUNT_MAX)
    {
        /*
            A and B are small fmpz's: strassen is effective at large dimension
            either through small fmpz's or through multi_mod. The cutoff
            for multi_mod's strassen has not been observed, but it must exist
            somewhere past 4000.
        */

        /* first take care of small cases */
        if (ar < 9 || ar + br < 20)
        {
            if (cbits <= SMALL_FMPZ_BITCOUNT_MAX)
                _fmpz_mat_mul_small_1(C, A, B);
            else if (cbits <= 2*FLINT_BITS - 1)
                _fmpz_mat_mul_small_2a(C, A, B);
            else
                _fmpz_mat_mul_small_2b(C, A, B);

            return;
        }

        if (dim > 1000)
        {
            /* do more mul_small with more threads */
            limit = 300*flint_get_num_threads();

            if (cbits <= SMALL_FMPZ_BITCOUNT_MAX && dim - 1000 > limit)
            {
                /* strassen avoids big fmpz intermediates */
                fmpz_mat_mul_strassen(C, A, B);
                return;
            }
            else if (cbits > SMALL_FMPZ_BITCOUNT_MAX && dim - 4000 > limit)
            {
                _fmpz_mat_mul_multi_mod(C, A, B, sign, cbits);
                return;
            }
        }

        _fmpz_mat_mul_small_internal(C, A, B, cbits);
        return;
    }
    else if (abits + sign <= 2*FLINT_BITS && bbits + sign <= 2*FLINT_BITS)
    {
        /*
            both A and B fit into two words: the complexity of mul_double_word
            does not depend on cbits much and hence does better as cbits
            increases, but mul_multi_mod eventually does strassen.
        */

        /* mul_double_word handles unsigned input better than signed input. */
        if (sign)
            dim = 2*dim;

        if (dim > 300)
        {
            /* do more mul_double_word with more threads and more cbits */
            limit = (cbits - 2*FLINT_BITS)/8;
            limit = limit*limit*flint_get_num_threads();
            if (dim - 300 > limit)
            {
                _fmpz_mat_mul_multi_mod(C, A, B, sign, cbits);
                return;
            }
        }

        _fmpz_mat_mul_double_word_internal(C, A, B, sign, cbits);
        return;
    }
    else
    {
        /* want balanced entries for mul_waksman */
        /* todo: should check for structured matrices (favor mul_classical) */
        slong min_bits = FLINT_MIN(abits, bbits);
        slong max_bits = FLINT_MAX(abits, bbits);

        if (dim >= 3 * (slong) FLINT_BIT_COUNT(cbits))  /* tuning param */
            _fmpz_mat_mul_multi_mod(C, A, B, sign, cbits);
        else if (dim < 20 && ((dim == 2 && min_bits >= 5000 && max_bits <= 1.1 * min_bits)
                              || (max_bits <= 1.6 * min_bits && ((dim == 3 && min_bits >= 3000)
                              || (dim >= 4 && min_bits >= 1000)
                              || (dim >= 12 && min_bits >= 500)))))
            fmpz_mat_mul_waksman(C, A, B);
        else if (abits >= 500 && bbits >= 500 && dim >= 8)  /* tuning param */
            fmpz_mat_mul_strassen(C, A, B);
        else
            fmpz_mat_mul_classical(C, A, B);
    }
}
