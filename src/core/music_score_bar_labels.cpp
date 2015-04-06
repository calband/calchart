#include "music_score_bar_labels.h"

MusicScoreBarLabelsCollection::MusicScoreBarLabelsCollection(MusicScoreBarLabel defaultEvent) 
: super(defaultEvent)
{}

void MusicScoreBarLabelsCollection::addBarLabel(std::shared_ptr<const MusicScoreFragment> fragment, BarNumber bar, MusicScoreBarLabel label) {
	addEvent(MusicScoreMoment(fragment, bar, 0), label);
}

std::string MusicScoreBarLabelsCollection::getBarName(MusicScoreMoment time) {
	std::pair<MusicScoreMoment, MusicScoreBarLabel> labelEvent = getMostRecentEvent(time);
	if (time.beatAndBar.bar == labelEvent.first.beatAndBar.bar) {
		return labelEvent.second.label;
	}
	return std::to_string(time.beatAndBar.bar);
}