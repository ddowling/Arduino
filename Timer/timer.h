/** $Id$
 * $Revision: 24 $
 *
 * @copyright &copy; 2012 by Open Source Solutions Pty Ltd. All Rights Reserved
 * @file timer.h
 *
 * @author Denis Dowling
 * @date 09/12/2012
 *
 * @brief Timer library that aims to schedule one-shot and continuous timer
 * events from the main thread of control.
 *
 */
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Timer Timer;

typedef void (*TimerCallback)(Timer *t);

struct Timer {
    uint32_t period;
    TimerCallback callback;
    bool continuous;
    uint32_t timeout_time; // Private. Set by timer_start and timer_stop
    // Timers are maintained in a linked list. Keep next and prev to make
    // removal quick
    Timer *next;
    Timer *prev;
};

void timer_start(Timer *timer);

void timer_stop(Timer *timer);

/**
 * Schedule a timer to expire at a given timeout time rather than basing
 * timeout on the current time plus the period.
 */
void timer_schedule(Timer *timer, uint32_t timeout_time);

bool timer_dispatch_next(void);

bool timer_is_running(Timer *timer);

uint32_t timer_remaining(Timer *timer);

// Return the current time in milliseconds for the timer system
uint32_t timer_current_time(void);

// Return true if all timers have stopped
bool timer_all_done(void);

int32_t timer_next_event(void);

#endif
