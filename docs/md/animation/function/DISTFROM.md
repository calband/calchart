---
title: CalChart User's Manual
version: 3.8
---

# DISTFROM

Format: `DISTFROM(<point 1> <point 2>)`

`DISTFROM` returns the distance, in units of one high-step, from `<point 1>`
to `<point 2>`.

For example,

```
  DISTFROM(R1 NP)
```

will return the distance between `R1` and `NP`.

A related function, `[DIST](DIST.md)`, assumes `P` for `<point 1>` and
uses diagonal-military steps if a diagonal direction is used.

