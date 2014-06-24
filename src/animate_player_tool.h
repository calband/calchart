#ifndef _ANIMATE_PLAYER_TOOL_H_
#define _ANIMATE_PLAYER_TOOL_H_

#include "animation_frame.h"
#include "animation_view.h"
#include "play_animation_controller.h"
#include "animation_player_module.h"

/**
 * Represents a tool that can be selected from the Animation Frame and that interacts with the AnimationPlayerModule.
 */
class AnimationPlayerTool {
public:
	/**
	 * Makes the tool.
	 * @param module The AnimationPlayerModule that the tool will interact with.
	 */
	AnimationPlayerTool(AnimationPlayerModule* module);

	/**
	 * Activates the tool.
	 */
	virtual void activate() = 0;
	/**
	 * Returns whether or not the tool can be activated.
	 * @return True if the tool can be activated; false otherwise.
	 */
	virtual bool canActivate() = 0;
protected:
	/**
	 * Returns the AnimationPlayerModule that this tool interacts with.
	 * @return The AnimationPlayerModule that this tool interacts with.
	 */
	AnimationPlayerModule* getModule();
private:
	/**
	 * The AnimationPlayerModule that this tool interacts with.
	 */
	AnimationPlayerModule* mModule;
};


/**
 * A tool that installs a PlayAnimationController into the AnimationPlayerModule.
 */
template <typename ControllerType>
class AnimationPlayerInstallerTool : public AnimationPlayerTool {
private:
	using super = AnimationPlayerTool;
public:
	/**
	 * Makes the tool.
	 * @param view The AnimationView whose animation will be controlled by the AnimationPlayerModule.
	 * @param module The AnimationPlayerModule will install the PlayAnimationController to.
	 */
	AnimationPlayerInstallerTool(AnimationView* view, AnimationPlayerModule* module) : super(module), mView(view) {};

	virtual void activate() {
		getModule()->setPlayController(getController());
	}

	virtual bool canActivate() {
		return true;
	}
protected:
	/**
	 * Associates this tool with a new PlayAnimationController and deletes the old one.
	 * @param newController The new PlayAnimationController to use in place of the old one.
	 */
	virtual void tradeController(ControllerType* newController) {
		if (getModule()->getPlayController() == getController()) {
			getModule()->setPlayController(getController());
		}
		delete getController();
		setController(newController);
	}
	/**
	 * Sets the PlayAnimationController associated with this tool.
	 * @param controller The new PlayAnimationController to associate with this tool.
	 */
	virtual void setController(ControllerType* controller) = 0;
	/**
	 * Returns the PlayAnimationController that is currently associated with this tool.
	 * @return The PlayAnimationController that is currently associated with this tool.
	 */
	virtual ControllerType* getController() = 0;

	/**
	 * Returns the AnimationView linked to this tool.
	 * @return The AnimationView linked to this tool.
	 */
	AnimationView* getAnimView() {
		return mView;
	}
private:
	/**
	 * The AnimationView linked to this tool.
	 */
	AnimationView* mView;
};

/**
 * A tool used to activate the PlayAnimationController that plays the Animation with music.
 */
class MusicAnimationPlayerTool : public AnimationPlayerInstallerTool<MusicPlayAnimationController> {
private:
	using super = AnimationPlayerInstallerTool;
public:
	/**
	 * Constructor.
	 * @param view The view whose Animation will be played.
	 * @param module The module to install the animation controller to.
	 */
	MusicAnimationPlayerTool(AnimationView* view, AnimationPlayerModule* module);
	/**
	 * Destructor.
	 */
	~MusicAnimationPlayerTool();

	/**
	 * Loads a new Animation Controller for playing the show behind music.
	 * @param frame The AnimationFrame to link the music player to. The AnimationFrame will be the music player's parent frame.
	 * @param filename The path for the music file.
	 * @param beats A collection specifying where the beats fall against the music.
	 * @param responsibleForBeats If true, this tool will make sure that it deletes the passed BeatInfo object when it destoys the Animation Controller. Otherwise, the BeatInfo will not be deleted.
	 */
	void load(AnimationFrame* frame, std::string filename, BeatInfo* beats, bool responsibleForBeats = true);

	virtual bool canActivate();
protected:
	virtual void tradeController(MusicPlayAnimationController* newController);

	virtual MusicPlayAnimationController* getController();
	virtual void setController(MusicPlayAnimationController* controller);
private:
	/**
	 * The current animation controller.
	 */
	MusicPlayAnimationController* mPlayer;
	/**
	 * The BeatInfo object associated with the player (if known).
	 * This must be remembered, since the player does not delete it on its own.
	 */
	BeatInfo* mInfoToDelete;
};

/**
 * A tool used to activate the PlayAnimationController that will play the Animation according to the tempos of the music.
 */
class TempoAnimationPlayerTool : public AnimationPlayerInstallerTool<TempoPlayAnimationController> {
private:
	using super = AnimationPlayerInstallerTool;
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be played.
	 * @param tempoSource The tempos for the animation.
	 * @param measuresSource The MeasureData for the music, which specifies where the bars of the music fall.
	 * @param module The module to install the PlayAnimationController to.
	 */
	TempoAnimationPlayerTool(AnimationView* view, TempoData* tempoSource, MeasureData* measuresSource, AnimationPlayerModule* module);
	 /**
	  * Destructor.
	  */
	~TempoAnimationPlayerTool();

protected:
	virtual TempoPlayAnimationController* getController();
	virtual void setController(TempoPlayAnimationController* controller);
private:
	/**
	 * The current animation controller.
	 */
	TempoPlayAnimationController* mPlayer;
};

/**
 * A tool used to activate the PlayAnimationController that plays the Animation at a constant speed.
 */
class ConstantSpeedAnimationPlayerTool : public AnimationPlayerInstallerTool<ConstantSpeedPlayAnimationController> {
private:
	using super = AnimationPlayerInstallerTool;
public:
	/**
	 * Constructor.
	 * @param view The AnimationView whose Animation will be played.
	 * @param module The module to install the PlayAnimationController to.
	 */
	ConstantSpeedAnimationPlayerTool(AnimationView* view, AnimationPlayerModule* module);
	/**
	 * Destructor.
	 */
	~ConstantSpeedAnimationPlayerTool();

	/**
	 * Changes the tempo at which the Animation will play.
	 * @param bpm The new tempo at which the Animation will play, in beats per minute.
	 */
	void updateTempo(int bpm);
protected:
	virtual ConstantSpeedPlayAnimationController* getController();
	virtual void setController(ConstantSpeedPlayAnimationController* controller);
private:
	/**
	 * The current animation controller.
	 */
	ConstantSpeedPlayAnimationController* mPlayer;
};

#endif