/*
    Copyright (C) 2013 William Hart
    Copyright (C) 2024 Fredrik Johansson

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "flint.h"
#include "mpn_extras.h"

/*
TODO:
 * fixed-length code for small n
 * use unbalanced mulhigh in the second loop
 * use mullow
*/

mp_limb_t flint_mpn_divrem_preinvn(mp_ptr qp, mp_ptr rp, mp_srcptr ap, mp_size_t m,
                          mp_srcptr d, mp_size_t n, mp_srcptr dinv)
{
   mp_limb_t cy, hi = 0;
   mp_ptr t, q, r, a;
   mp_size_t size;
   TMP_INIT;

   a = (mp_ptr) ap + m - 2*n;
   r = rp + m - 2*n;

   /* check if top n limbs of a exceed d */
   if (mpn_cmp(a + n, d, n) >= 0)
   {
      mpn_sub_n(r + n, a + n, d, n);
      hi = 1;
   } else if (r != a)
      mpn_copyi(r + n, a + n, n);

   q = qp + m - 2*n;

   TMP_START;
   t = TMP_ALLOC(2*n*sizeof(mp_limb_t));

   /* 2n by n division */
   while (m >= 2*n)
   {
      flint_mpn_mul_or_mulhigh_n(t, dinv, r + n, n);
      cy = mpn_add_n(q, t + n, r + n, n);

      /* note: we rely on the fact that mul_or_mullow_n actually
               writes at least n + 1 limbs */
      flint_mpn_mul_or_mullow_n(t, d, q, n);
      cy = r[n] - t[n] - mpn_sub_n(r, a, t, n);

      while (cy > 0)
      {
         cy -= mpn_sub_n(r, r, d, n);
         mpn_add_1(q, q, n, 1);
      }

      if (mpn_cmp(r, d, n) >= 0)
      {
         mpn_sub_n(r, r, d, n);
         mpn_add_1(q, q, n, 1);
      }

      FLINT_ASSERT(mpn_cmp(r, d, n) < 0);

      m -= n;
      r -= n;
      a -= n;
      q -= n;
   }

   size = m - n;

   /* m by n division with 2n > m > n */
   if (size)
   {
      if (rp != ap)
         mpn_copyi(rp, ap, size);

      flint_mpn_mul_or_mulhigh_n(t + n - size, dinv + n - size, rp + n, size);
      cy = mpn_add_n(qp, t + n, rp + n, size);

      flint_mpn_mul(t, d, n, qp, size);
      if (cy)
         mpn_add_n(t + size, t + size, d, n + 1 - size);

      cy = rp[n] - t[n] - mpn_sub_n(rp, rp, t, n);

      while (cy > 0)
      {
         cy -= mpn_sub_n(rp, rp, d, n);
         mpn_add_1(qp, qp, size, 1);
      }

      if (mpn_cmp(rp, d, n) >= 0)
      {
         mpn_sub_n(rp, rp, d, n);
         mpn_add_1(qp, qp, size, 1);
      }

      FLINT_ASSERT(mpn_cmp(rp, d, n) < 0);
   }

   TMP_END;

   return hi;
}
