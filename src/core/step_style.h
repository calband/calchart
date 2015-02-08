#pragma once

enum BaseStepStyle {
	Undefined,
	Close,
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

namespace CoreStepStyles {

	class Close : public StepStyle {
	private:
		using super = StepStyle;
	public:
		Close();
	};

	class HighStep : public StepStyle {
	private:
		using super = StepStyle;
	public:
		HighStep();
	};

	class MiniMilitary : public StepStyle {
	private:
		using super = StepStyle;
	public:
		MiniMilitary();
	};

	class ShowHigh : public StepStyle {
	private:
		using super = StepStyle;
	public:
		ShowHigh();
	};

	class JerkyStep : public StepStyle {
	private:
		using super = StepStyle;
	public:
		JerkyStep();
	};

	class Grapevine : public StepStyle {
	private:
		using super = StepStyle;
	public:
		Grapevine();
	};

	class Military : public StepStyle {
	private:
		using super = StepStyle;
	public:
		Military();
	};

	class TunnelStep : public StepStyle {
	private:
		using super = StepStyle;
	public:
		TunnelStep();
	};

	class FullFieldStep : public StepStyle {
	private:
		using super = StepStyle;
	public:
		FullFieldStep();
	};

	class JogStep : public StepStyle {
	private:
		using super = StepStyle;
	public:
		JogStep();
	};

}