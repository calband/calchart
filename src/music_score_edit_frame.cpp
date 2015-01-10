#include "music_score_edit_frame.h"
#include "ui_enums.h"
#include "cc_command.h"

#include <random>

#include <wx\gbsizer.h>

BEGIN_EVENT_TABLE(MusicScoreEditFrame, wxFrame)
EVT_BUTTON(CALCHART__MusicScore_Save, MusicScoreEditFrame::onSave)
EVT_BUTTON(CALCHART__MusicScore_Close, MusicScoreEditFrame::onClose)
EVT_BUTTON(CALCHART__MusicScore_AddFragment, MusicScoreEditFrame::onAddFragment)
EVT_MENU(CALCHART__MusicScore_PopupMenuRenameFragment, MusicScoreEditFrame::onPopupRenameFragment)
EVT_MENU(CALCHART__MusicScore_PopupMenuDeleteFragment, MusicScoreEditFrame::onPopupDeleteFragment)
EVT_MENU(CALCHART__MusicScore_PopupMenuDeleteFragmentProperty, MusicScoreEditFrame::onPopupDeleteFragmentProperty)
EVT_LIST_ITEM_RIGHT_CLICK(CALCHART__MusicScore_FragmentList, MusicScoreEditFrame::onFragmentRightClick)
EVT_LIST_ITEM_SELECTED(CALCHART__MusicScore_FragmentList, MusicScoreEditFrame::onFragmentSelected)
EVT_LIST_ITEM_DESELECTED(CALCHART__MusicScore_FragmentList, MusicScoreEditFrame::onFragmentDeselected)
EVT_LIST_END_LABEL_EDIT(CALCHART__MusicScore_FragmentList, MusicScoreEditFrame::onFragmentRenamed)
EVT_PG_CHANGED(CALCHART__MusicScore_FragmentEditor, MusicScoreEditFrame::onFragmentPropertyChanged)
EVT_PG_CHANGING(CALCHART__MusicScore_FragmentEditor, MusicScoreEditFrame::onFragmentPropertyChanging)
EVT_PG_RIGHT_CLICK(CALCHART__MusicScore_FragmentEditor, MusicScoreEditFrame::onFragmentPropertyRightClicked)
END_EVENT_TABLE()


template <typename EventType>
class FragmentEditor__MusicEventsCategoryHandler : public FragmentEditor__CategoryHandler {
public:
	/**
	 * Gives the name that will be given to the category handled by this handler.
	 * @return The name of the category for this handler.
	 */
	virtual std::string getCategoryName() = 0;

	virtual void loadPropertyFromEvent(MusicScoreDocComponent* musicScore, wxPGProperty* prop, MusicScoreMoment time, EventType evt) = 0;
	virtual void savePropertyToScore(MusicScoreDocComponent* musicScore, MusicScoreMoment eventTime, EventType evt) = 0;
	virtual CollectionOfMusicScoreEvents<EventType>* getEventCollection(MusicScoreDocComponent* musicScore) = 0;
	virtual wxPGProperty* insertEmptyProperty(MusicScoreDocComponent* musicScore, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) = 0;
	virtual std::pair<MusicScoreMoment, EventType> buildEventFromProperty(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop) = 0;

	/**
	 * Makes properties from the events in the MusicScoreDocComponent and
	 * adds them to the category.
	 * @param musicScore The doc component to load from.
	 * @param fragment The fragment that we are loading events from.
	 * @param propertyGrid The grid of all fragment properties.
	 * @param category The category to add the events to.
	 */
	virtual void loadEventsIntoCategory(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		auto eventCollection = getEventCollection(musicScore);
		for (unsigned index = 0; index < eventCollection->getNumEvents(fragment.get()); index++) {
			wxPGProperty* newProperty = insertEmptyProperty(musicScore, propertyGrid, category);
			auto fullEvent = eventCollection->getEvent(fragment.get(), index);
			loadPropertyFromEvent(musicScore, newProperty, fullEvent.first, fullEvent.second);
			newProperty->GetPropertyByName("Index")->SetValue((long)index);
		}
	}

	virtual void shiftPropertyIndices(wxPropertyCategory* category, unsigned startIndex, int shift) {
		for (unsigned index = 0; index < category->GetChildCount(); index++) {
			wxPGProperty* indexProperty = category->Item(index)->GetPropertyByName("Index");
			if (indexProperty->GetValue().GetLong() >= startIndex) {
				indexProperty->SetValue(indexProperty->GetValue().GetLong() + shift);
			}
		}
	}
	
