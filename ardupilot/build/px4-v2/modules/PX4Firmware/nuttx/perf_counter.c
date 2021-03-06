/*
 * perf_counter.c
 *
 *  Created on: 2019Äê2ÔÂ12ÈÕ
 *      Author: Administrator
 */


#include "perf_counter.h"
#include <rtthread.h>

/**
 * Create a new local counter.
 *
 * @param type			The type of the new counter.
 * @param name			The counter name.
 * @return			Handle for the new counter, or NULL if a counter
 *				could not be allocated.
 */
perf_counter_t	perf_alloc(enum perf_counter_type type, const char *name)
{
	//TODO
	return RT_NULL;
}

/**
 * Get the reference to an existing counter or create a new one if it does not exist.
 *
 * @param type			The type of the counter.
 * @param name			The counter name.
 * @return			Handle for the counter, or NULL if a counter
 *				could not be allocated.
 */
perf_counter_t	perf_alloc_once(enum perf_counter_type type, const char *name)
{
	//TODO
	return RT_NULL;
}

/**
 * Free a counter.
 *
 * @param handle		The performance counter's handle.
 */
void		perf_free(perf_counter_t handle)
{
	//TODO
}

/**
 * Count a performance event.
 *
 * This call only affects counters that take single events; PC_COUNT, PC_INTERVAL etc.
 *
 * @param handle		The handle returned from perf_alloc.
 */
void		perf_count(perf_counter_t handle)
{
	//TODO
}

/**
 * Begin a performance event.
 *
 * This call applies to counters that operate over ranges of time; PC_ELAPSED etc.
 *
 * @param handle		The handle returned from perf_alloc.
 */
void		perf_begin(perf_counter_t handle)
{
	//TODO
}

/**
 * End a performance event.
 *
 * This call applies to counters that operate over ranges of time; PC_ELAPSED etc.
 * If a call is made without a corresponding perf_begin call, or if perf_cancel
 * has been called subsequently, no change is made to the counter.
 *
 * @param handle		The handle returned from perf_alloc.
 */
void		perf_end(perf_counter_t handle)
{
	//TODO
}

/**
 * Register a measurement
 *
 * This call applies to counters that operate over ranges of time; PC_ELAPSED etc.
 * If a call is made without a corresponding perf_begin call. It sets the
 * value provided as argument as a new measurement.
 *
 * @param handle		The handle returned from perf_alloc.
 * @param elapsed		The time elapsed. Negative values lead to incrementing the overrun counter.
 */
void		perf_set(perf_counter_t handle, int64_t elapsed)
{
	//TODO
}

/**
 * Cancel a performance event.
 *
 * This call applies to counters that operate over ranges of time; PC_ELAPSED etc.
 * It reverts the effect of a previous perf_begin.
 *
 * @param handle		The handle returned from perf_alloc.
 */
void		perf_cancel(perf_counter_t handle)
{
	//TODO
}

/**
 * Reset a performance counter.
 *
 * This call resets performance counter to initial state
 *
 * @param handle		The handle returned from perf_alloc.
 */
void		perf_reset(perf_counter_t handle)
{
	//TODO
}

/**
 * Print one performance counter to stdout
 *
 * @param handle		The counter to print.
 */
void		perf_print_counter(perf_counter_t handle)
{
	//TODO
}

/**
 * Print one performance counter to a fd.
 *
 * @param fd			File descriptor to print to - e.g. 0 for stdout
 * @param handle		The counter to print.
 */
void		perf_print_counter_fd(int fd, perf_counter_t handle)
{
	//TODO
}

/**
 * Print all of the performance counters.
 *
 * @param fd			File descriptor to print to - e.g. 0 for stdout
 */
void		perf_print_all(int fd)
{
	//TODO
}

/**
 * Print hrt latency counters.
 *
 * @param fd			File descriptor to print to - e.g. 0 for stdout
 */
void		perf_print_latency(int fd)
{
	//TODO
}

/**
 * Reset all of the performance counters.
 */
void		perf_reset_all(void)
{
	//TODO
}

/**
 * Return current event_count
 *
 * @param handle		The counter returned from perf_alloc.
 * @return			event_count
 */
uint64_t	perf_event_count(perf_counter_t handle)
{
	//TODO
	return 0;
}
