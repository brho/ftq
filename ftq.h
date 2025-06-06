// SPDX-License-Identifier: GPL-2.0-only
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

/*
 * use cycle timers from FFTW3 (http://www.fftw.org/).  this defines a
 * "ticks" typedef, usually unsigned long long, that is used to store
 * timings.  currently this code will NOT work if the ticks typedef is
 * something other than an unsigned long long.
 */
#include "cycle.h"

/** defaults **/
#define MAX_SAMPLES    2000000
#define DEFAULT_COUNT  524288
#define DEFAULT_INTERVAL 100000
#define MAX_BITS       30
#define MIN_BITS       3
#define DEFAULT_OUTNAME "ftq"

/**
 * Work grain, which is now fixed. 
 */
#define ITERCOUNT      32

extern int ignore_wire_failures;

struct sample {
	unsigned long long ticklast;
	unsigned long long count;
};

/* ftqcore.c */
unsigned long main_loops(struct sample *samples, size_t numsamples,
                         ticks tickinterval, int offset);

/* must be provided by OS code */
/* Sorry, Plan 9; don't know how to manage FILE yet */
ticks nsec_ticks(void);
void osinfo(FILE * f, int core);
int threadinit(int numthreads);
double compute_ticksperns(void);
/* known to be doable in Plan 9 and Linux. Akaros? not sure. */
int wireme(int core);

int get_num_cores(void);
int get_coreid(void);
void set_sched_realtime(void);
struct sample *allocate_samples(size_t samples_size);