	unsigned getIndexOfEvent(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, MusicScoreMoment time, EventType evt) {
		auto eventCollection = getEventCollection(musicScore);
		for (unsigned index = 0; index < eventCollection->getNumEvents(fragment.get()); index++) {
			auto fullEvent = eventCollection->getEvent(fragment.get(), index);
			if (fullEvent.first == time && fullEvent.second == evt) {
				return index;
			}
		}
		return eventCollection->getNumEvents(fragment.get());
	}

	virtual void savePropertyToScore(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPropertyCategory* category, wxPGProperty* prop) {
		if (propertyIsEmpty(prop)) {
			return;
		}
		auto propertyEvent = buildEventFromProperty(musicScore, fragment, prop);
		savePropertyToScore(musicScore, propertyEvent.first, propertyEvent.second);
		unsigned addIndex = getIndexOfEvent(musicScore, fragment, propertyEvent.first, propertyEvent.second);
		prop->GetPropertyByName("Index")->SetValue((long)addIndex);
		shiftPropertyIndices(category, addIndex + 1, 1);
	}

	virtual void deletePropertyFromScore(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPropertyCategory* category, wxPGProperty* prop) {
		long index = getIndexFromProperty(prop);
		if (index < 0) {
			return;
		}
		getEventCollection(musicScore)->removeEvent(fragment.get(), index);
		shiftPropertyIndices(category, index, -1);
	}

	virtual void appendIndexToProperty(wxPGProperty* prop) {
		wxPGProperty* indexProp = new wxIntProperty("Index", wxPG_LABEL, -1);
		prop->AppendChild(indexProp);
		indexProp->Hide(true);
	}

	virtual long getIndexFromProperty(wxPGProperty* prop) {
		return prop->GetPropertyByName("Index")->GetValue().GetLong();
	}

	virtual std::string generateUniqueName(wxPropertyGrid* grid) {
		std::string name;
		do {
			name += std::to_string(rand() % 10);
		} while (grid->GetPropertyByName(name));
		return name;
	}

	virtual wxPGProperty* getBaseProperty(wxPropertyCategory* category, wxPGProperty* prop) {
		wxPGProperty* base = prop;
		while (base->GetParent() != category) {
			base = base->GetParent();
		}
		return base;
	}

	virtual wxPropertyCategory* loadGridFromMusicScore(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPropertyGrid* propertyGrid) {
		wxPropertyCategory* newCategory = new wxPropertyCategory(wxString(getCategoryName()), wxPG_LABEL);
		propertyGrid->Append(newCategory);
		loadEventsIntoCategory(musicScore, fragment, propertyGrid, newCategory);
		return newCategory;
	}

	virtual void onPropertyModified(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		prop = getBaseProperty(category, prop);
		if (!propertyIsEmpty(prop)) {
			deletePropertyFromScore(musicScore, fragment, category, prop);
			savePropertyToScore(musicScore, fragment, category, prop);
		}
	}

	virtual bool propertyIsEmpty(wxPGProperty* prop) {
		if (!prop->IsCategory() && prop->IsValueUnspecified()) {
			return true;
		}
		for (unsigned index = 0; index < prop->GetChildCount(); index++) {
			if (propertyIsEmpty(prop->Item(index))) {
				return true;
			}
		}
		return false;
	}

	virtual void onPropertyDeleted(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		prop = getBaseProperty(category, prop);
		if (!propertyIsEmpty(prop)) {
			deletePropertyFromScore(musicScore, fragment, category, prop);
		}
	}

	virtual void onPropertyAdded(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		prop = getBaseProperty(category, prop);
		if (!propertyIsEmpty(prop)) {
			savePropertyToScore(musicScore, fragment, category, prop);
		}
	}
};


class FragmentEditor__TimeSigCategoryHandler : public FragmentEditor__MusicEventsCategoryHandler<TimeSignature> {
public:
	virtual std::string getCategoryName() {
		return "Time Signatures";
	}

	virtual void loadPropertyFromEvent(MusicScoreDocComponent* musicScore, wxPGProperty* prop, MusicScoreMoment time, TimeSignature evt) {
		prop->GetPropertyByName("Bar")->SetValue(time.beatAndBar.bar);
		prop->GetPropertyByName("Beats Per Bar")->SetValue(evt.beatsPerBar);
	}

