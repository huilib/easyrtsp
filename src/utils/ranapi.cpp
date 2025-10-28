

#include "ranapi.h"
#include <stdio.h>


#define	TYPE_0		0		/* linear congruential */
#define	BREAK_0		8
#define	DEG_0		0
#define	SEP_0		0

#define	TYPE_1		1		/* x**7 + x**3 + 1 */
#define	BREAK_1		32
#define	DEG_1		7
#define	SEP_1		3

#define	TYPE_2		2		/* x**15 + x + 1 */
#define	BREAK_2		64
#define	DEG_2		15
#define	SEP_2		1

#define	TYPE_3		3		/* x**31 + x**3 + 1 */
#define	BREAK_3		128
#define	DEG_3		31
#define	SEP_3		3

#define	TYPE_4		4		/* x**63 + x + 1 */
#define	BREAK_4		256
#define	DEG_4		63
#define	SEP_4		1

/*
 * Array versions of the above information to make code run faster --
 * relies on fact that TYPE_i == i.
 */
#define	MAX_TYPES	5		/* max number of types above */

static int const degrees[MAX_TYPES] = { DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 };
static int const seps [MAX_TYPES] = { SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 };



static long randtbl[DEG_3 + 1] = {
	TYPE_3,
	0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342, 0xde3b81e0, 0xdf0a6fb5,
	0xf103bc02, 0x48f340fb, 0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
	0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86, 0xda672e2a, 0x1588ca88,
	0xe369735d, 0x904f35f7, 0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc,
	0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b, 0xf5ad9d0e, 0x8999220b,
	0x27fb47b9,
};

static long* fptr = &randtbl[SEP_3 + 1];
static long* rptr = &randtbl[1];

static long *state = &randtbl[1];
static int rand_type = TYPE_3;
static int rand_deg = DEG_3;
static int rand_sep = SEP_3;
static long* end_ptr = &randtbl[DEG_3 + 1];

long our_random(void); /*forward*/
void
our_srandom(unsigned int x)
{
	int i;

	if (rand_type == TYPE_0)
		state[0] = x;
	else {
		state[0] = x;
		for (i = 1; i < rand_deg; i++)
			state[i] = 1103515245 * state[i - 1] + 12345;
		fptr = &state[rand_sep];
		rptr = &state[0];
		for (i = 0; i < 10 * rand_deg; i++)
			(void)our_random();
	}
}


long our_random() {
  long i;

  if (rand_type == TYPE_0) {
    i = state[0] = (state[0] * 1103515245 + 12345) & 0x7fffffff;
  } else {
    /* Make copies of "rptr" and "fptr" before working with them, in case we're being called concurrently by multiple threads: */
    long* rp = rptr;
    long* fp = fptr;

    /* Make sure "rp" and "fp" are separated by the correct distance (again, allowing for concurrent access): */
    if (!(fp == rp+SEP_3 || fp+DEG_3 == rp+SEP_3)) {
      /* A rare case that should occur only if we're being called concurrently by multiple threads. */
      /* Restore the proper separation between the pointers: */
      if (rp <= fp) rp = fp-SEP_3; else rp = fp+DEG_3-SEP_3;
    }

    *fp += *rp;
    i = (*fp >> 1) & 0x7fffffff;	/* chucking least random bit */
    if (++fp >= end_ptr) {
      fp = state;
      ++rp;
    } else if (++rp >= end_ptr) {
      rp = state;
    }

    /* Restore "rptr" and "fptr" from our working copies: */
    rptr = rp;
    fptr = fp;
  }

  return i;
}


u_int32_t our_random32() {

  long random_1 = our_random();
  u_int32_t random16_1 = (u_int32_t)(random_1&0x00FFFF00);

  long random_2 = our_random();
  u_int32_t random16_2 = (u_int32_t)(random_2&0x00FFFF00);

  return (random16_1<<8) | (random16_2>>8);
}

