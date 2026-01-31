---
title: CalChart User's Manual
version: 3.8
---

# EITHER

Format: `EITHER(<direction 1> <direction 2> <reference>)`

`EITHER` returns either `<direction 1>` or `<direction 2>`, whichever
is closer to the direction from `P` to `<reference>`.  When the two
directions are equally far from the direction from `P` to `<reference>`,
then `<direction 1>` is returned.

For example,

```
  EITHER(N S R1)
```

executed when `R1` is north of `P` will return `N`; otherwise it will return `S`.

