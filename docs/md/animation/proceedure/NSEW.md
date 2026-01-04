---
title: CalChart User's Manual
version: 3.8
---

# NSEW

Format: `NSEW <destination>`

`NSEW` plots a two part course, starting with a north-south trajectory, and
ending in an east-west trajectory.  `NSEW` is equivalent to:

```
  FOUNTAIN N E <destination>
```

because `FOUNTAIN` can reverse directions (see that [`FOUNTAIN`](FOUNTAIN.md) for details).

A similar procedure is [`EWNS`](EWNS.md).

