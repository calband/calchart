#include "animation_player_module.h"

AnimationPlayerModule::AnimationPlayerModule()
: mCurrentPlayController(nullptr)
{}

PlayAnimationController* AnimationPlayerModule::getPlayController() {
	return mCurrentPlayController;
}

void AnimationPlayerModule::setPlayController(PlayAnimationController* controller) {
	mCurrentPlayController = controller;
}