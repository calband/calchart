---
title: CalChart User's Manual
version: 3.8
---

# HSCM

Format: `HSCM <point 1> <point 2> <beats>`

HSCM executes a high-step countermarch in the Cal Band style.

The movement commences at `<point 1>` by moving one step either N or S,
then moving two steps E or W, then moving opposite to the first direction
until reaching `<point 2>`, then continuing one step in the same
direction, then moving the two steps E or W, then in the initial direction
until `<point 1>` is reached again.

This pattern is continued until the specified number of beats have been used.

The upper-right and lower-left corners of the countermarch block should be
specified with reference points.

A high-step countermarch of length 16 and `R1` and `R2` as the two corners would
look like this:

```
  HSCM R1 R2 16
```

This is a specialized case of [`COUNTERMARCH`](COUNTERMARCH.md), as is
[`DMCM`](DMCM.md).

