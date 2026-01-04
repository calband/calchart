---
title: CalChart User's Manual
version: 3.8
---

# STEPSIZES

Stepsizes are specified in units of one high-step.  The following constants
are supplied step-sizes:

| **NAME** | **STEPSIZE** | **VALUE** |
|----------|--------------|-----------|
| `HS` | 1 high-step | 1 |
| `MM` | 1 mini-military | 1 |
| `GV` | 1 grapevine-step | 1 |
| `SH` | 1 show-high | .5 |
| `JS` | 1 jerky-step | .5 |
| `M` | 1 military-step | 1.333333 (8/6) |
| `DM` | 1 diagonal-military | 1.414214 (the square root of 2) |

In addition, any numerical expression may be used (for example, 1.5, or
`DIST`/8, etc.)

Some procedures determine a stepsize based on direction.  The method is:
```
  If direction is NW, SW, SE or NE, stepsize is DM
  Otherwise,                        stepsize is HS
```

Procedures and functions that utilize this method are:
`[DIST](function/DIST.md)`, `[STEP](function/STEP.md)`,
`[COUNTERMARCH](proceedure/COUNTERMARCH.md)`, `[EWNS](proceedure/EWNS.md)`,
`[FM](proceedure/FM.md)`, `[FMTO](proceedure/FMTO.md)`, `[FOUNTAIN](proceedure/FOUNTAIN.md)`, `[NSEW](proceedure/NSEW.md)`

