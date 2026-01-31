---
title: CalChart User's Manual
version: 3.8
---

# EVEN

Format: `EVEN <steps> <destination>`

`EVEN` marches towards the point `<destination>` and determines the
stepsize so that it arrives at the point `<destination>` in exactly
the number of steps specified.  It is precisely the same as:

```
  MARCH DIST(<destination>)/<steps> <steps> DIR(<destination>)
```

