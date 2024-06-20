/******************************************************************************
 *  This file is part of Dimmable Light for Arduino, a library to control     *
 *  dimmers.                                                                  *
 *                                                                            *
 *  Copyright (C) 2018-2023  Fabiano Riccardi                                 *
 *                                                                            *
 *  Dimmable Light for Arduino is free software; you can redistribute         *
 *  it and/or modify it under the terms of the GNU Lesser General Public      *
 *  License as published by the Free Software Foundation; either              *
 *  version 2.1 of the License, or (at your option) any later version.        *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
 *  Lesser General Public License for more details.                           *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/
#include "thyristor.h"
#include <Arduino.h>
#include <MycilaCircularBuffer.h>

#include "hw_timer_esp32.h"

// In microseconds
static uint16_t semiPeriodLength = 0;

// maximum supported pulse width in microseconds
#define MAX_PULSE_WIDTH 4000
#define MIN_PULSE_WIDTH 100

// These margins are precautions against noise, electrical spikes and frequency skew errors.
// Activation delays lower than *startMargin* turn the thyristor fully ON.
// Activation delays higher than *endMargin* turn the thyristor fully OFF.
// Tune this parameters accordingly to your setup (electrical network, MCU, and ZC circuitry).
// Values are expressed in microseconds.
static const uint16_t startMargin = 100;
static const uint16_t endMargin = 100;

// Merge Period represents the time span in which 2 (or more) very near delays are merged (the
// higher ones are merged in the smaller one). This could be necessary for 2 main reasons:
// 1) Efficiency, in fact in some applications you will never seem differences between
//    near delays, hence raising many interrupts is useless.
// 2) MCU inability to satisfy very tight "timer start".
// After some experiments on incandescence light bulbs, I noted that even 50 microseconds
// are not negligible, so I decided to set threshold lower than 20microsecond. Before lowering this
// value, check the documentation of the specific MCU since some have limitations. For example,
// ESP8266 API documentation suggests to set timer dealy higher than >10us. If you use 8-bit timers
// on AVR, you should set a bigger Merge Period (e.g. 100us). Moreover, you should also consider the
// number of instantiated dimmers: ISRs will take more time as the dimmer count increases, so you
// may need to increase Merge Period. The default value is intended to handle up to 8 dimmers.
static const uint16_t mergePeriod = 20;

// Period in microseconds before the end of the semiperiod when an interrupt is triggered to
// turn off all gate signals.
static const uint16_t gateTurnOffTime = 70;

static_assert(endMargin - gateTurnOffTime > mergePeriod, "endMargin must be greater than "
                                                         "(gateTurnOffTime + mergePeriod)");

struct PinDelay {
    uint8_t pin;
    uint16_t delay;
};

enum class INT_TYPE { ZC,
                      ACTIVATE_THYRISTORS,
                      TURN_OFF_GATES };

static INT_TYPE nextISR = INT_TYPE::ACTIVATE_THYRISTORS;

/**
 * Temporary struct manipulated by the ISR storing the timing information about each dimmer.
 */
static struct PinDelay pinDelay[Thyristor::N];

/**
 * Summary of thyristors' state used by ISR (concurrent-safe).
 */
static bool _allThyristorsOnOff = true;

/**
 * Tell if zero-cross interrupt is enabled.
 */
static bool interruptEnabled = false;

/**
 * Number of thyristors already managed in the current semi-period.
 */
static uint8_t thyristorManaged = 0;

/**
 * Number of thyristors FULLY on. The remaining ones must be turned
 * off by turn_off_gates_int at the end of the semi-period.
 */
static uint8_t alwaysOnCounter = 0;
static uint8_t alwaysOffCounter = 0;

void ARDUINO_ISR_ATTR turn_off_gates_int() {
  for (int i = alwaysOnCounter; i < Thyristor::nThyristors; i++) {
    if (pinDelay[i].pin != 0xff) {
      digitalWrite(pinDelay[i].pin, LOW);
    }
  }
}

/**
 * Timer routine to turn on one or more thyristors. This function may be be called multiple times
 * per semi-period depending on the current thyristors configuration.
 */
