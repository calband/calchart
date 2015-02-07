#pragma once

enum BaseStepStyle {
	Undefined,
	HighStep,
	MiniMilitary,
	ShowHigh,
	JerkyStep,
	Grapevine,
	Military,
	TunnelStep,
	FullFieldStep,
	JogStep
};

class StepStyle {
public:
	StepStyle(std::string briefName, BaseStepStyle baseStyle, float stepsize, float numBeatsPerStep, bool facingDirectionIsAbsolute, float facingDirection);
	StepStyle(const StepStyle& other);

	bool hasAbsoluteFacingDirection() const;
	float getFacingDirection(float movementDirection = 0) const;
	float getNumBeatsPerStep() const;
	float getStepSize() const;
	std::string getBriefName() const;
	BaseStepStyle getBaseStyle() const;

	StepStyle deriveNewAbsoluteFacingDirection(float newAbsoluteFacingDir) const;
	StepStyle deriveNewRelativeFacingDirection(float newRelativeFacingDir) const;
	StepStyle deriveNewBeatsPerStep(float newBeatsPerStep) const;
	StepStyle deriveNewStepSize(float newSize) const;
	StepStyle deriveNewBriefName(std::string newName) const;
	StepStyle deriveNewBaseStyle(BaseStepStyle newStyle) const;
	StepStyle deriveDiagonalStepSize() const;
private:
	bool mHasAbsoluteFacingDirection;
	float mFacingDirection;
	float mNumBeatsPerStep;
	float mStepSize;
	std::string mBriefName;
	BaseStepStyle mBaseStyle;
};

class StepStyle_HighStep : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_HighStep();
};

class StepStyle_MiniMilitary : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_MiniMilitary();
};

class StepStyle_ShowHigh : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_ShowHigh();
};

class StepStyle_JerkyStep : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_JerkyStep();
};

class StepStyle_Grapevine : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_Grapevine();
};

class StepStyle_Military : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_Military();
};

class StepStyle_TunnelStep : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_TunnelStep();
};

class StepStyle_FullFieldStep : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_FullFieldStep();
};

class StepStyle_JogStep : public StepStyle {
private:
	using super = StepStyle;
public:
	StepStyle_JogStep();
};