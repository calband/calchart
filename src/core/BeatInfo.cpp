/*
 The name of the class should be BeatInfo.
 This class keeps track of the times at which every beat occurs in a music file.
 Every beat has a "time" and an "beatNum":
 The "beatNum" is the beat number, essentially. The show begins with the zeroth beat, and every beat afterward increments. The first bar of the show, then, would start at beat 0, and would go through the start of beat 4. The start of beat 4 through the start of beat 8 is bar 2, etc. (this is, of course, assuming 4 beats per bar, which may not be the case)
 The "time" is the time, in milliseconds, at which the beat occurs since the start of the music. A beat that begins at 1 second after the start of the music would thus have a time of 1000 (for 1000 milliseconds).
 */
#include "BeatInfo.h"
    

void BeatInfo::addBeat(long time)
{
    ++beatCount;
    beatsList.push_back(time);
}
	

void BeatInfo::removeBeat(long time)
{
	int i = 0;
    while (i < beatsList.size())
    {
        if (beatsList[i] == time)
        {
            --beatCount;
            beatsList.erase(beatsList.begin() + i);
        }
        ++i;
    }
}
    

void BeatInfo::removeBeatByIndex(int beatNum)
{
    --beatCount;
    beatsList.erase(beatsList.begin() + beatNum);
}
    

void BeatInfo::clearBeats()
{
    beatCount = 0;
    beatsList.clear();
}
    
	
int BeatInfo::getCurrentBeat(long time)
{
    int i = 0;
    while (i < beatsList.size())
    {
        if (beatsList[i] == time)
        {
            return i;
        }
        if (beatsList[i] < time && beatsList[i+1] > time)
        {
            return i;
        }
        ++i;
    }
    return 0;// check this.
}
    

long BeatInfo::getBeat(int beatNum)
{
    return beatsList[beatNum];
}
    
int BeatInfo::getNumBeats()
{
     return beatsList.size();
}
