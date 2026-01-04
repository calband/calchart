---
title: CalChart User's Manual
version: 3.8
---

# DIR

Format: `DIR(<point>)`

`DIR` returns the direction that `P` would have to travel to reach `<point>`.
The value is expressed in degrees.  Note also that this direction follows
the math convention (see the section on [directions](../DIRECTIONS.md)).

For example,

```
  DIR(NP)
```

will return the direction to travel from `P` to `NP`.

`DIR` is precisely equivalent to:

```
  DIRFROM(P <point>)
```

