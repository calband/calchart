---
title: CalChart User's Manual
version: 3.8
---

# FM

Format: `FM <steps> <direction>`

`FM` is similar to [`MARCH`](MARCH.md) in that it moves `P` in the
specified direction for the specified number of steps.  However, the
stepsize is inferred from direction (see [Step Sizes](../STEPSIZES.md)
for details).

An error will occur if the number of beats remaining is less than
`<steps>`.

