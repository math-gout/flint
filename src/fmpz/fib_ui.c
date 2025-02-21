/*
    Copyright (C) 2010 Fredrik Johansson

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "flint.h"
#include "gmpcompat.h"
#include "ulong_extras.h"
#include "fmpz.h"

#if FLINT64
#define NUM_SMALL_FIB 94
#define NUM_SMALL_FIB2 184
#else
#define NUM_SMALL_FIB 48
#define NUM_SMALL_FIB2 92
#endif

static const ulong small_fib[NUM_SMALL_FIB] =
{
    0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987,
    1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393,
    196418, 317811, 514229, 832040, 1346269, 2178309, 3524578, 5702887,
    9227465, 14930352, 24157817, 39088169, 63245986, 102334155,
    165580141, 267914296, 433494437, 701408733, 1134903170, 1836311903,
    UWORD(2971215073),
#if FLINT64
    UWORD(4807526976), UWORD(7778742049), UWORD(12586269025), UWORD(20365011074), UWORD(32951280099),
    UWORD(53316291173), UWORD(86267571272), UWORD(139583862445), UWORD(225851433717),
    UWORD(365435296162), UWORD(591286729879), UWORD(956722026041), 1548008755920,
    UWORD(2504730781961), UWORD(4052739537881), UWORD(6557470319842), 10610209857723,
    UWORD(17167680177565), UWORD(27777890035288), UWORD(44945570212853), 72723460248141,
    UWORD(117669030460994), UWORD(190392490709135), UWORD(308061521170129), 498454011879264,
    UWORD(806515533049393), UWORD(1304969544928657), UWORD(2111485077978050),
    UWORD(3416454622906707), UWORD(5527939700884757), UWORD(8944394323791464),
    UWORD(14472334024676221), UWORD(23416728348467685), UWORD(37889062373143906),
    UWORD(61305790721611591), UWORD(99194853094755497), UWORD(160500643816367088),
    UWORD(259695496911122585), UWORD(420196140727489673), UWORD(679891637638612258),
    UWORD(1100087778366101931), UWORD(1779979416004714189), UWORD(2880067194370816120),
    UWORD(4660046610375530309), UWORD(7540113804746346429), UWORD(12200160415121876738)
#endif
};

void fmpz_fib_ui(fmpz_t f, ulong n)
{
    if (n < NUM_SMALL_FIB)
    {
        fmpz_set_ui(f, small_fib[n]);
    }
    else if (n < NUM_SMALL_FIB2)
    {
        ulong hi, lo, a, b;
        a = small_fib[n / 2];
        b = small_fib[n / 2 - 1];
        if (n & 1)
        {
            umul_ppmm(hi, lo, 2 * a + b, 2 * a - b);
            if ((n / 2) % 2 == 0)
                lo += 2;
            else
                lo -= 2;
        }
        else
        {
            umul_ppmm(hi, lo, a, a + 2 * b);
        }
        fmpz_set_uiui(f, hi, lo);
    }
    else
    {
        flint_mpz_fib_ui(_fmpz_promote(f), n);
    }
}
