Part of the dimmer implementation and LUT code are based on information given by Florent in the following discussions.

-----------------------------------------------------------------------------
https://github.com/fabianoriccardi/dimmable-light/issues/58#issuecomment-2000024933
-----------------------------------------------------------------------------

These are the textbook functions to convert a power ratio to a phase delay ratio.
The duty2phase should only be used to build a lookup table since it's quite expensive.

```python
def phase2duty(phase):
  """
  Phase delay ratio [0,1] to power ratio [0,1]
  Sine square CDF (Cumulative Distribution Function)
  """
  return sin(2 * pi * phase) / (2 * pi) - phase + 1
```

```python
def duty2phase(duty):
  """
  Power delay ratio [0,1] to phase ratio [0,1]
  Invert Sine square CDF, Bisection method
  """
  bounds = [0, 1]
  for _ in range(32):
      phase = (bounds[0] + bounds[1]) / 2
      bounds[duty > phase2duty(phase)] = phase
  return phase
```

This d2p function is much more accurate.
Of course the accuracy is as good as the grid remains a perfect sine wave.

phase = (powf(1 - duty, 0.31338) - powf(duty, 0.31338) + 1) / 2

-----------------------------------------------------------------------------
https://github.com/mathieucarbou/YaSolR/discussions/21#discussion-6435999
-----------------------------------------------------------------------------

One thing I don't like is that the duty level is only 8 bits.
It's a 7.8 watts step/error for a 2000 watts heating element.
The step would be negligible with a 12bits resolution or over.

The code I use to convert the duty to a phase delay: (see LUT Model)

```c++
// AC phase delay table (sine square distribution).
// power_ratio = sin(2 * pi * delay_ratio) / (2 * pi) - delay_ratio + 1
// index 0  ≈   0% power ≈   5% phase angle ≈ 95% phase delay
// index 39 ≈  50% power ≈  50% phase angle ≈ 50% phase delay
// index 79 ≈ 100% power ≈  95% phase angle ≈  5% phase delay

#define TRIAC_RESOLUTION   ( 12 )  // bits
#define TRIAC_MAX          ( (1 << TRIAC_RESOLUTION) - 1 )

#define TABLE_PHASE_LEN    ( 80 )
#define TABLE_PHASE_SCALE  ( (TABLE_PHASE_LEN - 1) * (1UL << (16 - TRIAC_RESOLUTION)) )

static const uint16_t TABLE_PHASE_DELAY[TABLE_PHASE_LEN] {
    0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094,
    0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2,
    0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9,
    0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8,
    0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf,
    0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2,
    0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606,
    0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686,
    0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296,
    0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016};

static uint32_t lookup_phase_delay(uint32_t duty12, uint16_t period)
{
    uint32_t slot  = duty12 * TABLE_PHASE_SCALE + (TABLE_PHASE_SCALE >> 1);
    uint16_t index = slot >> 16;
    uint32_t a     = TABLE_PHASE_DELAY[index]
    uint32_t b     = TABLE_PHASE_DELAY[index + 1];
    uint32_t delay = a - (((a - b) * (slot & 0xffff)) >> 16);  // interpolate a b

    return (delay * period) >> 16;  // scale to period
}
```

The 0% and 100% needs to be handled separately.
The lookup is interpolated without FPU.
The error is under 0.001 compared to the ideal model.

-----------------------------------------------------------------------------
https://github.com/mathieucarbou/YaSolR/issues/18#issuecomment-2023988139
-----------------------------------------------------------------------------

The JSY should be plugged on main voltage, not after the dimmer.
If you need the RMS voltage of a phase cut resistive load, then simply use:
`Vrms_dimmer = ActivePower / Irms`
or
`Vrms_dimmer = Vrms_grid * PowerFactor`
or
`Vrms_dimmer = Vrms_grid * sqrt(power_duty)`

My system is first determining the dimmer max power once by turning it 0%/100% a few times and by reading the active power from the grid. It is used to compute and store the heating element resistance (R = ((Vrms grid) ^ 2) / ActivePower).
During the routing, I measure the grid's power, compute the power error + PID, add/remove it from the duty and then lookup an interpolated phase delay.

My lookup table has 80 unit16 generated from the duty2phase function I posted earlier.
The error is less then 0.001 with a perfect sine wave.
The heating element max power is  Router_PowerMax = (Grid_Vrms ^ 2) / R
The reported routed power is  Router_Power = duty * Router_PowerMax
The reported routed voltage is  Router_Vrms = Grid_Vrms * sqrt(duty)
The reported routed current is  Router_Irms = Router_Power / Router_Vrms
The reported routed power factor is Router_PowerFactor = sqrt(duty)

I use the MCPWM module of the ESP32 to drive a MOC3023 and BTA24-600CWRG . It's much more reliable and accurate since it doesn't require interrupt. It however requires a ZC detector with a good squared signal, not like those shitty RobotDyn.

I use the same kind of ZCD circuit you are mentioning but with modifications. I replaced the 1nF capacitor in front of the bridge rectifier by a 10nF to reduce the delay of the zero crossing. I also added a 10nF in parallel of the diode and reduced the 1k resistor to 330ohms to improve the triggering edge.

-----------------------------------------------------------------------------
https://github.com/mathieucarbou/YaSolR/issues/18#issuecomment-2177785644
-----------------------------------------------------------------------------

I simply use the PID coefficients without the timing and integral.

When the grid meter reports -800 Watts (injection), I consider the error to be 800 watts.
If the previous error was 0, then I will add to the triac:

```
correction = error * kp + (error - error_previous) * kd
correction = 800 * 0.4 + (800 - 0) * 0.2
correction = 480 watts
```

If the previous error was 800 watts:

```
correction = error * kp + (error - error_previous) * kd
correction = 800 * 0.4 + (800 - 800) * 0.2
correction = 320 watts
```

The correction is then added to the duty:

```
duty12bits += correction * TRIAC_MAX / (V_grid * V_grid / R_load)
```
