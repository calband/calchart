/* cont.h
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

float BoundDirection(float f) {
  while (f >= 360.0) f -= 360.0;
  while (f < 0.0) f += 360.0;
  return f;
}

ContPoint::~ContPoint() {}

CC_coord& ContPoint::Get(AnimateCompile* anim) {
  return anim->pt.pos;
}

CC_coord& ContNextPoint::Get(AnimateCompile* anim) {
  if (anim->curr_sheet->next) {
    return anim->curr_sheet->next->pts[anim->curr_pt].pos;
  } else {
    return ContPoint::Get(anim);
  }
}

CC_coord& ContRefPoint::Get(AnimateCompile* anim) {
  return anim->curr_sheet->pts[anim->curr_pt].ref[refnum];
}

ContValue::~ContValue() {}

float ContValueFloat::Get(AnimateCompile*) {
  return val;
}

float ContValueDefined::Get(AnimateCompile*) {
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
  case CC_M:
    f = 4.0/3;
    break;
  case CC_DM:
    f = 1.4142136;
    break;
  }
  return f;
}

ContValueAdd::~ContValueAdd() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueAdd::Get(AnimateCompile* anim) {
  return (val1->Get(anim) + val2->Get(anim));
}

ContValueSub::~ContValueSub() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueSub::Get(AnimateCompile* anim) {
  return (val1->Get(anim) - val2->Get(anim));
}

ContValueMult::~ContValueMult() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueMult::Get(AnimateCompile* anim) {
  return (val1->Get(anim) * val2->Get(anim));
}

ContValueDiv::~ContValueDiv() {
  if (val1) delete val1;
  if (val2) delete val2;
}

float ContValueDiv::Get(AnimateCompile* anim) {
  float f;

  f = val2->Get(anim);
  if ((f < 0.00001) && (f > -0.00001)) {
    anim->SetStatus(FALSE);
    return 0.0;
  } else {
    return (val1->Get(anim) / f);
  }
}

ContValueNeg::~ContValueNeg() {
  if (val) delete val;
}

float ContValueNeg::Get(AnimateCompile* anim) {
  return -val->Get(anim);
}

float ContValueREM::Get(AnimateCompile* anim) {
  return anim->beats_rem;
}

float ContValueVar::Get(AnimateCompile* anim) {
  return anim->vars[varnum];
}

void ContValueVar::Set(AnimateCompile* anim, float v) {
  anim->vars[varnum] = v;
}

ContFuncDir::~ContFuncDir() {
  if (pnt) delete pnt;
}

float ContFuncDir::Get(AnimateCompile* anim) {
  return anim->pt.pos.Direction(pnt->Get(anim));
}

ContFuncDirFrom::~ContFuncDirFrom() {
  if (pnt_start) delete pnt_start;
  if (pnt_end) delete pnt_end;
}

float ContFuncDirFrom::Get(AnimateCompile* anim) {
  return pnt_start->Get(anim).Direction(pnt_end->Get(anim));
}

ContFuncDist::~ContFuncDist() {
  if (pnt) delete pnt;
}

float ContFuncDist::Get(AnimateCompile* anim) {
  CC_coord vector;

  vector = pnt->Get(anim) - anim->pt.pos;
  return vector.DM_Magnitude();
}

ContFuncDistFrom::~ContFuncDistFrom() {
  if (pnt_start) delete pnt_start;
  if (pnt_end) delete pnt_end;
}

float ContFuncDistFrom::Get(AnimateCompile* anim) {
  CC_coord vector;

  vector = pnt_end->Get(anim) - pnt_start->Get(anim);
  return vector.Magnitude();
}

ContFuncEither::~ContFuncEither() {
  if (dir1) delete dir1;
  if (dir2) delete dir2;
  if (pnt) delete pnt;
}

float ContFuncEither::Get(AnimateCompile*) {
  // TODO
  return 0.0;
}

ContFuncOpp::~ContFuncOpp() {
  if (dir) delete dir;
}

float ContFuncOpp::Get(AnimateCompile* anim) {
  return (dir->Get(anim) + 180.0);
}

ContFuncStep::~ContFuncStep() {
  if (numbeats) delete numbeats;
  if (blksize) delete blksize;
  if (pnt) delete pnt;
}

float ContFuncStep::Get(AnimateCompile* anim) {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  return (c.Magnitude() * numbeats->Get(anim) / blksize->Get(anim));
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
  anim->Append(new AnimateCommandMove(anim->beats_rem, c));
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
}

ContProcDMCM::~ContProcDMCM() {
  if (pnt1) delete pnt1;
  if (pnt2) delete pnt2;
  if (numbeats) delete numbeats;
}

void ContProcDMCM::Compile(AnimateCompile* anim) {
}

ContProcDMHS::~ContProcDMHS() {
  if (pnt) delete pnt;
}

void ContProcDMHS::Compile(AnimateCompile* anim) {
}

ContProcEven::~ContProcEven() {
  if (stps) delete stps;
  if (pnt) delete pnt;
}

void ContProcEven::Compile(AnimateCompile* anim) {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  anim->Append(new AnimateCommandMove((unsigned)stps->Get(anim), c));
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
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2))) {
      return;
    }
  }
  if (c1.x != 0) {
    c2.x = c1.x;
    c2.y = 0;
    b = COORD2INT(c2.x);
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2))) {
      return;
    }
  }
}

ContProcFountain::~ContProcFountain() {
  if (dir1) delete dir1;
  if (dir2) delete dir2;
}

void ContProcFountain::Compile(AnimateCompile* anim) {
}

ContProcFM::~ContProcFM() {
  if (stps) delete stps;
  if (dir) delete dir;
}

void ContProcFM::Compile(AnimateCompile* anim) {
  CC_coord c;
  float rads, mag, d;

  d = dir->Get(anim);
  d = BoundDirection(d);
  if ((ABS(d - 45.0) < 0.00001) ||
      (ABS(d - 135.0) < 0.00001) ||
      (ABS(d - 225.0) < 0.00001) ||
      (ABS(d - 315.0) < 0.00001)) {
    c.x = c.y = FLOAT2COORD(stps->Get(anim));
    if ((d > 50.0) && (d < 310.0)) c.x = -c.x;
    if (d < 180.0) c.y = -c.y;
  } else {
    rads = d * PI / 180.0;
    mag = stps->Get(anim);
    c.x = FLOAT2COORD(cos(rads)*mag);
    c.y = -(FLOAT2COORD(sin(rads)*mag));
  }
  anim->Append(new AnimateCommandMove((unsigned)stps->Get(anim), c));
}

ContProcFMTO::~ContProcFMTO() {
  if (pnt) delete pnt;
}

void ContProcFMTO::Compile(AnimateCompile* anim) {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  
  anim->Append(new AnimateCommandMove((unsigned)c.DM_Magnitude(), c));
}

ContProcGrid::~ContProcGrid() {
  if (grid) delete grid;
}

void ContProcGrid::Compile(AnimateCompile* anim) {
}

ContProcHSCM::~ContProcHSCM() {
  if (pnt1) delete pnt1;
  if (pnt2) delete pnt2;
  if (numbeats) delete numbeats;
}

void ContProcHSCM::Compile(AnimateCompile* anim) {
}

ContProcHSDM::~ContProcHSDM() {
  if (pnt) delete pnt;
}

void ContProcHSDM::Compile(AnimateCompile* anim) {
}

ContProcMagic::~ContProcMagic() {
  if (pnt) delete pnt;
}

void ContProcMagic::Compile(AnimateCompile* anim) {
  CC_coord c;

  c = pnt->Get(anim) - anim->pt.pos;
  anim->Append(new AnimateCommandMove(0, c));
}

ContProcMarch::~ContProcMarch() {
  if (stpsize) delete stpsize;
  if (stps) delete stps;
  if (dir) delete dir;
}

void ContProcMarch::Compile(AnimateCompile* anim) {
  CC_coord c;
  float rads, mag;

  rads = dir->Get(anim) * PI / 180.0;
  mag = stpsize->Get(anim) * stps->Get(anim);
  c.x = FLOAT2COORD(cos(rads)*mag);
  c.y = -(FLOAT2COORD(sin(rads)*mag));
  anim->Append(new AnimateCommandMove((unsigned)stps->Get(anim), c));
}

ContProcMT::~ContProcMT() {
  if (numbeats) delete numbeats;
  if (dir) delete dir;
}

void ContProcMT::Compile(AnimateCompile* anim) {
  anim->Append(new AnimateCommandMT((unsigned)numbeats->Get(anim),
				    AnimGetDirFromAngle(dir->Get(anim))));
}

ContProcMTRM::~ContProcMTRM() {
  if (dir) delete dir;
}

void ContProcMTRM::Compile(AnimateCompile* anim) {
  anim->Append(new AnimateCommandMT(anim->beats_rem,
				    AnimGetDirFromAngle(dir->Get(anim))));
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
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2))) {
      return;
    }
  }
  if (c1.y != 0) {
    c2.x = 0;
    c2.y = c1.y;
    b = COORD2INT(c2.y);
    if (!anim->Append(new AnimateCommandMove(ABS(b), c2))) {
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
}
