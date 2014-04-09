#ifndef _ANIMATION_PLAYER_MODULE_H_
#define _ANIMATION_PLAYER_MODULE_H_

#include "play_animation_controller.h"

class AnimationPlayerModule {
public:
	AnimationPlayerModule();

	PlayAnimationController* getPlayController();
	void setPlayController(PlayAnimationController* controller);
private:
	PlayAnimationController* mCurrentPlayController;
};

#endif