void ARDUINO_ISR_ATTR activate_thyristors() {
  const uint8_t firstToBeUpdated = thyristorManaged;

  for (;
       // The last thyristor is managed outside the loop
       thyristorManaged < Thyristor::nThyristors - 1 &&
       // Consider the "near" thyristors
       pinDelay[thyristorManaged + 1].delay - pinDelay[firstToBeUpdated].delay < mergePeriod &&
       // Exclude the one who must remain totally off
       pinDelay[thyristorManaged].delay <= semiPeriodLength - endMargin;
       thyristorManaged++) {
    if (pinDelay[thyristorManaged].pin != 0xff) {
      digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
    }
  }
  if (pinDelay[thyristorManaged].pin != 0xff) {
    digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
  }
  thyristorManaged++;

  // This while is dedicated to all those thyristors with delay == semiPeriodLength-margin; those
  // are the ones who shouldn't turn on, hence they can be skipped
  while (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay == semiPeriodLength) {
    thyristorManaged++;
  }

  if (thyristorManaged < Thyristor::nThyristors) {
    int delayAbsolute = pinDelay[thyristorManaged].delay;
    setAlarm(delayAbsolute);
  } else {
    // If there are not more thyristors to serve, set timer to turn off gates' signal
    uint16_t delayAbsolute = semiPeriodLength - gateTurnOffTime;
    nextISR = INT_TYPE::TURN_OFF_GATES;
    setAlarm(delayAbsolute);
  }
}

void ARDUINO_ISR_ATTR zero_cross_int() {
  // Turn OFF all the thyristors, even if always ON.
  // This is to speed up transitions between ON to OFF state:
  // If I don't turn OFF all those thyristors, I must wait
  // a semiperiod to turn off those one.
  for (int i = 0; i < Thyristor::nThyristors; i++) {
    if (pinDelay[i].pin != 0xff) {
      digitalWrite(pinDelay[i].pin, LOW);
    }
  }

  // Update the structures and set thresholds, if needed
  if (Thyristor::newDelayValues && !Thyristor::updatingStruct) {
    Thyristor::newDelayValues = false;
    alwaysOffCounter = 0;
    alwaysOnCounter = 0;
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      pinDelay[i].pin = Thyristor::thyristors[i]->pin;
      // Rounding delays to avoid error and unexpected behavior due to
      // non-ideal thyristors and not perfect sine wave
      if (Thyristor::thyristors[i]->delay == 0) {
        alwaysOnCounter++;
        pinDelay[i].delay = 0;
      } else if (Thyristor::thyristors[i]->delay < startMargin) {
        alwaysOnCounter++;
        pinDelay[i].delay = 0;
      } else if (Thyristor::thyristors[i]->delay == semiPeriodLength) {
        alwaysOffCounter++;
        pinDelay[i].delay = semiPeriodLength;
      } else if (Thyristor::thyristors[i]->delay > semiPeriodLength - endMargin) {
        alwaysOffCounter++;
        pinDelay[i].delay = semiPeriodLength;
      } else {
        pinDelay[i].delay = Thyristor::thyristors[i]->delay;
      }
    }
    _allThyristorsOnOff = Thyristor::allThyristorsOnOff;
  }

  thyristorManaged = 0;

  // if all are on and off, I can disable the zero cross interrupt
  if (_allThyristorsOnOff) {
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      if (pinDelay[i].pin != 0xff) {
        if (pinDelay[i].delay == semiPeriodLength) {
          digitalWrite(pinDelay[i].pin, LOW);
        } else {
          digitalWrite(pinDelay[i].pin, HIGH);
        }
      }
      thyristorManaged++;
    }

    return;
  }

  // Turn on thyristors with 0 delay (always on)
  while (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay == 0) {
    if (pinDelay[thyristorManaged].pin != 0xff) {
      digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
    }
    thyristorManaged++;
  }

  // This block of code is inteded to manage the case near to the next semi-period:
  // In this case we should avoid to trigger the timer, because the effective semiperiod
  // perceived by the esp8266 could be less than 10000microsecond. This could be due to
  // the relative time (there is no possibily to set the timer to an absolute time)
  // Moreover, it is impossible to disable an interrupt once it is armed, neither
  // change the callback function.
  // NOTE: don't know why, but the timer seem trigger even when it is not set...
  // so a provvisory solution if to set the relative callback to NULL!
  // NOTE 2: this improvement should be think even for multiple lamp!
  if (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay < semiPeriodLength) {
    uint16_t delayAbsolute = pinDelay[thyristorManaged].delay;
    // setCallback(activate_thyristors);
    nextISR = INT_TYPE::ACTIVATE_THYRISTORS;
    startTimerAndTrigger(delayAbsolute);

  } else {
    // This while is dedicated to all those thyristor wih delay == semiPeriodLength-margin; those
    // are the ones who shouldn't turn on, hence they can be skipped
    while (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay == semiPeriodLength) {
      thyristorManaged++;
    }
    stopTimer();
  }
}

