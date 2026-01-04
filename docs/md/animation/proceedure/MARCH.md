---
title: CalChart User's Manual
version: 3.8
---

# MARCH

Format: `MARCH <stepsize> <steps> <direction>`

or

Format: `MARCH <stepsize> <steps> <direction> <face direction>`

`MARCH` will move the current point (`P`) in the specified direction for the
specified number of beats.  Each step will be of size
`<stepsize>`.  The point will face the direction of movement unless
a direction is given.

An error will occur if there are not enough counts remaining.

Note that if `<steps>` is negative, the person will march backwards!

