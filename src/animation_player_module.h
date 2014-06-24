#ifndef _ANIMATION_PLAYER_MODULE_H_
#define _ANIMATION_PLAYER_MODULE_H_

#include "play_animation_controller.h"

/**
 * Keeps track of PlayAnimationController that will be used whenever the "play" button is pressed in the Animation Frame.
 */
class AnimationPlayerModule {
public:
	/**
	 * Constructor.
	 */
	AnimationPlayerModule();

	/**
	 * Returns the current PlayAnimationController.
	 * @return The current PlayAnimationController.
	 */
	PlayAnimationController* getPlayController();
	/**
	 * Sets the current PlayAnimationController.
	 * @param controller The new PlayAnimationController.
	 */
	void setPlayController(PlayAnimationController* controller);
private:
	/**
	 * The current PlayAnimationController.
	 */
	PlayAnimationController* mCurrentPlayController;
};

#endif