// In microsecond
int Thyristor::semiPeriodShrinkMargin = 50;
int Thyristor::semiPeriodExpandMargin = 50;

static uint32_t lastTime = 0;
static Mycila::CircularBuffer<uint32_t, 5> queue(0, UINT32_MAX);
static Mycila::CircularBuffer<uint32_t, 5> pulses(0, UINT32_MAX);

void ARDUINO_ISR_ATTR zero_cross_pulse_int() {
  if (!lastTime) {
    lastTime = micros();

  } else {
    uint32_t now = micros();

    // "diff" is correct even when timer rolls back, because these values are unsigned
    uint32_t diff = now - lastTime;

    if (diff < MAX_PULSE_WIDTH && diff > MIN_PULSE_WIDTH) {
      pulses.add(diff);
      // - we detected a pulse, so we must be at its falling edge
      // - do not update the lastTime variable
      return;
    }

    // Filters out spurious interrupts. The effectiveness of this simple
    // filter could vary depending on noise on electrical network.
    if (diff < semiPeriodLength - Thyristor::semiPeriodShrinkMargin) {
      return;
    }

    // if diff is very very greater than the theoretical value, the electrical signal
    // can be considered as lost for a while and I must reset my moving average.
    // I decided to use "16" because is a power of 2, very fast to be computed.
    if (semiPeriodLength && diff > semiPeriodLength * 16) {
      queue.reset(0, UINT32_MAX);
      pulses.reset(0, UINT32_MAX);
    } else {
      // If filtering has passed, I can update the moving average
      queue.add(diff);
    }

    lastTime = now;
  }

  uint32_t pulseWidth = pulses.last();

  if (!pulseWidth)
    return;

  pulseWidth /= 2;

  if (pulseWidth > endMargin) {
    nextISR = INT_TYPE::ZC;
    startTimerAndTrigger(pulseWidth - endMargin);
  } else {
    zero_cross_int();
  }
}

void ARDUINO_ISR_ATTR isr_selector() {
  if (nextISR == INT_TYPE::ACTIVATE_THYRISTORS) {
    activate_thyristors();
  } else if (nextISR == INT_TYPE::TURN_OFF_GATES) {
    turn_off_gates_int();
  } else if (nextISR == INT_TYPE::ZC) {
    zero_cross_int();
  }
}

