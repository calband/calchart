#include "animation_player_module.h"

AnimationPlayerModule::AnimationPlayerModule()
: mCurrentPlayController(nullptr)
{}

PlayAnimationController* AnimationPlayerModule::getPlayController() {
	return mCurrentPlayController;
}

void AnimationPlayerModule::setPlayController(PlayAnimationController* controller) {
	if (mCurrentPlayController != nullptr) {
		mCurrentPlayController->pause();
	}
	mCurrentPlayController = controller;
	if (controller != nullptr) {
		controller->update();
	}
}