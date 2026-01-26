---
title: CalChart User's Manual
version: 3.8
---

# BLAM

Format: `BLAM`

`BLAM` uses all remaining beats, and any stepsize necessary to move the current
point to `NP`.  Note that all the beats are used, and the movement
may occur off the grid.  `BLAM` is equivalent to:

```
  MARCH DIST(NP)/REM REM DIR(NP)
```

or 

```
  EVEN REM NP
```

except when `REM` is 0, in which case it is equivalent to
`MAGIC <destination>` (see [`MAGIC`](MAGIC.md)).

