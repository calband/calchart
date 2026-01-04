---
title: CalChart User's Manual
version: 3.8
---

# FUNCTIONS

A function is used in place of a numerical parameter and takes on a
specific value depending on its parameters (if it needs any); this is
called "returning a value."  A typical use of a function is:

```
  MT STEP(2 2 R1) S
```

Note that a function must always be part of another command.

Some functions take parameters, and if there is more than one, they are
separated by a space.  Unlike procedures, the parameter list is surrounded
by parentheses.  In the example above, the STEP function takes three
parameters: 2 (a number), 2 (another number) and R1 (a point).  It returns
a number of steps (see the [step](function/STEP.md) function for more details).

Note that there are two types of parameters: numerical and point.
When one is expected, the other is not allowed.

These are the functions in CalChart:

- [DIR](function/DIR.md)
- [DIRFROM](function/DIRFROM.md)
- [DIST](function/DIST.md)
- [DISTFROM](function/DISTFROM.md)
- [EITHER](function/EITHER.md)
- [OPP](function/OPP.md)
- [STEP](function/STEP.md)
