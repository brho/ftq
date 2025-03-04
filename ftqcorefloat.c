// SPDX-License-Identifier: GPL-2.0-only
/**
 * ftq.c : Fixed Time Quantum microbenchmark
 *
 * Written by Matthew Sottile (mjsottile@gmail.com)
 *
 * This is a complete rewrite of the original tscbase code by
 * Ron and Matt, in order to use a better set of portable timers,
 * and more flexible argument handling and parameterization.
 *
 * 12/30/2007 : Updated to add pthreads support for FTQ execution on
 *              shared memory multiprocessors (multicore and SMPs).
 *              Also, removed unused TAU support.
 *
 * Licensed under the terms of the GNU Public License.  See LICENSE
 * for details.
 *
 * Keep this file OS-independent.
 */
#include "ftq.h"

/*************************************************************************
 * All time base here is in ticks; computation to ns is done elsewhere   *
 * as needed.                                                            *
 *************************************************************************/

unsigned long main_loops(struct sample *samples, size_t numsamples,
                         ticks tickinterval, int offset)
{
	int k;
	unsigned long done;
	volatile double count;
	double total_count = 0;
	ticks ticknow, ticklast, tickend;

	tickend = getticks();

	for (done = 0; done < numsamples; done++) {
		count = 0.0;
		tickend += tickinterval;

		for (ticknow = ticklast = getticks();
			 ticknow < tickend; ticknow = getticks()) {
			for (k = 0; k < ITERCOUNT; k++)
				count++;
			for (k = 0; k < (ITERCOUNT - 1); k++)
				count--;
		}

		samples[done + offset].ticklast = ticklast;
		samples[done + offset].count = (unsigned long long)count;
		total_count += (unsigned long long) count;
	}
	return total_count;
}
