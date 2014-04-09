#include "animate_player_tool.h"

AnimationPlayerTool::AnimationPlayerTool(AnimationPlayerModule* module)
: mModule(module)
{}

AnimationPlayerModule* AnimationPlayerTool::getModule() {
	return mModule;
}

MusicAnimationPlayerTool::MusicAnimationPlayerTool(AnimationView* view, AnimationPlayerModule* module)
: super(view, module), mPlayer(nullptr)
{}

MusicAnimationPlayerTool::~MusicAnimationPlayerTool() {
	tradeController(nullptr);
}

void MusicAnimationPlayerTool::load(AnimationFrame* frame, std::string filename, BeatInfo* beats) {
	mPlayer = new MusicPlayAnimationController(getAnimView(), frame, beats, filename);
}

bool MusicAnimationPlayerTool::canActivate() {
	return mPlayer != nullptr;
}

MusicPlayAnimationController* MusicAnimationPlayerTool::getController() {
	return mPlayer;
}

void MusicAnimationPlayerTool::setController(MusicPlayAnimationController* controller) {
	mPlayer = controller;
}

TempoAnimationPlayerTool::TempoAnimationPlayerTool(AnimationView* view, TempoData* tempoSource, MeasureData* measuresSource, AnimationPlayerModule* module)
	: super(view, module), mPlayer(new TempoPlayAnimationController(view, tempoSource, measuresSource))
{}

TempoAnimationPlayerTool::~TempoAnimationPlayerTool() {
	tradeController(nullptr);
}


TempoPlayAnimationController* TempoAnimationPlayerTool::getController() {
	return mPlayer;
}

void TempoAnimationPlayerTool::setController(TempoPlayAnimationController* controller) {
	mPlayer = controller;
}

ConstantSpeedAnimationPlayerTool::ConstantSpeedAnimationPlayerTool(AnimationView* view, AnimationPlayerModule* module)
: super(view, module), mPlayer(new ConstantSpeedPlayAnimationController(view, 60))
{}

ConstantSpeedAnimationPlayerTool::~ConstantSpeedAnimationPlayerTool() {
	tradeController(nullptr);
}

void ConstantSpeedAnimationPlayerTool::updateTempo(int bpm) {
	mPlayer->setBPM(bpm);
}

ConstantSpeedPlayAnimationController* ConstantSpeedAnimationPlayerTool::getController() {
	return mPlayer;
}

void ConstantSpeedAnimationPlayerTool::setController(ConstantSpeedPlayAnimationController* controller) {
	mPlayer = controller;
}