/**
 * ftq.c : Fixed Time Quantum microbenchmark
 *
 * Written by Matthew Sottile (matt@cs.uoregon.edu)
 *
 * This is a complete rewrite of the original tscbase code by
 * Ron and Matt, in order to use a better set of portable timers,
 * and more flexible argument handling and parameterization.
 *
 * 12/30/2007 : Updated to add pthreads support for FTQ execution on
 *              shared memory multiprocessors (multicore and SMPs).
 *              Also, removed unused TAU support.
 *
 * Licensed under the terms of the GNU Public Licence.  See LICENCE_GPL
 * for details.
 */
#include "ftq.h"

void 
usage(char *av0)
{
	fprintf(stderr, "usage: %s [-t threads] [-n samples] [-i bits] [-h] [-o outname] [-s]\n",
		av0);
	exit(EXIT_FAILURE);
}

int 
main(int argc, char **argv)
{
	/* local variables */
	char            fname_times[1024], fname_counts[1024], buf[32],
	                outname[255];
	int             i, j;
	int             numthreads = 1, use_threads = 0;
	int             fp;
	int             use_stdout = 0;
	int             rc;
	pthread_t      *threads;

	/* default output name prefix */
	sprintf(outname, "ftq");

	/*
         * getopt_long to parse command line options.
         * basically the code from the getopt man page.
         */
	while (1) {
		int             c;
		int             option_index = 0;
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"numsamples", 0, 0, 'n'},
			{"interval", 0, 0, 'i'},
			{"outname", 0, 0, 'o'},
			{"stdout", 0, 0, 's'},
			{"threads", 0, 0, 't'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "n:hsi:o:t:",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			numthreads = atoi(optarg);
			use_threads = 1;
			break;
		case 's':
			use_stdout = 1;
			break;
		case 'o':
			sprintf(outname, "%s", optarg);
			break;
		case 'i':
			interval_bits = atoi(optarg);
			break;
		case 'n':
			numsamples = atoi(optarg);
			break;
		case 'h':
		default:
			usage(argv[0]);
			break;
		}
	}

	/* sanity check */
	if (numsamples > MAX_SAMPLES) {
		fprintf(stderr, "WARNING: sample count exceeds maximum.\n");
		fprintf(stderr, "         setting count to maximum.\n");
		numsamples = MAX_SAMPLES;
	}
	/* allocate sample storage */
	samples = malloc(sizeof(unsigned long long) * numsamples * 2 * numthreads);
	assert(samples != NULL);

	if (interval_bits > MAX_BITS || interval_bits < MIN_BITS) {
		fprintf(stderr, "WARNING: interval bits invalid.  set to %d.\n",
			MAX_BITS);
		interval_bits = MAX_BITS;
	}
	if (use_threads == 1 && numthreads < 2) {
		fprintf(stderr, "ERROR: >1 threads required for multithread mode.\n");
		exit(EXIT_FAILURE);
	}
	if (use_threads == 1 && use_stdout == 1) {
		fprintf(stderr, "ERROR: cannot output to stdout for multithread mode.\n");
		exit(EXIT_FAILURE);
	}
	/*
	 * set up sampling.  first, take a few bogus samples to warm up the
	 * cache and pipeline
	 */
	interval_length = 1 << interval_bits;
	
	if (use_threads == 1) {
		threads = malloc(sizeof(pthread_t) * numthreads);
		assert(threads != NULL);

		for (i = 0; i < numthreads; i++) {
			rc = pthread_create(&threads[i], NULL, ftq_core, (void *) (intptr_t) i);
			if (rc) {
				fprintf(stderr, "ERROR: pthread_create() failed.\n");
				exit(EXIT_FAILURE);
			}
		}

		for (i = 0; i < numthreads; i++) {
			rc = pthread_join(threads[i], NULL);
			if (rc) {
				fprintf(stderr, "ERROR: pthread_join() failed.\n");
				exit(EXIT_FAILURE);
			}
		}

	} else {
		ftq_core(0);
	}

	if (use_stdout == 1) {
		for (i = 0; i < numsamples; i++) {
			fprintf(stdout, "%lld %lld\n", samples[i * 2], samples[i * 2 + 1]);
		}
	} else {

		for (j = 0; j < numthreads; j++) {
			sprintf(fname_times, "%s_%d_times.dat", outname, j);
			sprintf(fname_counts, "%s_%d_counts.dat", outname, j);

			fp = open(fname_times, O_CREAT | O_TRUNC | O_WRONLY, 0644);
			if (fp < 0) {
				perror("can not create file");
				exit(EXIT_FAILURE);
			}
			for (i = 0; i < numsamples; i++) {
				sprintf(buf, "%lld\n", samples[(i * 2) + (numsamples * j)]);
				write(fp, buf, strlen(buf));
			}
			close(fp);

			fp = open(fname_counts, O_CREAT | O_TRUNC | O_WRONLY, 0644);
			if (fp < 0) {
				perror("can not create file");
				exit(EXIT_FAILURE);
			}
			for (i = 0; i < numsamples; i++) {
				sprintf(buf, "%lld\n", samples[i * 2 + 1 + (numsamples * j)]);
				write(fp, buf, strlen(buf));
			}
			close(fp);
		}
	}

	free(samples);

	pthread_exit(NULL);

	exit(EXIT_SUCCESS);
}