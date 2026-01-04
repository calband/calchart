---
title: CalChart User's Manual
version: 3.8
---

# DMCM

Format: `DMCM <point 1> <point 2> <beats>`

DMCM executes a diagonal-military countermarch in the Cal Band style.

The movement commences at `<point 1>` by moving one step either NW, NE,
SE or SW, then moving two steps E or W, then moving opposite to the first
direction until reaching `<point 2>`, then continuing one step in the
same direction, then moving the two steps E or W, then in the initial
direction until `<point 1>` is reached again.

This pattern is continued until the specified number of beats have been used.

A diagonal-military countermarch of length 16 and `R1` and `R2` as the two
corners would look like this:

```
  DMCM R1 R2 16
```

This is a specialized case of [`COUNTERMARCH`](COUNTERMARCH.md), as is
[`HSCM`](HSCM.md).

