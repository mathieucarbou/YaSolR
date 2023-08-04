---
layout: default
title: Blog
description: Everything on the JSY
---

_Date: 2024-06-26_

# Everything on the JSY

I am developing quite a few projects and librairies in the Arduino / ESP32 / Home automation landscape ([https://mathieu.carbou.me](https://mathieu.carbou.me)), including YaSolR routing software as well.

Last year I created a specialized library for the JSY, which I wanted to talk to you about today in order to present its operation and use in the context of a solar router.

## How the JSY is used in a solar router

A solar router needs to measure the current, to react to these measurements and propose a new duty cycle to apply to the dimmer, for a certain time, until it obtains a new measurement.
So it depends on the speed at which these measurements are taken, and their precision, but also of course on the routing algorithm used and the precision of the steps (0-100, 8-bit, 12-bit, etc.).

It's rare to find a solar router that correctly uses JSY at its full capacity, partly due to the severe lack of an effective library for JSY, and that's why I created it.

## Asynchronous operation

As JSY slows down routing, it is important to be able to retrieve its values ​​as quickly as possible.
Most libraries use delays or read() in loops, which is non-blocking, good, but does not guarantee reading the data as soon as it is ready because these read tests depend on the speed of execution of the loop.

Not many people know this, but Arduino offers a serial read method (Serial.readBytes()), which is implemented differently from the non-blocking read(): readBytes() is blocking and directly uses the UART interrupts backwards to unlock as soon as the data is ready.
This is the most effective method to be notified immediately of the availability of measurements.
It is then enough to set up a reading loop in an asynchronous task on core 0 of the ESP32, to be notified as soon as the data is ready.

## JSY reading speeds

The JSY speed can be changed from its default (4800 bps) to 9600, 19200 and 38400 bps.

- At 4800 bps, the JSY takes on average 171 ms to make a reading
- At 9600 bps, the JSY takes on average 100 ms to make a reading
- At 19200 bps, the JSY takes on average 60 ms to take a reading
- At 38400 bps, the JSY takes on average 41 ms to take a reading

So increasing the speed of your JSY from 4800 to 38400 potentially allows you to react 4x faster to these readings... 
But are these readings meaningful? 
Let's see...

## Internal workings of JSY

The JSY works with an Renergy RN8209G chip, which continuously measures by taking a rolling average and makes the results available on the UART.
For example. if you read the JSY repeatedly:

- At 4800 bps, the JSY reports on average one change every 3 readings, or 513 ms
- At 9600 bps, the JSY reports on average a change every 4-5 readings, or 400-500 ms
- At 19200 bps, the JSY reports on average one change every 9 readings, or 360 ms
- At 38400 bps, the JSY reports on average one change every 9 readings, or 369 ms

So it is not possible to have a router that will make a correction to the routing faster than this minimum delay for the JSY to detect a change in the measurements.
So the routing algorithm should apply for at least 300 ms.

## Load Detection Time

The JSY has a load detection time. 
For example, when turning on a load of 0-100%, it takes a certain amount of time to start making its data available.

- At 4800 bps, the JSY takes approximately 680 ms to detect a load turning on
- At 9600 bps, the JSY takes approximately 400-700 ms to detect a load turning on
- At 19200 bps, the JSY takes approximately 360 ms to detect a load turning on
- At 38400 bps, the JSY takes approximately 330 ms to detect a load turning on

This is the minimum time that the JSY takes to make a measurement available after a load change.

## Ramp-up Time

When a load is on from 0 to 100%, goes from 0 to 3000W for example, the JSY takes time to see the nominal power because it uses moving average.

- At 4800 bps, the JSY has a ramp-up time (time before seeing nominal power) of 1198 ms
- At 9600 bps, JSY has a ramp-up time of 800-1101 ms
- At 19200 bps, JSY has a ramp-up time of 1020 ms
- At 38400 bps, the JSY has a ramp-up time of 1030 ms

This is pretty consistent, and it's the JSY window duration, which is about 1 second.

## Using JSY in remote mode

The library includes a `Sender` application and another `Listener`.
The sender application is a standalone application to flash on an ESP32 connected to a JSY.
It uses ESP-DASH, ElegantOTA, MycilaESPConnect, etc. and allows you to see all the JSY stats, reset the energy, and, above all, sends the measurements via UDP at a **speed of 20 messages per second**, which is as fast as than reading the data locally at 38400 bps on an JSY connected to the ESP32.
The `Listener` application shows how to receive them at a processing speed of 20 messages per second.

![](https://github.com/mathieucarbou/MycilaJSY/assets/61346/3066bf12-31d5-45de-9303-d810f14731d0)

## Conclusion

- Increasing the speed of the JSY does not serve to read more measurements, but above all serves to be notified as quickly as possible when new measurements arrive, without having to wait for the loop to return.
- The JSY appears to have a sliding window of 1 second, and appears to make its measurements available every 300 ms
- Whether the JSY is connected to the ESP or whether the data is retrieved remotely by UDP with the `Sender` application does not change the speed or precision of routing

## MycilaJSY Library

The library is here, with performance tests based on speeds: [https://github.com/mathieucarbou/MycilaJSY](https://github.com/mathieucarbou/MycilaJSY)

It supports:
- Non-blocking mode with asynchronous task
- Callbacks to be notified of metric readings and changes
- Energy reset
- Gear switch
- Remote operation via UDP at a speed of 20 messages per second
