---
title: CalChart User's Manual
version: 3.8
---

# DIST

Format: `DIST(\verb$<point>$)`

`DIST` returns the distance from `P` (the current position) to `<point>`.
If the direction from `P` to `<point>` is `NW`, `SW`, `SE` or `NE`, then the
distance is given in diagonal-military steps; otherwise it is given in
high-steps.

For example,

```
  DIST(NP)
```

returns the distance from `P` to `NP`.

A related function, `[DISTFROM](DISTFROM.md)`, always returns high-steps.

