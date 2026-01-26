---
title: CalChart User's Manual
version: 3.8
---

# COUNTERMARCH

Format: `COUNTERMARCH <point 1> <point 2> <steps> <dir 1> <dir 2> <beats>`

COUNTERMARCH is a complicated procedure which implements a countermarch in
the Cal band style.

The movement commences at `<point 1>` by moving `<steps>` in
direction `<dir 1>`, then moving in direction `<dir 2>`, then
moving the opposite of `<dir 1>` until reaching `<point 2>`,
then continuing `<steps>` in the direction opposite of `<dir1>`,
then moving the opposite of `<dir 2>`, then in direction `<dir 1>`
until `<point 1>` is reached again.

This pattern is continued until the specified number of beats have been used.

A standard high-step countermarch of length 16 and `R1` and `R2` as the two
corners would look like this:

```
  COUNTERMARCH R1 R2 1 N E 16
```

In most cases, [`DMCM`](DMCM.md) or [`HSCM`](HSCM.md) can be used
more easily.

A diagram follows:

```
       <dir 1>
       --------->

------------------------------------------>*------->        <dir 2>
^                                      <point 1>   |           |
|     <point 2>                                    V           |
<-------*<------------------------------------------           V
```
