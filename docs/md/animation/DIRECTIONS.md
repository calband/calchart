---
title: CalChart User's Manual
version: 3.8
---

# DIRECTIONS

CalChart uses the mathematical convention to assign degree values to
directions.  Any real number is acceptable for a direction, including
negative values.  The program uses the following symbols to represent the
most commonly used directions:

| **NAME** | **DIRECTION** | **VALUE** |
|----------|---------------|-----------|
| N        | north         | 0         |
| NW       | northwest     | 45        |
| W        | west          | 90        |
| SW       | southwest     | 135       |
| S        | south         | 180       |
| SE       | southeast     | 225       |
| E        | east          | 270       |
| NE       | northeast     | 315       |

A special symbol for the "direction of flow" is called DOF.  DOF contains
the direction of the last movement or marktime (note that it is undefined
if none has occurred).

A special symbol for the "direction of horn" is called DOH.  DOH contains
the direction faced after the last movement or marktime (note that it is
undefined if none has occurred).  This is only different from DOF for
things like grapevine ([march](proceedures/MARCH.md).)

Any function which returns a direction may be used in place of a direction;
see the section on [functions](FUNCTIONS.md).


