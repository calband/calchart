#ifndef _ANIMATE_PLAYER_TOOL_H_
#define _ANIMATE_PLAYER_TOOL_H_

#include "animation_frame.h"
#include "animation_view.h"
#include "play_animation_controller.h"
#include "animation_player_module.h"

class AnimationPlayerTool {
public:
	AnimationPlayerTool(AnimationPlayerModule* module);

	virtual void activate() = 0;
	virtual bool canActivate() = 0;
protected:
	AnimationPlayerModule* getModule();
private:
	AnimationPlayerModule* mModule;
};


template <typename ControllerType>
class AnimationPlayerInstallerTool : public AnimationPlayerTool {
private:
	using super = AnimationPlayerTool;
public:
	AnimationPlayerInstallerTool(AnimationView* view, AnimationPlayerModule* module) : super(module), mView(view) {};

	virtual void activate() {
		getModule()->setPlayController(getController());
	}

	virtual bool canActivate() {
		return true;
	}
protected:
	virtual void tradeController(ControllerType* newController) {
		if (getModule()->getPlayController() == getController()) {
			getModule()->setPlayController(getController());
		}
		delete getController();
		setController(newController);
	}
	virtual void setController(ControllerType* controller) = 0;
	virtual ControllerType* getController() = 0;

	AnimationView* getAnimView() {
		return mView;
	}
private:
	AnimationView* mView;
};


class MusicAnimationPlayerTool : public AnimationPlayerInstallerTool<MusicPlayAnimationController> {
private:
	using super = AnimationPlayerInstallerTool;
public:
	MusicAnimationPlayerTool(AnimationView* view, AnimationPlayerModule* module);
	~MusicAnimationPlayerTool();

	void load(AnimationFrame* frame, std::string filename, BeatInfo* beats);

	virtual bool canActivate();
protected:
	virtual MusicPlayAnimationController* getController();
	virtual void setController(MusicPlayAnimationController* controller);
private:
	MusicPlayAnimationController* mPlayer;
};


class TempoAnimationPlayerTool : public AnimationPlayerInstallerTool<TempoPlayAnimationController> {
private:
	using super = AnimationPlayerInstallerTool;
public:
	TempoAnimationPlayerTool(AnimationView* view, TempoData* tempoSource, MeasureData* measuresSource, AnimationPlayerModule* module);
	~TempoAnimationPlayerTool();

protected:
	virtual TempoPlayAnimationController* getController();
	virtual void setController(TempoPlayAnimationController* controller);
private:
	TempoPlayAnimationController* mPlayer;
};

class ConstantSpeedAnimationPlayerTool : public AnimationPlayerInstallerTool<ConstantSpeedPlayAnimationController> {
private:
	using super = AnimationPlayerInstallerTool;
public:
	ConstantSpeedAnimationPlayerTool(AnimationView* view, AnimationPlayerModule* module);
	~ConstantSpeedAnimationPlayerTool();

	void updateTempo(int bpm);
protected:
	virtual ConstantSpeedPlayAnimationController* getController();
	virtual void setController(ConstantSpeedPlayAnimationController* controller);
private:
	ConstantSpeedPlayAnimationController* mPlayer;
};

#endif