#include <Arduino.h>
extern "C"
{
#include "timer.h"
}

int green_led = 8;
int red_led = 9;

void callback1(Timer *t)
{
    if (!digitalRead(green_led))
	digitalWrite(green_led, 1);
    else
	digitalWrite(green_led, 0);
}

// Timers must be declared within a scope that will exist for their lifetime.
// Generally this means they must be declared with global scope or from
// within the main function
Timer timer1 = {
    /*.period =*/ 100,
    /*.callback =*/ callback1,
    /*.continuous =*/ true,
};

void callback2(Timer *t)
{
    if (!digitalRead(red_led))
	digitalWrite(red_led, 1);
    else
	digitalWrite(red_led, 0);
}

Timer timer2 = {
    /*.period =*/ 101,
    /*.callback =*/ callback2,
    /*.continuous =*/ true,
};

void setup()
{
    Serial.begin(115200);

    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);

    timer_start(&timer1);
    timer_start(&timer2);
}

void loop()
{
    timer_dispatch_next();

#if 0
    Serial.print("Time : ");
    Serial.print(timer_current_time());
    Serial.println();
#endif
}
