---
title: CalChart User's Manual
version: 3.8
---

# ROTATE

Format: `ROTATE <angle> <steps> <pivot point>`

`ROTATE` will rotate `P` around the `<pivot point>` for the given
number of steps.  The total angle traversed will be `<angle>` (given in
degrees).

Note that angle is interpreted in the standard mathematical way:

 * Positive angles rotate counter-clockwise
 * Negative angles rotate clockwise

Another note:  If `<pivot point>` coincides with `P`, then the person will
rotate around himself (i.e. stand in one place and turn slowly).  The
person will begin the rotation facing `DOH`.
