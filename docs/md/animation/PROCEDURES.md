---
title: CalChart User's Manual
version: 3.8
---

# PROCEDURES

Procedures are used to give marching instructions, like moving from one
position to another, rotating, or marking time.

A typical procedure call is:

```
  EVEN REM-12 NP
```

The first word is the name of a procedure.  Following are the parameters
that tell the procedure what to do (if it needs any).  Parameters are of
two types: POINTS and NUMBERS (see those sections for more information).
When one is expected, the other is not allowed.  Parameters should be
separated from each other by a space.  Unlike functions, the parameter list
is not surrounded by parentheses.

The above example has

```
  REM-12
```

as the first (numerical) parameter, and

```
  NP
```

as the second (point) parameter.

These are the procedures that may be used in CalChart:

- [BLAM](proceedure/BLAM.md)
- [CLOSE](proceedure/CLOSE.md)
- [COUNTERMARCH](proceedure/COUNTERMARCH.md)
- [DMCM](proceedure/DMCM.md)
- [DMHS](proceedure/DMHS.md)
- [EVEN](proceedure/EVEN.md)
- [EWNS](proceedure/EWNS.md)
- [FM](proceedure/FM.md)
- [FMTO](proceedure/FMTO.md)
- [FOUNTAIN](proceedure/FOUNTAIN.md)
- [GRID](proceedure/GRID.md)
- [HSCM](proceedure/HSCM.md)
- [HSDM](proceedure/HSDM.md)
- [MAGIC](proceedure/MAGIC.md)
- [MARCH](proceedure/MARCH.md)
- [MT](proceedure/MT.md)
- [MTRM](proceedure/MTRM.md)
- [NSEW](proceedure/NSEW.md)
- [ROTATE](proceedure/ROTATE.md)
