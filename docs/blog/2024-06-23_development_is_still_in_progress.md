---
layout: default
title: Blog
description: Development is still in progress at a good pace
---

_Date: 2024-06-23_

# Development is still in progress

Development of YaSolR is still in progress at a good pace.
You can follow the progress in the [GitHub project](https://github.com/mathieucarbou/YaSolR-OSS/projects?query=is%3Aopen) view.

- The analysis is completed on real data to determine a good routing algorithm using a PID controller with a tweaked Kp, Ki and Kd in order to minimize the grid import and export as much as possible.
- This PID algorithm will recompute each time a power change is detected, which means in a few milliseconds with a JSY.
- The routing precision is high with a 12-bits resolution
- The remote JSY feature through UDP is being implemented to facilitate testing
