/*
    Copyright 2010 William Hart

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include <math.h>
#include "profiler.h"
#include "flint.h"
#include "ulong_extras.h"
#include "fmpz.h"
#include "fmpz_poly.h"
#include "fmpq_poly.h"
#include "nf.h"
#include "nf_elem.h"

#define BITS 10

typedef struct
{
   slong length;
   int monic;
} info_t;

void random_fmpq_poly(fmpq_poly_t pol, flint_rand_t state, slong length)
{
   fmpz * arr;
   slong i;

   fmpq_poly_fit_length(pol, length);

   arr = fmpq_poly_numref(pol);

   for (i = 0; i < length; i++)
      fmpz_randbits(arr + i, state, BITS);

   fmpz_randbits(fmpq_poly_denref(pol), state, BITS);

   _fmpq_poly_set_length(pol, length);
   _fmpq_poly_normalise(pol);
   fmpq_poly_canonicalise(pol);
}

void random_nf_elem(nf_elem_t a, flint_rand_t state, nf_t nf)
{
   slong len = nf->pol->length - 1;

   random_fmpq_poly(NF_ELEM(a), state, len);
}

void sample(void * arg, ulong count)
{
   info_t * info = (info_t *) arg;
   slong length = info->length, i, j;
   int monic = info->monic;
   int scale;

   flint_rand_t state;

   fmpq_poly_t pol;
   nf_t nf;
   nf_elem_t a, b, c;

   flint_rand_init(state);

   scale = 100;
   if (length >= 50) scale = 10;
   if (length >= 500) scale = 4;

   fmpq_poly_init(pol);

   for (i = 0; i < count; i++)
   {
      random_fmpq_poly(pol, state, length);
      if (monic)
      {
         fmpz_one(fmpq_poly_denref(pol));
         fmpq_poly_set_coeff_ui(pol, length - 1, 1);
      }

      nf_init(nf, pol);

      nf_elem_init(a, nf);
      nf_elem_init(b, nf);
      nf_elem_init(c, nf);

      random_nf_elem(a, state, nf);
      random_nf_elem(b, state, nf);
      if (monic)
      {
         fmpz_one(fmpq_poly_denref(NF_ELEM(a)));
         fmpz_one(fmpq_poly_denref(NF_ELEM(b)));
      }

      prof_start();
      for (j = 0; j < scale; j++)
      {
         nf_elem_mul(c, a, b, nf);
      }
	   prof_stop();
   }

   nf_elem_clear(a, nf);
   nf_elem_clear(b, nf);
   nf_elem_clear(c, nf);

   nf_clear(nf);

   fmpq_poly_clear(pol);

   flint_rand_clear(state);
}

int main(void)
{
   double min, max;
   info_t info;
   slong k, scale;

   printf("Number field element multiplication\n");
   flint_printf("bits = %ld\n", BITS);

   for (k = 4; k <= 1000; k = (slong) ceil(1.1*k))
   {
      info.length = k;
      info.monic = 0;

      scale = 100;
      if (k >= 50) scale = 10;
      if (k >= 500) scale = 4;

      prof_repeat(&min, &max, sample, (void *) &info);

      flint_printf("length %wd, min %.3e us, max %.3e us\n",
           info.length,
		   (min/scale),
           (max/scale)
	     );

     info.monic = 1;

     prof_repeat(&min, &max, sample, (void *) &info);

      flint_printf("length %wd, min %.3e us, max %.3e us\n",
           info.length,
		   (min/scale),
           (max/scale)
	     );
   }

   return 0;
}
