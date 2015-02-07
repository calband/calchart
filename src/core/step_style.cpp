#include "step_style.h"
#include "math_utils.h"

StepStyle::StepStyle(std::string briefName, BaseStepStyle baseStyle, float stepsize, float numBeatsPerStep, bool facingDirectionIsAbsolute, float facingDirection)
: mBriefName(briefName), mBaseStyle(baseStyle), mStepSize(stepsize), mNumBeatsPerStep(numBeatsPerStep), mHasAbsoluteFacingDirection(facingDirectionIsAbsolute), mFacingDirection(facingDirection)
{}

StepStyle::StepStyle(const StepStyle& other)
: mBriefName(other.mBriefName), mBaseStyle(other.mBaseStyle), mStepSize(other.mStepSize), mNumBeatsPerStep(other.mNumBeatsPerStep), mHasAbsoluteFacingDirection(other.mHasAbsoluteFacingDirection), mFacingDirection(other.mFacingDirection)
{}

bool StepStyle::hasAbsoluteFacingDirection() const {
	return mHasAbsoluteFacingDirection;
}

float StepStyle::getFacingDirection(float movementDirection = 0) const {
	if (mHasAbsoluteFacingDirection) {
		return mFacingDirection;
	} else {
		return movementDirection + mFacingDirection;
	}
}

float StepStyle::getNumBeatsPerStep() const {
	return mNumBeatsPerStep;
}

float StepStyle::getStepSize() const {
	return mStepSize;
}

std::string StepStyle::getBriefName() const {
	return mBriefName;
}

BaseStepStyle StepStyle::getBaseStyle() const {
	return mBaseStyle;
}

StepStyle StepStyle::deriveNewAbsoluteFacingDirection(float newAbsoluteFacingDir) const {
	StepStyle returnVal(*this);
	returnVal.mHasAbsoluteFacingDirection = true;
	returnVal.mFacingDirection = newAbsoluteFacingDir;
	return returnVal;
}

StepStyle StepStyle::deriveNewRelativeFacingDirection(float newRelativeFacingDir) const {
	StepStyle returnVal(*this);
	returnVal.mHasAbsoluteFacingDirection = false;
	returnVal.mFacingDirection = newRelativeFacingDir;
	return returnVal;
}

StepStyle StepStyle::deriveNewBeatsPerStep(float newBeatsPerStep) const {
	StepStyle returnVal(*this);
	returnVal.mNumBeatsPerStep = newBeatsPerStep;
	return returnVal;
}

StepStyle StepStyle::deriveNewStepSize(float newSize) const {
	StepStyle returnVal(*this);
	returnVal.mStepSize = newSize;
	return returnVal;
}

StepStyle StepStyle::deriveNewBriefName(std::string newName) const {
	StepStyle returnVal(*this);
	returnVal.mBriefName = newName;
	return returnVal;
}

StepStyle StepStyle::deriveNewBaseStyle(BaseStepStyle newStyle) const {
	StepStyle returnVal(*this);
	returnVal.mBaseStyle = newStyle;
	return returnVal;
}

StepStyle StepStyle::deriveDiagonalStepSize() const {
	StepStyle returnVal(*this);
	returnVal.mStepSize *= static_cast<float>(SQRT2);
	return returnVal;
}

StepStyle_HighStep::StepStyle_HighStep()
: super("HS", BaseStepStyle::HighStep, 1, 1, false, 0)
{}

StepStyle_MiniMilitary::StepStyle_MiniMilitary()
: super("MM", BaseStepStyle::MiniMilitary, 1, 1, false, 0)
{}

StepStyle_ShowHigh::StepStyle_ShowHigh()
: super("SH", BaseStepStyle::ShowHigh, 1, 2, false, 0)
{}

StepStyle_JerkyStep::StepStyle_JerkyStep()
: super("JS", BaseStepStyle::JerkyStep, 1, 2, false, 0)
{}

StepStyle_Grapevine::StepStyle_Grapevine()
: super("GV", BaseStepStyle::Grapevine, 1, 1, false, 90)
{}

StepStyle_Military::StepStyle_Military()
: super("M", BaseStepStyle::Military, (8.0 / 6.0), 1, false, 0)
{}

StepStyle_TunnelStep::StepStyle_TunnelStep()
: super("TS", BaseStepStyle::TunnelStep, (8.0 / 6.0), 1, false, 0)
{}

StepStyle_FullFieldStep::StepStyle_FullFieldStep()
: super("FF", BaseStepStyle::FullFieldStep, 1, 1, false, 0)
{}

StepStyle_JogStep::StepStyle_JogStep()
: super("JS", BaseStepStyle::JogStep, 1, 1, false, 0)
{}
