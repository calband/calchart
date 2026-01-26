---
title: CalChart User's Manual
version: 3.8
---

# STEP

Format: `STEP(<beats> <blocksize> <start point>)`

`STEP` is used in conjunction with MT to implement step-drills,
where one person starts, and then some steps (say x) later the next leaves,
and x steps after that the next leaves, etc.  `STEP` returns the number of
counts for the given person to marktime in relation to the given start
point; however, it can be used for other things besides step-drills.

The `<beats>` parameter specifies how many beats are to elapse between
people leaving, and the `<counts>` parameter specifies how far apart the
people are from each other.

The `<start point>` is a point which is considered the beginning of
the drill (e.g. the first to leave).

For example,

```
  MT STEP(2 4 R1) W
```

will make each person in a 4-step block leave 2 counts after his neighbor;
the first person to leave is at the same location as `R1`.  While they are
waiting to go, the people will marktime west.

