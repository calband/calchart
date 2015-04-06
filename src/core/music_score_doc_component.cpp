#include "music_score_doc_component.h"

MusicScoreDocComponent::MusicScoreDocComponent() 
: mStartMoment(std::shared_ptr<MusicScoreFragment>(nullptr), 0, 0)
{
	mBarLabels.reset(new MusicScoreBarLabelsCollection(MusicScoreBarLabel("No Label")));
	mJumps.reset(new MusicScoreJumpsCollection(MusicScoreJump(MusicScoreMoment(0, 0, 0))));
	mTimeSignatures.reset(new TimeSignaturesCollection(TimeSignature(4)));
	mTempos.reset(new MusicScoreTemposCollection(MusicScoreTempo(120)));
}

MusicScoreDocComponent::MusicScoreDocComponent(const MusicScoreDocComponent& other)
: MusicScoreDocComponent()
{
	copyContentFrom(other);
}

void MusicScoreDocComponent::copyContentFrom(const MusicScoreDocComponent& other) {
	mBarLabels->copyContentFrom(other.mBarLabels.get());
	mJumps->copyContentFrom(other.mJumps.get());
	mTimeSignatures->copyContentFrom(other.mTimeSignatures.get());
	mTempos->copyContentFrom(other.mTempos.get());
	mFragments = other.mFragments;
	mStartMoment = other.mStartMoment;
}

MusicScoreJumpsCollection* MusicScoreDocComponent::getScoreJumps() {
	return mJumps.get();
}

MusicScoreBarLabelsCollection* MusicScoreDocComponent::getBarLabels() {
	return mBarLabels.get();
}

TimeSignaturesCollection* MusicScoreDocComponent::getTimeSignatures() {
	return mTimeSignatures.get();
}

MusicScoreTemposCollection* MusicScoreDocComponent::getTempos() {
	return mTempos.get();
}

const MusicScoreJumpsCollection* MusicScoreDocComponent::getScoreJumps() const {
	return mJumps.get();
}

const MusicScoreBarLabelsCollection* MusicScoreDocComponent::getBarLabels() const {
	return mBarLabels.get();
}

const TimeSignaturesCollection* MusicScoreDocComponent::getTimeSignatures() const {
	return mTimeSignatures.get();
}

const MusicScoreTemposCollection* MusicScoreDocComponent::getTempos() const {
	return mTempos.get();
}

void MusicScoreDocComponent::addScoreFragment(std::shared_ptr<MusicScoreFragment> fragment) {
	if (scoreFragmentIsRegistered(fragment.get())) {
		return;
	}
	mFragments.push_back(fragment);
}

void MusicScoreDocComponent::removeScoreFragment(int fragmentIndex, bool removeAssociatedEvents) {
	if (fragmentIndex < 0 || fragmentIndex >= getNumScoreFragments()) {
		return;
	}
	MusicScoreFragment* fragment = mFragments[fragmentIndex].get();
	if (removeAssociatedEvents) {
		mBarLabels->clearEvents(fragment);
		mJumps->clearEvents(fragment);
		mTimeSignatures->clearEvents(fragment);
		mTempos->clearEvents(fragment);
	}
	mFragments.erase(mFragments.begin() + fragmentIndex);
	if (mStartMoment.fragment.get() == fragment) {
		setStartFragmentToNullFragment();
	}
}

bool MusicScoreDocComponent::scoreFragmentIsRegistered(const MusicScoreFragment* fragment) const {
	for (unsigned index = 0; index < getNumScoreFragments(); index++) {
		if (mFragments[index].get() == fragment) {
			return true;
		}
	}
	return false;
}

unsigned MusicScoreDocComponent::getNumScoreFragments() const {
	return mFragments.size();
}

std::shared_ptr<MusicScoreFragment> MusicScoreDocComponent::getScoreFragment(int fragmentIndex) {
	if (fragmentIndex < 0 || fragmentIndex >= getNumScoreFragments()) {
		return nullptr;
	}
	return mFragments[fragmentIndex];
}
std::shared_ptr<const MusicScoreFragment> MusicScoreDocComponent::getScoreFragment(int fragmentIndex) const {
	if (fragmentIndex < 0 || fragmentIndex >= getNumScoreFragments()) {
		return nullptr;
	}
	return mFragments[fragmentIndex];
}

int MusicScoreDocComponent::getIndexOfFragment(const MusicScoreFragment* fragment) const {
	for (unsigned index = 0; index < getNumScoreFragments(); index++) {
		if (getScoreFragment(index).get() == fragment) {
			return index;
		}
	}
	return -1;
}

MusicScoreMoment MusicScoreDocComponent::getStartMoment() const {
	return mStartMoment;
}

void MusicScoreDocComponent::setStartFragmentToNullFragment() {
	mStartMoment.fragment.reset();
}

void MusicScoreDocComponent::setStartFragmentToFragmentAtIndex(int fragmentIndex) {
	mStartMoment.fragment = getScoreFragment(fragmentIndex);
}