	virtual void savePropertyToScore(MusicScoreDocComponent* musicScore, MusicScoreMoment eventTime, TimeSignature evt) {
		musicScore->getTimeSignatures()->addTimeSignatureChange(eventTime.fragment, eventTime.beatAndBar.bar, evt);
	}

	virtual CollectionOfMusicScoreEvents<TimeSignature>* getEventCollection(MusicScoreDocComponent* musicScore) {
		return musicScore->getTimeSignatures();
	}

	virtual std::pair<MusicScoreMoment, TimeSignature> buildEventFromProperty(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop) {
		unsigned bar = prop->GetPropertyByName("Bar")->GetValue().GetLong();
		unsigned beatsPerBar = prop->GetPropertyByName("Beats Per Bar")->GetValue().GetLong();
		return std::pair<MusicScoreMoment, TimeSignature>(MusicScoreMoment(fragment, bar, 0), TimeSignature(beatsPerBar));
	}

	virtual wxPGProperty* insertEmptyProperty(MusicScoreDocComponent* musicScore, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		wxPGProperty* newProperty = new wxStringProperty("Time Signature", generateUniqueName(propertyGrid));
		newProperty->Enable(false);
		propertyGrid->AppendIn(category, newProperty);
		wxPGProperty* childProp;
		childProp = new wxUIntProperty("Bar");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxUIntProperty("Beats Per Bar");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		appendIndexToProperty(newProperty);
		return newProperty;
	}
};

class FragmentEditor__JumpsCategoryHandler : public FragmentEditor__MusicEventsCategoryHandler<MusicScoreJump> {
public:
	virtual std::string getCategoryName() {
		return "Score Jumps";
	}

	virtual long getIndexOfFragment(MusicScoreDocComponent* musicScore, std::shared_ptr<const MusicScoreFragment> fragment) {
		for (unsigned checkIndex = 0; checkIndex < musicScore->getNumScoreFragments(); checkIndex++) {
			if (musicScore->getScoreFragment(checkIndex) == fragment) {
				return checkIndex;
			}
		}
		return -1;
	}

	virtual void loadPropertyFromEvent(MusicScoreDocComponent* musicScore, wxPGProperty* prop, MusicScoreMoment time, MusicScoreJump evt) {
		prop->GetPropertyByName("Start Bar")->SetValue(time.beatAndBar.bar);
		prop->GetPropertyByName("Start Beat")->SetValue(time.beatAndBar.beat);
		prop->GetPropertyByName("Destination Bar")->SetValue(evt.jumpTo.beatAndBar.bar);
		prop->GetPropertyByName("Destination Beat")->SetValue(evt.jumpTo.beatAndBar.beat);
		prop->GetPropertyByName("Destination Fragment")->SetValue(getIndexOfFragment(musicScore, evt.jumpTo.fragment));
	}

	virtual void savePropertyToScore(MusicScoreDocComponent* musicScore, MusicScoreMoment eventTime, MusicScoreJump evt) {
		musicScore->getScoreJumps()->addJump(eventTime, evt);
	}

	virtual CollectionOfMusicScoreEvents<MusicScoreJump>* getEventCollection(MusicScoreDocComponent* musicScore) {
		return musicScore->getScoreJumps();
	}

	virtual std::pair<MusicScoreMoment, MusicScoreJump> buildEventFromProperty(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop) {
		unsigned bar = prop->GetPropertyByName("Start Bar")->GetValue().GetLong();
		unsigned beat = prop->GetPropertyByName("Start Beat")->GetValue().GetLong();
		unsigned destBar = prop->GetPropertyByName("Destination Bar")->GetValue().GetLong();
		unsigned destBeat = prop->GetPropertyByName("Destination Beat")->GetValue().GetLong();
		long destFragIndex = prop->GetPropertyByName("Destination Fragment")->GetValue().GetLong();
		std::shared_ptr<MusicScoreFragment> destFrag;
		if (destFragIndex >= 0) {
			destFrag = musicScore->getScoreFragment(destFragIndex);
		}
		return std::pair<MusicScoreMoment, MusicScoreJump>(MusicScoreMoment(fragment, bar, beat), MusicScoreJump(MusicScoreMoment(destFrag, destBar, destBeat)));
	}

