---
title: CalChart User's Manual
version: 3.8
---

# DMHS

Format: `DMHS <destination>`

DMHS marches a two-part course to `<destination>`.  First, the
direction will be NW, NE, SW, or SE, and stepsize will be `DM`.  Then,
the direction will be N, S, E, or W, and stepsize will be `HS`.  The
specific directions are selected so that the path traveled is as short
as possible.

This procedure is similar to [`FOUNTAIN`](FOUNTAIN.md), but `DMHS` does
not require explicit directions.  It is also similar to [`HSDM`](HSDM.md).
