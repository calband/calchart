---
title: CalChart User's Manual
version: 3.8
---

# HSDM

Format: `HSDM <destination>`

HSDM marches a two-part course to <destination>.  First, the
direction will be N, E, W, or S, and stepsize will be `HS`.  Then, the
direction will be NW, NE, SE, or SW, and stepsize will be `DM`.  The
specific directions are selected so that the path traveled is as short
as possible.

This procedure is similar to [`FOUNTAIN`](FOUNTAIN.md), but `HSDM` does
not require explicit directions.  It is also similar to [`DMHS`](DMHS.md).