void Thyristor::setDelay(uint16_t newDelay) {
  if (verbosity > 2) {
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      Serial.print(String("setB: ") + "posIntoArray:" + thyristors[i]->posIntoArray + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }

  if (newDelay > semiPeriodLength) {
    newDelay = semiPeriodLength;
  }

  // Reorder the array to speed up the interrupt.
  // This mini-algorithm works on a different memory area w.r.t. the ISR,
  // so it is concurrent-safe

  updatingStruct = true;
  // Array example, it is always ordered, higher values means lower brightness levels
  // [45,678,5000,7500,9000]
  if (newDelay > delay) {
    if (verbosity > 2)
      Serial.println("\tlowering the light..");
    bool done = false;
    /////////////////////////////////////////////////////////////////
    // Let's find the new position
    int i = posIntoArray + 1;
    while (i < nThyristors && !done) {
      if (newDelay <= thyristors[i]->delay) {
        done = true;
      } else {
        i++;
      }
    }
    // This could be due to 2 reasons:
    // 1) the light is already the lowest delay (i.e. turned off)
    // 2) the delay is not changed to overpass the neightbour
    if (posIntoArray + 1 == i) {
      if (verbosity > 2)
        Serial.println("No need to shift..");
    } else {
      int target;
      // Means that we have reached the end, the target i the last element
      if (i == nThyristors) {
        target = nThyristors - 1;
      } else {
        target = i - 1;
      }

      // Let's shift
      for (int i = posIntoArray; i < target; i++) {
        thyristors[i] = thyristors[i + 1];
        thyristors[i]->posIntoArray = i;
      }
      thyristors[target] = this;
      this->posIntoArray = target;
    }
  } else if (newDelay < delay) {
    if (verbosity > 2)
      Serial.println("\traising the light..");
    bool done = false;
    int i = posIntoArray - 1;
    while (i >= 0 && !done) {
      if (newDelay >= thyristors[i]->delay) {
        done = true;
      } else {
        i--;
      }
    }
    if (posIntoArray - 1 == i) {
      if (verbosity > 2)
        Serial.println("No need to shift..");
    } else {
      int target;
      // Means that we have reached the start, the target is the first element
      if (!done) {
        target = 0;
      } else {
        target = i + 1;
      }

      // Let's shift
      for (int i = posIntoArray; i > target; i--) {
        thyristors[i] = thyristors[i - 1];
        thyristors[i]->posIntoArray = i;
      }
      thyristors[target] = this;
      this->posIntoArray = target;
    }
  } else {
    if (verbosity > 2)
      Serial.println("Warning: you are setting the same delay as the previous one!");
    updatingStruct = false;
    return;
  }

  delay = newDelay;

  // Temp values those are "commited" at the end of this method
  bool newAllThyristorsOnOff = allThyristorsOnOff;
  if (newDelay == semiPeriodLength || newDelay == 0) {
    newAllThyristorsOnOff = areThyristorsOnOff();
  } else {
    // if newDelay is not optimizable i.e. a value between (0; semiPeriodLength)
    newAllThyristorsOnOff = false;
  }
  allThyristorsOnOff = newAllThyristorsOnOff;
  if (verbosity > 1)
    Serial.println(String("allThyristorsOnOff: ") + allThyristorsOnOff);

  newDelayValues = true;
  updatingStruct = false;

  if (verbosity > 2) {
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      Serial.print(String("\tsetB: ") + "posIntoArray:" + thyristors[i]->posIntoArray + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }
}

void Thyristor::begin() {
  pinMode(syncPin, syncPullup ? INPUT_PULLUP : INPUT);
  timerInit(isr_selector);
  // Starts immediately to sense the electricity grid
  interruptEnabled = true;
  attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_pulse_int, CHANGE);
}

void Thyristor::end() {
  detachInterrupt(digitalPinToInterrupt(syncPin));
  pinMode(syncPin, INPUT);
  queue.reset(0, UINT32_MAX);
  pulses.reset(0, UINT32_MAX);
  setFrequency(0);
}

float Thyristor::getFrequency() {
  if (semiPeriodLength == 0) {
    return 0;
  }
  return 1000000 / 2 / (float)(semiPeriodLength);
}

uint16_t Thyristor::getSemiPeriod() {
  return semiPeriodLength;
}

void Thyristor::setFrequency(float frequency) {
  if (frequency < 0) {
    return;
  }

  if (frequency == 0) {
    semiPeriodLength = 0;
    return;
  }

  semiPeriodLength = 1000000 / 2 / frequency;
}

uint16_t Thyristor::getPulseWidth() {
  noInterrupts();
  uint32_t diff = micros() - lastTime;
  if (diff > semiPeriodLength) {
    pulses.reset(0, UINT32_MAX);
  }
  uint16_t avg = pulses.avg();
  interrupts();
  return avg;
}

uint16_t Thyristor::getMaxPulseWidth() {
  noInterrupts();
  uint32_t diff = micros() - lastTime;
  if (diff > semiPeriodLength) {
    pulses.reset(0, UINT32_MAX);
  }
  uint16_t max = pulses.max();
  interrupts();
  return max;
}

uint16_t Thyristor::getMinPulseWidth() {
  noInterrupts();
  uint32_t diff = micros() - lastTime;
  if (diff > semiPeriodLength) {
    pulses.reset(0, UINT32_MAX);
  }
  uint16_t min = pulses.min();
  interrupts();
  return min;
}

uint16_t Thyristor::getLastPulseWidth() {
  noInterrupts();
  uint32_t diff = micros() - lastTime;
  if (diff > semiPeriodLength) {
    pulses.reset(0, UINT32_MAX);
  }
  uint16_t last = pulses.last();
  interrupts();
  return last;
}

float Thyristor::getDetectedFrequency() {
  float c;
  float tot;
  {
    // Stop interrupt to freeze variables modified or accessed in the interrupt
    noInterrupts();

    // "diff" is correct even when rolling back, because all of them are unsigned
    uint32_t diff = micros() - lastTime;

    // if diff is very very greater than the theoretical value, the electrical signal
    // can be considered as lost for a while.
    // I decided to use "16" because is a power of 2, very fast to be computed.
    if (semiPeriodLength && diff > semiPeriodLength * 16) {
      queue.reset(0, UINT32_MAX);
    }

    c = queue.count();
    tot = queue.sum();
    interrupts();
  }

  // We need at least a sample to return a value different from 0
  // *1000000: us
  // /2: from semiperiod to full period
  return tot > 0 ? (c * 1000000 / 2 / tot) : 0;
}

Thyristor::Thyristor(int pin) : pin(pin), delay(semiPeriodLength) {
  if (nThyristors < N) {
    pinMode(pin, OUTPUT);

    updatingStruct = true;

    posIntoArray = nThyristors;
    nThyristors++;
    thyristors[posIntoArray] = this;

    // Full reorder of the array
    for (int i = 0; i < nThyristors; i++) {
      for (int j = i + 1; j < nThyristors - 1; j++) {
        if (thyristors[i]->delay > thyristors[j]->delay) {
          Thyristor* temp = thyristors[i];
          thyristors[i] = thyristors[j];
          thyristors[j] = temp;
        }
      }
    }
    // Set the posIntoArray with a "brutal" assignement to each Thyristor
    for (int i = 0; i < nThyristors; i++) {
      thyristors[i]->posIntoArray = i;
    }

    newDelayValues = true;
    updatingStruct = false;
  } else {
    // TODO return error or exception
  }
}

Thyristor::~Thyristor() {
  // Recompact the array
  updatingStruct = true;

  if (nThyristors == 1) {
    thyristors[0] = nullptr;
    pinDelay[0].pin = 0xff;
  } else if (nThyristors == 2) {
    if (thyristors[0] == this) {
      thyristors[0] = thyristors[1];
    }
    if (pinDelay[0].pin == pin) {
      pinDelay[0] = pinDelay[1];
    }
  }
  thyristors[1] = nullptr;
  pinDelay[1].pin = 0xff;

  nThyristors--;

  updatingStruct = false;
}

bool Thyristor::areThyristorsOnOff() {
  bool allOnOff = true;
  int i = 0;
  while (i < nThyristors && allOnOff) {
    if (thyristors[i]->getDelay() != 0 && thyristors[i]->getDelay() != semiPeriodLength) {
      allOnOff = false;
    } else {
      i++;
    }
  }
  return allOnOff;
}

uint8_t Thyristor::nThyristors = 0;
Thyristor* Thyristor::thyristors[Thyristor::N] = {nullptr};
bool Thyristor::newDelayValues = false;
bool Thyristor::updatingStruct = false;
bool Thyristor::allThyristorsOnOff = true;
uint8_t Thyristor::syncPin = 255;
bool Thyristor::syncPullup = false;
