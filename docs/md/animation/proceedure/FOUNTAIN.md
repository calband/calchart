---
title: CalChart User's Manual
version: 3.8
---

# FOUNTAIN

Format: `FOUNTAIN <direction 1> <direction 2> <destination>`

or

Format: `FOUNTAIN <direction 1> <direction 2> <stepsize 1> <stepsize 2> <destination>`

FOUNTAIN marches a two-part course, starting in `<direction 1>` and
ending in `<direction 2>`, and arriving at `<destination>`.
If necessary, the opposite of `<direction 1>` will be used instead
of `<direction 1>`, and the opposite of `<direction 2>` will
be used instead of `<direction 2>`.

If the stepsizes are not given, a value of 1 will be used.

Note that if either trajectory is unnecessary, it will be skipped.

In most cases, [`NSEW`](NSEW.md), [`EWNS`](EWNS.md),
[`DMHS`](DMHS.md), or [`HSDM`](HSDM.md) can be used more easily.

