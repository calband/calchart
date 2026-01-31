---
title: CalChart User's Manual
version: 3.8
---

# GRID

Format: `GRID <grid scale>`

`GRID` is a special command that moves `P` (see [`MAGIC`](MAGIC.md))
without taking any beats.  It moves `P` onto the specified scale of grid,
where <grid scale> is interpreted in high-steps.

For example,

```
  GRID 2
```

will move `P` so that it is on a two-step high-step grid.

This command is useful for correcting small errors in special continuities.

