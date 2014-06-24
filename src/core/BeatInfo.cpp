#include "BeatInfo.h"
    

void BeatInfo::addBeat(long time)
{
    mBeatsList.push_back(time);
}
	

void BeatInfo::removeBeat(long time)
{
	int i = 0;
	while (i < mBeatsList.size())
    {
        if (mBeatsList[i] == time)
        {
            mBeatsList.erase(mBeatsList.begin() + i);
        }
        ++i;
    }
}
    

void BeatInfo::removeBeatByIndex(int beatNum)
{
    mBeatsList.erase(mBeatsList.begin() + beatNum);
}
    

void BeatInfo::clearBeats()
{
    mBeatsList.clear();
}
    
	
int BeatInfo::getCurrentBeat(long time) const
{
    int i = 0;
    while (i < mBeatsList.size())
    {
        if (time < mBeatsList[i])
        {
            break;
        }
		++i;
    }
    return i - 1;
}
    

long BeatInfo::getBeat(int beatNum) const
{
	if (mBeatsList.size() > beatNum) {
		return mBeatsList[beatNum];
	} else {
		return -1;
	}
}
   

int BeatInfo::getNumBeats() const
{
     return mBeatsList.size();
}