	virtual wxPGProperty* insertEmptyProperty(MusicScoreDocComponent* musicScore, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		wxPGProperty* newProperty = new wxStringProperty("Jump", generateUniqueName(propertyGrid));
		newProperty->Enable(false);
		propertyGrid->AppendIn(category, newProperty);
		wxPGProperty* childProp;
		childProp = new wxUIntProperty("Start Bar");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxUIntProperty("Start Beat");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxUIntProperty("Destination Bar");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxUIntProperty("Destination Beat");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxEnumProperty("Destination Fragment");
		newProperty->AppendChild(childProp);
		for (unsigned index = 0; index < musicScore->getNumScoreFragments(); index++) {
			childProp->AddChoice(musicScore->getScoreFragment(index)->name, index);
		}
		childProp->AddChoice("NO FRAGMENT", -1);
		childProp->SetValueToUnspecified();
		appendIndexToProperty(newProperty);
		return newProperty;
	}
};

class FragmentEditor__TempoCategoryHandler : public FragmentEditor__MusicEventsCategoryHandler<MusicScoreTempo> {
public:
	virtual std::string getCategoryName() {
		return "Tempos";
	}

	virtual void loadPropertyFromEvent(MusicScoreDocComponent* musicScore, wxPGProperty* prop, MusicScoreMoment time, MusicScoreTempo evt) {
		prop->GetPropertyByName("Bar")->SetValue(time.beatAndBar.bar);
		prop->GetPropertyByName("Beat")->SetValue(time.beatAndBar.beat);
		prop->GetPropertyByName("Beats Per Minute")->SetValue(evt.beatsPerMinute);
	}

	virtual void savePropertyToScore(MusicScoreDocComponent* musicScore, MusicScoreMoment eventTime, MusicScoreTempo evt) {
		musicScore->getTempos()->addTempoChange(eventTime, evt);
	}

	virtual CollectionOfMusicScoreEvents<MusicScoreTempo>* getEventCollection(MusicScoreDocComponent* musicScore) {
		return musicScore->getTempos();
	}

	virtual std::pair<MusicScoreMoment, MusicScoreTempo> buildEventFromProperty(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop) {
		unsigned bar = prop->GetPropertyByName("Bar")->GetValue().GetLong();
		unsigned beat = prop->GetPropertyByName("Beat")->GetValue().GetLong();
		unsigned bpm = prop->GetPropertyByName("Beats Per Minute")->GetValue().GetLong();
		return std::pair<MusicScoreMoment, MusicScoreTempo>(MusicScoreMoment(fragment, bar, beat), MusicScoreTempo(bpm));
	}

	virtual wxPGProperty* insertEmptyProperty(MusicScoreDocComponent* musicScore, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		wxPGProperty* newProperty = new wxStringProperty("Tempo", generateUniqueName(propertyGrid));
		newProperty->Enable(false);
		propertyGrid->AppendIn(category, newProperty);
		wxPGProperty* childProp;
		childProp = new wxUIntProperty("Bar");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxUIntProperty("Beat");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxUIntProperty("Beats Per Minute");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		appendIndexToProperty(newProperty);
		return newProperty;
	}
};

class FragmentEditor__BarLabelCategoryHandler : public FragmentEditor__MusicEventsCategoryHandler<MusicScoreBarLabel> {
public:
	virtual std::string getCategoryName() {
		return "Bar Labels";
	}

	virtual void loadPropertyFromEvent(MusicScoreDocComponent* musicScore, wxPGProperty* prop, MusicScoreMoment time, MusicScoreBarLabel evt) {
		prop->GetPropertyByName("Bar")->SetValue(time.beatAndBar.bar);
		prop->GetPropertyByName("Bar Label")->SetValue(evt.label);
	}

	virtual void savePropertyToScore(MusicScoreDocComponent* musicScore, MusicScoreMoment eventTime, MusicScoreBarLabel evt) {
		musicScore->getBarLabels()->addBarLabel(eventTime.fragment, eventTime.beatAndBar.bar, evt);
	}

	virtual CollectionOfMusicScoreEvents<MusicScoreBarLabel>* getEventCollection(MusicScoreDocComponent* musicScore) {
		return musicScore->getBarLabels();
	}

