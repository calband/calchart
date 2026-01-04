---
title: CalChart User's Manual
version: 3.8
---

# MTRM

Format: `MTRM <direction>`

`MTRM` is similar to [`MT`](MT.md), in that `P` remains stationary,
facing `<direction>`.  However, the number of beats is assumed to
be the number of beats remaining.  Note that because of this, all the
remaining beats are used up.

`MTRM` is precisely equivalent to:

```
  MT REM <direction>
```
