---
layout: default
title: Blog
description: "The Importance of a good ZCD circuit"
---

_Date: 2024-07-024_

# The Importance of a good ZCD circuit

During YaSolR development, I've tested several ZCD modules with an oscilloscope, but also programmatically with a library allowing me to analyse the quality of the detection of ZCD pulses.

My conclusions, which are also common to a lot of other experts in Solar Routers, is that **the quality of the ZCD module is very important for the accuracy of routing.**

## What is a ZCD circuit?

Please read the [YaSolR Overview](../overview#zero-cross-detection-zcd) to understand the concept of Zero-Cross Detection.
A Wikipedia article is also available [here](https://en.wikipedia.org/wiki/Zero_crossing).

Using a good ZCD circuit producing a reliable pulse is very important.

If the pulses are not reliable, some short flickering could be caused by a mis-detection of the zero point or by the existence of spurious pulses (false-positives), and consequently cause the TRIAC to fire at the wrong time, or the calculations for the burst fire sequence to be wrong.
These are visible if you plug an incandescent light bulb to the dimmer output: the bulb will flicker from time to time.
The effect on a water tank resistance is even bigger: it will create some spurious spikes of power consumption, that the router will try to compensate just after by considerably reducing the dimming level.
This creates some waves instead of keeping the import and export at a near-0 level.

These phenomena are not visible with a good ZCD module coupled with a Random SSR.

The Robodyn is such a device that has an unreliable ZC pulse: all experts working on Solar Routers who have measured that correctly, tend to agree with the fact that **the Robodyn is one of the worst device to use because of its unreliable ZC and poor quality circuit and heat sink**.

Here is below a YaSolR screenshot of the Grid Power graph showing the effect of a bad ZCD module on the power consumption and import.
On the lef side, the Robodyn ZCD is used, then I've switched (live) to a dedicated ZCD module.

[![](assets/img/screenshots/robodyn-vs-zc-module-grid-power.jpeg)](assets/img/screenshots/robodyn-vs-zc-module-grid-power.jpeg)

Here is another example below of th YaSolR PID Tuning view showing the input value of the PID controller.
The dedicated ZCD module was used, then I've switched (live) to a Robodyn ZCD module.
The update rate is high: 3 times per second.
All the JSY measurements are captured and displayed.
You can clearly see the flickering caused by the bad quality of the Robodyn ZCD pulses, which gets compensated just after by the PID controller.

[![](assets/img/screenshots/robodyn-vs-zc-module-pid-tuning.jpeg)](assets/img/screenshots/robodyn-vs-zc-module-pid-tuning.jpeg)

Lastly, here is a graph showing in Home Assistant the effect of the Robodyn ZCD on the dimmer output.
The Robodyn ZCD was used form 11:58 to 12:02, then I've switched (live) to a dedicated ZCD module.

[![](assets/img/screenshots/robodyn-vs-zc-module-ha.jpeg)](assets/img/screenshots/robodyn-vs-zc-module-ha.jpeg)

You can read more about these issues here also:

- [About dimmer boards](https://github.com/fabianoriccardi/dimmable-light/wiki/About-dimmer-boards)
- [Notes about specific architectures](https://github.com/fabianoriccardi/dimmable-light/wiki/Notes-about-specific-architectures#esp32)
