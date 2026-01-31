---
title: CalChart User's Manual
version: 3.8
---

# POINTS

To specify a point, one of these symbols MUST be used:

| **NAME** | **POINT** |
|----------|-----------|
| `SP` | location on the current stuntsheet |
| `P` | current location |
| `NP` | location on the next stuntsheet |
| `R1` | first reference point on the current stuntsheet |
| `R2` | second reference point |
| `R3` | third reference point |

First, note that `P` changes with every beat, but the others are constant
throughout a single stuntsheet.

Second, points differ from numerical expressions in that they cannot be
combined by arithmetic operators, and cannot be used when a numeric
parameter is expected.

