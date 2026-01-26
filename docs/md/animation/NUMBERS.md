---
title: CalChart User's Manual
version: 3.8
---

# NUMBERS

When a function or procedure calls for a number (for example a stepsize,
direction, or angle), the best method of expressing this value is with
a constant, such as `HS` (for a stepsize) or `NW` (for a direction).
See the sections on [Directions](DIRECTIONS.md), BEATS, and [step sizes](STEPSIZES.md) for lists
of constants.

Alternatively, a function can be used (see that section for more
information), or a simple number can be used, such as 12.5 or -180.

Also, a variable can be used, such as A or X.  See the section on
[variables](VARIABLES.md) for details.

Finally, these different types of numerical expressions can be combined
using standard arithmetic symbols:

| Operation      | Expression            |
|----------------|-----------------------|
| addition       | `<num>` + `<num>`     |
| subtraction    | `<num>` - `<num>`     |
| multiplication | `<num>` * `<num>`     |
| division       | `<num>` / `<num>`     |
| negation       | - `<num>`             |

These symbols are interpreted in stardard arithmetic order, unless they are
grouped with parentheses.

Numerical expressions cannot be used when a point is expected.