	virtual std::pair<MusicScoreMoment, MusicScoreBarLabel> buildEventFromProperty(MusicScoreDocComponent* musicScore, std::shared_ptr<MusicScoreFragment> fragment, wxPGProperty* prop) {
		unsigned bar = prop->GetPropertyByName("Bar")->GetValue().GetLong();
		std::string label = prop->GetPropertyByName("Label")->GetValue().GetString();
		return std::pair<MusicScoreMoment, MusicScoreBarLabel>(MusicScoreMoment(fragment, bar, 0), MusicScoreBarLabel(label));
	}

	virtual wxPGProperty* insertEmptyProperty(MusicScoreDocComponent* musicScore, wxPropertyGrid* propertyGrid, wxPropertyCategory* category) {
		wxPGProperty* newProperty = new wxStringProperty("Bar Label", generateUniqueName(propertyGrid));
		newProperty->Enable(false);
		propertyGrid->AppendIn(category, newProperty);
		wxPGProperty* childProp;
		childProp = new wxUIntProperty("Bar");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		childProp = new wxStringProperty("Label");
		childProp->SetValueToUnspecified();
		newProperty->AppendChild(childProp);
		appendIndexToProperty(newProperty);
		return newProperty;
	}
};

int fragmentPropertyComparator(wxPropertyGrid* grid, wxPGProperty* first, wxPGProperty* second) {
	if (first->GetPropertyByName("Index") == nullptr) {
		if (second->GetPropertyByName("Index") == nullptr) {
			return 1;
		} else {
			return 1;
		}
	} else {
		if (second->GetPropertyByName("Index") == nullptr) {
			return -1;
		}
	}
	return first->GetPropertyByName("Index")->GetValue().GetLong() - second->GetPropertyByName("Index")->GetValue().GetLong();
}

