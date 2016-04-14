/** $Id$
 * $Revision: 24 $
 *
 * @copyright &copy; 2012 by Open Source Solutions Pty Ltd. All Rights Reserved
 * @file timer.c
 *
 * @author Denis Dowling
 * @date 09/12/2012
 *
 * @brief Timer library that aims to schedule one-shot and continuous timer
 * events from the main thread of control.
 *
 */
#include "timer.h"

#if defined(ARDUINO)
#include <Arduino.h>
#elif defined(__linux__)
#include <sys/time.h>
#endif

static struct Timer *timer_list = 0;

// Must provide an implementation for each architecture
uint32_t timer_current_time(void)
{
#if defined(ARDUINO)
    return millis();
#elif defined(__linux__)
    // Linuxversion.
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

// Do any locking and unlocking necessary for the timer functions to
// ensure that they can be called safely from whatever threads or interrupt
// contexts make sense.
static void timer_lock(void)
{
#if defined(ARDUINO)
    // Block interrupts
    cli();
#endif
}

static void timer_unlock(void)
{
#if defined(ARDUINO)
    // Enable interrupts
    sei();
#endif

}

void timer_schedule(Timer *timer, uint32_t timeout_time)
{
    // Stop the timer if it is currently running otherwise bad things happen
    // when timers are restarted
    timer_stop(timer);

    timer_lock();

    timer->timeout_time = timeout_time;

    Timer *p = timer_list;
    Timer *p_prev = 0;
    while(p != 0)
    {
	// Check if we need to insert before this entry
	if (p->timeout_time > timer->timeout_time)
	    break;

	p_prev = p;
	p = p->next;
    }

    // Insert the timer entry between p_prev and p
    timer->next = p;
    timer->prev = p_prev;
    if (p != 0)
	p->prev = timer;
    if (p_prev != 0)
	p_prev->next = timer;
    else
	timer_list = timer;

    timer_unlock();
}


void timer_start(Timer *timer)
{
    // No lock needed as timer_schedule does this
    timer_schedule(timer, timer_current_time() + timer->period);
}

// Stop the timer being careful not to cause damage if the timer is not
// actually running
void timer_stop(Timer *timer)
{
    if (!timer_is_running(timer))
	return;

    timer_lock();

    Timer *timer_prev = timer->prev;
    Timer *timer_next = timer->next;

    if (timer_prev != 0)
	timer_prev->next = timer_next;
    else if (timer_list == timer)
	timer_list = timer_next;

    if (timer_next != 0)
	timer_next->prev = timer_prev;

    timer->prev = 0;
    timer->next = 0;

    timer_unlock();
}

bool timer_dispatch_next(void)
{
    timer_lock();

    Timer *head = timer_list;
    bool head_timer_expired =
	(head != 0 && head->timeout_time <= timer_current_time());
    timer_unlock();

    if (head_timer_expired)
    {
	// Remove the timer from the list as the first thing as the callback
	// code might decide to add it again
	timer_stop(head);

	if (head->callback != 0)
	    head->callback(head);

	// If a continuous timer then reschedule for the expected timeout
	if (head->continuous)
	    timer_schedule(head, head->timeout_time + head->period);

	return true;
    }
    else
	return false;
}

int32_t timer_next_event(void)
{
    timer_lock();

    Timer *head = timer_list;
    uint32_t delta;
    if (head == 0)
	delta = 1000000; // FIXME
    else
	delta = head->timeout_time - timer_current_time();

    timer_unlock();

    return delta;
}

uint32_t timer_remaining(Timer *timer)
{
    uint32_t remaining;

    timer_lock();

    uint32_t t = timer_current_time();

    bool timer_running =
	(timer->next != 0 || timer->prev != 0 || timer_list == timer);

    if (!timer_running || t > timer->timeout_time)
	remaining = 0;
    else
	remaining = timer->timeout_time - t;

    timer_unlock();

    return remaining;
}

bool timer_is_running(Timer *timer)
{
    // A running timer must have a prev or next point as not null.
    timer_lock();
    bool timer_running =
	(timer->next != 0 || timer->prev != 0 || timer_list == timer);
    timer_unlock();

    return timer_running;
}

bool timer_all_done(void)
{
    return timer_list == 0;
}

