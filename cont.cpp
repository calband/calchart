/* cont.cc
 * Classes for continuity
 *
 * Modification history:
 * 1-3-95     Garrick Meeker              Created
 *
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "cont.h"
#include <math.h>
#include "parse.h"

int float2int(const ContProcedure *proc,
	      AnimateCompile *anim,
	      float f) {
  int v = (int)floor(f+0.5);
  if (ABS(f - (float)v) >= COORD_DECIMAL) {
    anim->RegisterError(ANIMERR_NONINT, proc);
  }
  return v;
}

unsigned float2unsigned(const ContProcedure *proc,
			AnimateCompile *anim,
			float f) {
  int v = float2int(proc, anim, f);
  if (v < 0) {
    anim->RegisterError(ANIMERR_NEGINT, proc);
    return 0;
  } else {
    return (unsigned)v;
  }
}

void DoCounterMarch(const ContProcedure *proc,
		    AnimateCompile* anim, ContPoint *pnt1, ContPoint *pnt2,
		    ContValue *stps, ContValue *dir1, ContValue *dir2,
		    ContValue *numbeats) {
  CC_coord p[4];
  CC_coord v1, v2;
  CC_coord ref1, ref2;
  float steps1, steps2, d1, d2, beats;
  float c;
  unsigned leg;

  //use the law of sines to compute components
  ref1 = pnt1->Get(anim);
  ref2 = pnt2->Get(anim);
  steps1 = stps->Get(anim);
  d1 = dir1->Get(anim);
  d2 = dir2->Get(anim);
  beats = numbeats->Get(anim);
  c = sin(DEG2RAD(d1 - d2));
  if (IS_ZERO(c)) {
    anim->RegisterError(ANIMERR_INVALID_CM, proc);
    return;
  }
  CreateVector(v1, d1, steps1);
  p[1] = ref1 + v1;
  steps2 = (ref2 - p[1]).Magnitude() * sin(DEG2RAD(ref2.Direction(p[1]) - d1))
    / c;
  if (IsDiagonalDirection(d2)) {
    steps2 /= SQRT2;
  }
  CreateVector(v2, d2, steps2);
  p[2] = p[1] + v2;
  p[3] = ref2 - v1;
  p[0] = p[3] - v2;

  v1 = p[1] - anim->pt.pos;
  c = BoundDirectionSigned(v1.Direction() - d1);
  if ((v1 != 0) && (IS_ZERO(c))) {
    leg = 1;
  } else {
    v1 = p[2] - anim->pt.pos;
    c = BoundDirectionSigned(v1.Direction() - d2);
    if ((v1 != 0) && (IS_ZERO(c))) {
      leg = 2;
    } else {
      v1 = p[3] - anim->pt.pos;
      c = BoundDirectionSigned(v1.Direction() - d1 - 180.0);
      if ((v1 != 0) && (IS_ZERO(c))) {
	leg = 3;
      } else {
	v1 = p[0] - anim->pt.pos;
	c = BoundDirectionSigned(v1.Direction() - d2 - 180.0);
	if ((v1 != 0) && (IS_ZERO(c))) {
	  leg = 0;
	} else {
	  // Current point is not in path of countermarch
	  anim->RegisterError(ANIMERR_INVALID_CM, proc);
	  return;
	}
      }
    }
  }

  while (beats > 0) {
    v1 = p[leg] - anim->pt.pos;
    c = v1.DM_Magnitude();
    if (c <= beats) {
      beats -= c;
      if (!anim->Append(new AnimateCommandMove(float2unsigned(proc, anim, c),
					       v1), proc)) {
	return;
      }
    } else {
      switch(leg) {
      case 0:
	CreateVector(v1, d2+180.0, beats);
	break;
      case 1:
	CreateVector(v1, d1, beats);
	break;
      case 2:
	CreateVector(v1, d2, beats);
	break;
      default:
	CreateVector(v1, d1+180.0, beats);
	break;
      }
      anim->Append(new AnimateCommandMove(float2unsigned(proc, anim, beats),
					  v1), proc);
      return;
    }
    leg++;
    if (leg > 3) leg = 0;
  }
}

ContToken::ContToken(): line(yylloc.first_line), col(yylloc.first_column) {}
ContToken::~ContToken() {}

ContPoint::~ContPoint() {}

const CC_coord& ContPoint::Get(AnimateCompile* anim) const {
  return anim->pt.pos;
}

const CC_coord& ContStartPoint::Get(AnimateCompile* anim) const {
  return anim->curr_sheet->GetPosition(anim->curr_pt);
}

const CC_coord& ContNextPoint::Get(AnimateCompile* anim) const {
  CC_sheet *sheet = anim->curr_sheet->next; 

  while (1) {
    if (sheet == NULL) {
      anim->RegisterError(ANIMERR_UNDEFINED, this);
      return ContPoint::Get(anim);
    }
    if (sheet->IsInAnimation()) {
      return sheet->GetPosition(anim->curr_pt);
    }
    sheet = sheet->next;
  }
}

const CC_coord& ContRefPoint::Get(AnimateCompile* anim) const {
  return anim->curr_sheet->GetPosition(anim->curr_pt, refnum+1);
}

ContValue::~ContValue() {}

float ContValueFloat::Get(AnimateCompile*) const {
  return val;
}

float ContValueDefined::Get(AnimateCompile*) const {
  float f;

  switch (val) {
  default:
    f = 0.0;
    break;
  case CC_NW:
    f = 45.0;
    break;
  case CC_W:
    f = 90.0;
    break;
  case CC_SW:
    f = 135.0;
    break;
  case CC_S:
    f = 180.0;
    break;
  case CC_SE:
    f = 225.0;
    break;
  case CC_E:
    f = 270.0;
    break;
  case CC_NE:
    f = 315.0;
    break;
  case CC_HS:
    f = 1.0;
    break;
  case CC_MM:
    f = 1.0;
    break;
  case CC_SH:
    f = 0.5;
    break;
  case CC_JS:
    f = 0.5;
    break;
  case CC_GV:
    f = 1.0;
    break;
  case CC_M:
    f = 4.0/3;
    break;
  case CC_DM:
    f = SQRT2;
    break;
  }
  return f;
}

ContValueAdd::~ContValueAdd() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueAdd::Get(AnimateCompile* anim) const {
  return (val1->Get(anim) + val2->Get(anim));
}

ContValueSub::~ContValueSub() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueSub::Get(AnimateCompile* anim) const {
  return (val1->Get(anim) - val2->Get(anim));
}

ContValueMult::~ContValueMult() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueMult::Get(AnimateCompile* anim) const {
  return (val1->Get(anim) * val2->Get(anim));
}

ContValueDiv::~ContValueDiv() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueDiv::Get(AnimateCompile* anim) const {
  float f;

  f = val2->Get(anim);
  if (IS_ZERO(f)) {
    anim->RegisterError(ANIMERR_DIVISION_ZERO, this);
    return 0.0;
  } else {
    return (val1->Get(anim) / f);
  }
}

ContValueNeg::~ContValueNeg() {
  if (val) delete val;
}

float ContValueNeg::Get(AnimateCompile* anim) const {
  return -val->Get(anim);
}

float ContValueREM::Get(AnimateCompile* anim) const {
  return anim->beats_rem;
}

float ContValueVar::Get(AnimateCompile* anim) const {
  return anim->GetVarValue(varnum, this);
}

void ContValueVar::Set(AnimateCompile* anim, float v) {
  anim->SetVarValue(varnum, v);
}

ContFuncDir::~ContFuncDir() {
  if (pnt) delete pnt;
}

float ContFuncDir::Get(AnimateCompile* anim) const {
  CC_coord c = pnt->Get(anim);
  if (c == anim->pt.pos) {
    anim->RegisterError(ANIMERR_UNDEFINED, this);
  }
  return anim->pt.pos.Direction(c);
}

ContFuncDirFrom::~ContFuncDirFrom() {
  if (pnt_start) delete pnt_start;
  if (pnt_end) delete pnt_end;
}

float ContFuncDirFrom::Get(AnimateCompile* anim) const {
  CC_coord start = pnt_start->Get(anim);
  CC_coord end = pnt_end->Get(anim);
  if (start == end) {
    anim->RegisterError(ANIMERR_UNDEFINED, this);
  }
  return start.Direction(end);
}

ContFuncDist::~ContFuncDist() {
  if (pnt) delete pnt;
}

float ContFuncDist::Get(AnimateCompile* anim) const {
  CC_coord vector;

  vector = pnt->Get(anim) - anim->pt.pos;
  return vector.DM_Magnitude();
}

ContFuncDistFrom::~ContFuncDistFrom() {
  if (pnt_start) delete pnt_start;
  if (pnt_end) delete pnt_end;
}

float ContFuncDistFrom::Get(AnimateCompile* anim) const {
  CC_coord vector;

  vector = pnt_end->Get(anim) - pnt_start->Get(anim);
  return vector.Magnitude();
}

ContFuncEither::~ContFuncEither() {
  if (dir1) delete dir1;
  if (dir2) delete dir2;
  if (pnt) delete pnt;
}

float ContFuncEither::Get(AnimateCompile* anim) const {
  float dir;
  float d1, d2;
  CC_coord c = pnt->Get(anim);

  if (anim->pt.pos == c) {
    anim->RegisterError(ANIMERR_UNDEFINED, this);
    return dir1->Get(anim);
  }
  dir = anim->pt.pos.Direction(c);
  d1 = dir1->Get(anim) - dir;
  d2 = dir2->Get(anim) - dir;
  while (d1 > 180) d1-=360;
  while (d1 < -180) d1+=360;
  while (d2 > 180) d2-=360;
  while (d2 < -180) d2+=360;
  return (ABS(d1) > ABS(d2)) ? dir2->Get(anim) : dir1->Get(anim);
}

ContFuncOpp::~ContFuncOpp() {
  if (dir) delete dir;
}

float ContFuncOpp::Get(AnimateCompile* anim) const {
  return (dir->Get(anim) + 180.0);
}

ContFuncStep::~ContFuncStep() {
  if (numbeats) delete numbeats;
  if (blksize) delete blksize;
  if (pnt) delete pnt;
}

float ContFuncStep::Get(AnimateCompile* anim) const {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  return (c.DM_Magnitude() * numbeats->Get(anim) / blksize->Get(anim));
}

ContProcedure::~ContProcedure() {}

ContProcSet::~ContProcSet() {
  if (var) delete var;
  if (val) delete val;
}

void ContProcSet::Compile(AnimateCompile* anim) {
  var->Set(anim, val->Get(anim));
}

void ContProcBlam::Compile(AnimateCompile* anim) {
  ContNextPoint np;
  CC_coord c;

  c = np.Get(anim) - anim->pt.pos;
  anim->Append(new AnimateCommandMove(anim->beats_rem, c), this);
}

ContProcCM::~ContProcCM() {
  if (pnt1) delete pnt1;
  if (pnt2) delete pnt2;
  if (stps) delete stps;
  if (dir1) delete dir1;
  if (dir2) delete dir2;
  if (numbeats) delete numbeats;
}

void ContProcCM::Compile(AnimateCompile* anim) {
  DoCounterMarch(this, anim, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

ContProcDMCM::~ContProcDMCM() {
  if (pnt1) delete pnt1;
  if (pnt2) delete pnt2;
  if (numbeats) delete numbeats;
}

void ContProcDMCM::Compile(AnimateCompile* anim) {
  CC_coord r1, r2;
  Coord c;
  ContValueFloat steps(1.0);

  r1 = pnt1->Get(anim);
  r2 = pnt2->Get(anim);
  c = r2.x - r1.x;
  if (c == (r2.y - r1.y + INT2COORD(2))) {
    if (c >= 0) {
      ContValueDefined dir1(CC_SW);
      ContValueDefined dir2(CC_W);
      DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
      return;
    }
  } else if (c == (r1.y - r2.y - INT2COORD(2))) {
    if (c >= 0) {
      ContValueDefined dir1(CC_SE);
      ContValueDefined dir2(CC_W);
      DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
      return;
    }
  } else if (c == (r1.y - r2.y + INT2COORD(2))) {
    if (c <= 0) {
      ContValueDefined dir1(CC_NW);
      ContValueDefined dir2(CC_E);
      DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
      return;
    }
  } else if (c == (r2.y - r1.y - INT2COORD(2))) {
    if (c <= 0) {
      ContValueDefined dir1(CC_NE);
      ContValueDefined dir2(CC_E);
      DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
      return;
    }
  }
  anim->RegisterError(ANIMERR_INVALID_CM, this);
}

ContProcDMHS::~ContProcDMHS() {
  if (pnt) delete pnt;
}

void ContProcDMHS::Compile(AnimateCompile* anim) {
  CC_coord c, c_hs, c_dm;
  short b, b_hs;

  c = pnt->Get(anim) - anim->pt.pos;
  if (ABS(c.x) > ABS(c.y)) {
    c_hs.x = ((c.x<0) != (c.y<0)) ? c.x+c.y : c.x-c.y; // adjust sign
    c_hs.y = 0;
    c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y; // adjust sign
    c_dm.y = c.y;
    b_hs = COORD2INT(c_hs.x);
  } else {
    c_hs.x = 0;
    c_hs.y = ((c.x<0) != (c.y<0)) ? c.y+c.x : c.y-c.x; // adjust sign
    c_dm.x = c.x;
    c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x; // adjust sign
    b_hs = COORD2INT(c_hs.y);
  }
  if (c_dm != 0) {
    b = COORD2INT(c_dm.x);
    if (!anim->Append(new AnimateCommandMove(ABS(b), c_dm), this)) {
      return;
    }
  }
  if (c_hs != 0) {
    anim->Append(new AnimateCommandMove(ABS(b_hs), c_hs), this);
  }
}

ContProcEven::~ContProcEven() {
  if (stps) delete stps;
  if (pnt) delete pnt;
}

void ContProcEven::Compile(AnimateCompile* anim) {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  int steps = float2int(this, anim, stps->Get(anim));
  if (steps < 0) {
    anim->Append(new AnimateCommandMove((unsigned)-steps, c,
					-c.Direction()), this);
  } else {
    anim->Append(new AnimateCommandMove((unsigned)steps, c), this);
  }
}

ContProcEWNS::~ContProcEWNS() {
  if (pnt) delete pnt;
}

void ContProcEWNS::Compile(AnimateCompile* anim) {
  CC_coord c1, c2;
  short b;

  c1 = pnt->Get(anim) - anim->pt.pos;
  if (c1.y != 0) {
    c2.x = 0;
    c2.y = c1.y;
    b = COORD2INT(c2.y);
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2), this)) {
      return;
    }
  }
  if (c1.x != 0) {
    c2.x = c1.x;
    c2.y = 0;
    b = COORD2INT(c2.x);
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2), this)) {
      return;
    }
  }
}

ContProcFountain::~ContProcFountain() {
  if (dir1) delete dir1;
  if (dir2) delete dir2;
  if (stepsize1) delete stepsize1;
  if (stepsize2) delete stepsize2;
}

void ContProcFountain::Compile(AnimateCompile* anim) {
  float a, b, c, d, e, f;
  float f1, f2;
  CC_coord v;

  f1 = dir1->Get(anim);
  if (stepsize1) {
    f2 = stepsize1->Get(anim);
    a = f2 * cos(DEG2RAD(f1));
    c = f2 * -sin(DEG2RAD(f1));
  } else {
    CreateUnitVector(a, c, f1);
  }
  f1 = dir2->Get(anim);
  if (stepsize2) {
    f2 = stepsize2->Get(anim);
    b = f2 * cos(DEG2RAD(f1));
    d = f2 * -sin(DEG2RAD(f1));
  } else {
    CreateUnitVector(b, d, f1);
  }
  v = pnt->Get(anim) - anim->pt.pos;
  e = COORD2FLOAT(v.x);
  f = COORD2FLOAT(v.y);
  f1 = a*d - b*c;
  if (IS_ZERO(f1)) {
    if (IS_ZERO(a-b) && IS_ZERO(c-d) && IS_ZERO(e*c-a*f)) {
      // Special case: directions are same
      if (IS_ZERO(c)) {
	f1 = f/c;
      } else {
	f1 = e/a;
      }
      if (!anim->Append(new AnimateCommandMove(float2unsigned(this, anim, f1),
					       v), this)) {
	return;
      }
    } else {
      anim->RegisterError(ANIMERR_INVALID_FNTN, this);
      return;
    }
  } else {
    f2 = (d*e - b*f) / f1;
    if (!IS_ZERO(f2)) {
      v.x = FLOAT2COORD(f2*a);
      v.y = FLOAT2COORD(f2*c);
      if (!anim->Append(new AnimateCommandMove(float2unsigned(this, anim, f2),
					       v), this)) {
	return;
      }
    }
    f2 = (a*f - c*e) / f1;
    if (!IS_ZERO(f2)) {
      v.x = FLOAT2COORD(f2*b);
      v.y = FLOAT2COORD(f2*d);
      if (!anim->Append(new AnimateCommandMove(float2unsigned(this, anim, f2),
					       v), this)) {
	return;
      }
    }
  }
}

ContProcFM::~ContProcFM() {
  if (stps) delete stps;
  if (dir) delete dir;
}

void ContProcFM::Compile(AnimateCompile* anim) {
  CC_coord c;
  int b;

  b = float2int(this, anim, stps->Get(anim));
  if (b != 0) {
    CreateVector(c, dir->Get(anim), stps->Get(anim));
    if (c != 0) {
      if (b < 0) {
	anim->Append(new AnimateCommandMove((unsigned)-b, c,
					    -c.Direction()), this);
      } else {
	anim->Append(new AnimateCommandMove((unsigned)b, c), this);
      }
    }
  }
}

ContProcFMTO::~ContProcFMTO() {
  if (pnt) delete pnt;
}

void ContProcFMTO::Compile(AnimateCompile* anim) {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  if (c != 0) {
    anim->Append(new AnimateCommandMove((unsigned)c.DM_Magnitude(), c),
		 this);
  }
}

ContProcGrid::~ContProcGrid() {
  if (grid) delete grid;
}

static inline Coord roundcoord(Coord a, Coord mod) {
  mod = ABS(mod);
  if (mod > 0) {
    if (a < 0) {
      a = ((a-(mod/2))/mod)*mod;
    } else {
      a = ((a+(mod/2))/mod)*mod;
    }
  }
  return a;
}

void ContProcGrid::Compile(AnimateCompile* anim) {
  Coord gridc;
  CC_coord c;

  gridc = FLOAT2COORD(grid->Get(anim));

  c.x = roundcoord(anim->pt.pos.x, gridc);
  // Adjust so 4 step grid will be on visible grid
  c.y = roundcoord(anim->pt.pos.y-INT2COORD(2), gridc) + INT2COORD(2);

  c -= anim->pt.pos;
  if (c != 0) {
    anim->Append(new AnimateCommandMove(0, c), this);
  }
}

ContProcHSCM::~ContProcHSCM() {
  if (pnt1) delete pnt1;
  if (pnt2) delete pnt2;
  if (numbeats) delete numbeats;
}

void ContProcHSCM::Compile(AnimateCompile* anim) {
  CC_coord r1, r2;
  ContValueFloat steps(1.0);

  r1 = pnt1->Get(anim);
  r2 = pnt2->Get(anim);
  switch (r1.y - r2.y) {
  case INT2COORD(2):
    if (r2.x >= r1.x) {
      ContValueDefined dirs(CC_S);
      ContValueDefined dirw(CC_W);
      DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dirs, &dirw, numbeats);
      return;
    }
    break;
  case -INT2COORD(2):
    if (r1.x >= r2.x) {
      ContValueDefined dirn(CC_N);
      ContValueDefined dire(CC_E);
      DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dirn, &dire, numbeats);
      return;
    }
    break;
  default:
    break;
  }
  anim->RegisterError(ANIMERR_INVALID_CM, this);
}

ContProcHSDM::~ContProcHSDM() {
  if (pnt) delete pnt;
}

void ContProcHSDM::Compile(AnimateCompile* anim) {
  CC_coord c, c_hs, c_dm;
  short b;

  c = pnt->Get(anim) - anim->pt.pos;
  if (ABS(c.x) > ABS(c.y)) {
    c_hs.x = ((c.x<0) != (c.y<0)) ? c.x+c.y : c.x-c.y; // adjust sign
    c_hs.y = 0;
    c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y; // adjust sign
    c_dm.y = c.y;
    b = COORD2INT(c_hs.x);
  } else {
    c_hs.x = 0;
    c_hs.y = ((c.x<0) != (c.y<0)) ? c.y+c.x : c.y-c.x; // adjust sign
    c_dm.x = c.x;
    c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x; // adjust sign
    b = COORD2INT(c_hs.y);
  }
  if (c_hs != 0) {
    if (!anim->Append(new AnimateCommandMove(ABS(b), c_hs), this)) {
      return;
    }
  }
  if (c_dm != 0) {
    b = COORD2INT(c_dm.x);
    anim->Append(new AnimateCommandMove(ABS(b), c_dm), this);
  }
}

ContProcMagic::~ContProcMagic() {
  if (pnt) delete pnt;
}

void ContProcMagic::Compile(AnimateCompile* anim) {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  anim->Append(new AnimateCommandMove(0, c), this);
}

ContProcMarch::~ContProcMarch() {
  if (stpsize) delete stpsize;
  if (stps) delete stps;
  if (dir) delete dir;
}

void ContProcMarch::Compile(AnimateCompile* anim) {
  CC_coord c;
  float rads, mag;
  int b;

  b = float2int(this, anim, stps->Get(anim));
  if (b != 0) {
    rads = DEG2RAD(dir->Get(anim));
    mag = stpsize->Get(anim) * stps->Get(anim);
    c.x = FLOAT2COORD(cos(rads)*mag);
    c.y = -(FLOAT2COORD(sin(rads)*mag));
    if (c != 0) {
      if (facedir)
	anim->Append(new AnimateCommandMove((unsigned)ABS(b), c,
					    facedir->Get(anim)), this);
      else
	if (b < 0) {
	  anim->Append(new AnimateCommandMove((unsigned)-b, c,
					      -c.Direction()), this);
	} else {
	  anim->Append(new AnimateCommandMove((unsigned)b, c), this);
	}
    }
  }
}

ContProcMT::~ContProcMT() {
  if (numbeats) delete numbeats;
  if (dir) delete dir;
}

void ContProcMT::Compile(AnimateCompile* anim) {
  int b;

  b = float2int(this, anim, numbeats->Get(anim));
  if (b != 0) {
    anim->Append(new AnimateCommandMT((unsigned)ABS(b), dir->Get(anim)), this);
  }
}

ContProcMTRM::~ContProcMTRM() {
  if (dir) delete dir;
}

void ContProcMTRM::Compile(AnimateCompile* anim) {
  anim->Append(new AnimateCommandMT(anim->beats_rem,
				    dir->Get(anim)), this);
}

ContProcNSEW::~ContProcNSEW() {
  if (pnt) delete pnt;
}

void ContProcNSEW::Compile(AnimateCompile* anim) {
  CC_coord c1, c2;
  short b;

  c1 = pnt->Get(anim) - anim->pt.pos;
  if (c1.x != 0) {
    c2.x = c1.x;
    c2.y = 0;
    b = COORD2INT(c2.x);
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2), this)) {
      return;
    }
  }
  if (c1.y != 0) {
    c2.x = 0;
    c2.y = c1.y;
    b = COORD2INT(c2.y);
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2), this)) {
      return;
    }
  }
}

ContProcRotate::~ContProcRotate() {
  if (ang) delete ang;
  if (stps) delete stps;
  if (pnt) delete pnt;
}

void ContProcRotate::Compile(AnimateCompile* anim) {
  float start_ang;
  CC_coord c;
  CC_coord rad;

  // Most of the work is converting to polar coordinates
  c = pnt->Get(anim);
  rad = anim->pt.pos - c;
  if (c == anim->pt.pos)
    start_ang = anim->GetVarValue(CONTVAR_DOH, this);
  else
    start_ang = c.Direction(anim->pt.pos);
  int b = float2int(this, anim, stps->Get(anim));
  float angle = ang->Get(anim);
  bool backwards = false;
  if (b < 0) {
    backwards = true;
  }
  anim->Append(new AnimateCommandRotate((unsigned)ABS(b), c,
					// Don't use Magnitude() because
					// we want Coord numbers
					sqrt(rad.x*rad.x + rad.y*rad.y),
					start_ang, start_ang+angle,
					backwards),
	       this);
}
