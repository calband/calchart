---
title: CalChart User's Manual
version: 3.8
---

# DIRFROM

Format: `DIRFROM(<start point> <end point>)`

`DIRFROM` returns the direction needed to get from `<start point>` to
`<end point>`. The value is expressed in degrees and using the math
convention (see [Directions](../DIRECTIONS.md) for more details).

For example,

```
  DIRFROM(R1 NP)
```

will return the direction one would have to go to reach NP from R1.

A related function, `[DIR](DIR.md)`, assumes `P` for the `<start point>`.