MusicScoreEditFrame::MusicScoreEditFrame(CalChartDoc& doc, wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
: mDoc(doc), mLocalMusicScore(*mDoc.getMusicScore()), mFragmentEditor(nullptr), mModified(false)
{
	mFragCategoryHandlers.push_back(std::unique_ptr<FragmentEditor__CategoryHandler>(new FragmentEditor__TimeSigCategoryHandler()));
	mFragCategoryHandlers.push_back(std::unique_ptr<FragmentEditor__CategoryHandler>(new FragmentEditor__JumpsCategoryHandler()));
	mFragCategoryHandlers.push_back(std::unique_ptr<FragmentEditor__CategoryHandler>(new FragmentEditor__TempoCategoryHandler()));
	mFragCategoryHandlers.push_back(std::unique_ptr<FragmentEditor__CategoryHandler>(new FragmentEditor__BarLabelCategoryHandler()));
	Create(parent, id, caption, pos, size, style);
}

void MusicScoreEditFrame::onClose(wxCommandEvent& event) {
	if (mModified) {
		int choice = wxMessageBox(wxT("Changes were made.  Save changes before exiting?"), wxT("Save changes?"), wxYES_NO | wxCANCEL);
		if (choice == wxYES) {
			saveToMusicScoreDocComponent();
		}
		if (choice == wxCANCEL) {
			return;
		}
	}
	Close();
}

void MusicScoreEditFrame::onSave(wxCommandEvent& evt) {
	saveToMusicScoreDocComponent();
}

void MusicScoreEditFrame::onAddFragment(wxCommandEvent& evt) {
	mFragmentList->InsertItem(mFragmentList->GetItemCount(), "New Fragment");
	mLocalMusicScore.addScoreFragment(std::shared_ptr<MusicScoreFragment>(new MusicScoreFragment("New Fragment")));
	resetFragmentEditWindow();
	mModified = true;
}

void MusicScoreEditFrame::onPopupRenameFragment(wxCommandEvent& evt) {
	long *index = (long*)(((wxMenu*)evt.GetEventObject())->GetClientData());
	mFragmentList->EditLabel(*index);
	delete index;
}


void MusicScoreEditFrame::onPopupDeleteFragment(wxCommandEvent& evt) {
	long *index = (long*)(((wxMenu*)evt.GetEventObject())->GetClientData());
	mFragmentList->DeleteItem(*index);
	mLocalMusicScore.removeScoreFragment(*index);
	delete index;
	resetFragmentEditWindow();
	mModified = true;
}

void MusicScoreEditFrame::onPopupDeleteFragmentProperty(wxCommandEvent& evt) {
	wxPGProperty* prop = (wxPGProperty*)(((wxMenu*)evt.GetEventObject())->GetClientData());
	wxPGProperty* category = prop;
	wxPGProperty* base = category;
	while (!category->IsCategory() || mFragCategoryToHandlerMap.find((wxPropertyCategory*)category) == mFragCategoryToHandlerMap.end()) {
		base = category;
		category = category->GetParent();
	}
	auto handler = mFragCategoryToHandlerMap[(wxPropertyCategory*)category];
	handler->onPropertyDeleted(&mLocalMusicScore, mSelectedFragment, prop, mFragmentEditor, (wxPropertyCategory*)category);
	bool replaceWithEmpty = (handler->propertyIsEmpty(base));
	mFragmentEditor->DeleteProperty(prop);
	if (replaceWithEmpty) {
		handler->insertEmptyProperty(&mLocalMusicScore, mFragmentEditor, (wxPropertyCategory*)category);
	}
	mFragmentEditor->SortChildren(category);
	mModified = true;
}

void MusicScoreEditFrame::onFragmentRightClick(wxListEvent& evt) {
	makeFragmentRightClickMenu(evt.GetPoint(), evt.GetIndex());
}

void MusicScoreEditFrame::onFragmentSelected(wxListEvent& evt) {
	mSelectedFragment = mLocalMusicScore.getScoreFragment(evt.GetIndex());
	resetFragmentEditWindow();
}

void MusicScoreEditFrame::onFragmentDeselected(wxListEvent& evt) {
	deselectFragmentIfSelected(mLocalMusicScore.getScoreFragment(evt.GetIndex()));
	resetFragmentEditWindow();
}

void MusicScoreEditFrame::onFragmentRenamed(wxListEvent& evt) {
	mLocalMusicScore.getScoreFragment(evt.GetIndex())->name = evt.GetLabel();
	resetFragmentEditWindow();
}

void MusicScoreEditFrame::onFragmentPropertyChanged(wxPropertyGridEvent& evt) {
	wxPGProperty* prop = evt.GetProperty();
	wxPGProperty* potentialCategory = prop;
	while (!potentialCategory->IsCategory()) {
		potentialCategory = potentialCategory->GetParent();
	}
	wxPropertyCategory* category = (wxPropertyCategory*)potentialCategory;
	FragmentEditor__CategoryHandler& handler = *mFragCategoryToHandlerMap.at(category);
	handler.onPropertyModified(&mLocalMusicScore, mSelectedFragment, prop, mFragmentEditor, category);
	refreshEditorCategory(category);
	mModified = true;
}

void MusicScoreEditFrame::onFragmentPropertyChanging(wxPropertyGridEvent& evt) {
}

void MusicScoreEditFrame::onFragmentPropertyRightClicked(wxPropertyGridEvent& evt) {
	if (evt.GetProperty()->IsCategory() && mFragCategoryToHandlerMap.find((wxPropertyCategory*)evt.GetProperty()) != mFragCategoryToHandlerMap.end()) {
		return;
	}
	makeFragmentPropertyRightClickMenu(mFragmentEditor->GetPosition(), evt.GetProperty());
}

bool MusicScoreEditFrame::Create(wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
	if (!super::Create(parent, id, caption, pos, size, style))
		return false;
	createControls();
	loadFrameContentFromMusicScore();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Center();
	Update();
	return true;
}

void MusicScoreEditFrame::createControls() {
	wxGridBagSizer *mainSizer = new wxGridBagSizer(5, 10);
	SetSizer(mainSizer);

	wxBoxSizer *fragmentListText = new wxBoxSizer(wxVERTICAL);
	fragmentListText->AddSpacer(10);
	fragmentListText->Add(new wxStaticText(this, wxID_ANY, wxT("Select a Music Score Fragment:")));
	mainSizer->Add(fragmentListText, wxGBPosition(0, 0), wxGBSpan(1, 1));

	mFragmentList = new wxListCtrl(this, CALCHART__MusicScore_FragmentList, wxDefaultPosition, wxSize(300, 300), wxLC_LIST | wxLC_EDIT_LABELS | wxLC_SINGLE_SEL);
	mFragmentList->SetColumnWidth(0, 300);
	mainSizer->Add(mFragmentList, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);

	wxBoxSizer *mainButtons = new wxBoxSizer(wxHORIZONTAL);
	mainButtons->Add(new wxButton(this, CALCHART__MusicScore_Close, wxT("Close")));
	mainButtons->Add(new wxButton(this, CALCHART__MusicScore_Save, wxT("Save")));
	mainButtons->Add(new wxButton(this, CALCHART__MusicScore_AddFragment, wxT("Add Fragment")));

	mainSizer->Add(mainButtons, wxGBPosition(2, 0), wxGBSpan(1, 1));

	mFragmentEditor = new wxPropertyGrid(this, CALCHART__MusicScore_FragmentEditor, wxDefaultPosition, wxSize(300, 350));
	mFragmentEditor->SetSortFunction(&fragmentPropertyComparator);
	mainSizer->Add(mFragmentEditor, wxGBPosition(1, 1), wxGBSpan(2, 1), wxEXPAND);

	mainSizer->AddGrowableRow(1, 1);
	mainSizer->AddGrowableRow(2, 1);
	mainSizer->AddGrowableCol(1, 1);
}

void MusicScoreEditFrame::makeFragmentRightClickMenu(wxPoint& position, long index) {
	wxMenu popupMenu;
	popupMenu.SetClientData(new long(index));
	popupMenu.Append(CALCHART__MusicScore_PopupMenuRenameFragment, "Rename");
	popupMenu.Append(CALCHART__MusicScore_PopupMenuDeleteFragment, "Delete");
	PopupMenu(&popupMenu, position);
}

void MusicScoreEditFrame::makeFragmentPropertyRightClickMenu(wxPoint& position, wxPGProperty* clickedProp) {
	wxMenu popupMenu;
	popupMenu.SetClientData(clickedProp);
	popupMenu.Append(CALCHART__MusicScore_PopupMenuDeleteFragmentProperty, "Delete");
	PopupMenu(&popupMenu, position);
}

void MusicScoreEditFrame::resetFragmentEditWindow() {
	mFragmentEditor->Clear();
	if (mSelectedFragment.get() == nullptr) {
		return;
	}
	mFragCategoryToHandlerMap.clear();
	for (unsigned index = 0; index < mFragCategoryHandlers.size(); index++) {
		FragmentEditor__CategoryHandler& handler = *mFragCategoryHandlers[index].get();
		wxPropertyCategory* newCategory = handler.loadGridFromMusicScore(&mLocalMusicScore, mSelectedFragment, mFragmentEditor);
		mFragCategoryToHandlerMap.emplace(newCategory, &handler);
		refreshEditorCategory(newCategory);
		mFragmentEditor->Collapse(newCategory);
	}
}

void MusicScoreEditFrame::deselectFragmentIfSelected(std::shared_ptr<MusicScoreFragment> fragment) {
	if (mSelectedFragment == fragment) {
		mSelectedFragment.reset();
		resetFragmentEditWindow();
	}
}

void MusicScoreEditFrame::saveToMusicScoreDocComponent() {
	mDoc.GetCommandProcessor()->Submit(new OverwriteMusicScoreCommand(mDoc, mLocalMusicScore));
	mModified = false;
}


void MusicScoreEditFrame::refreshEditorCategory(wxPropertyCategory* category) {
	FragmentEditor__CategoryHandler& handler = *mFragCategoryToHandlerMap[category];
	bool foundEmpty = false;
	for (int index = category->GetChildCount() - 1; index >= 0; index--) {
		wxPGProperty* currProperty = category->Item(index);
		if (handler.propertyIsEmpty(currProperty)) {
			if (foundEmpty) {
				handler.onPropertyDeleted(&mLocalMusicScore, mSelectedFragment, currProperty, mFragmentEditor, category);
				mFragmentEditor->DeleteProperty(currProperty);
			} else {
				foundEmpty = true;
			}
		}
	}
	if (!foundEmpty) {
		wxPGProperty* newEmpty = handler.insertEmptyProperty(&mLocalMusicScore, mFragmentEditor, category);
		handler.onPropertyAdded(&mLocalMusicScore, mSelectedFragment, newEmpty, mFragmentEditor, category);
	}
	mFragmentEditor->SortChildren(category);
}

void MusicScoreEditFrame::loadFrameContentFromMusicScore() {
	for (unsigned index = 0; index < mLocalMusicScore.getNumScoreFragments(); index++) {
		mFragmentList->InsertItem(index, mLocalMusicScore.getScoreFragment(index)->name);
	}
	resetFragmentEditWindow();
}