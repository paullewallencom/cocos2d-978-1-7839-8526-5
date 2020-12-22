#include "GameGlobals.h"

CCSize GameGlobals::screen_size_ = CCSizeZero;

void GameGlobals::Init()
{
	srand(time(0));
	screen_size_ = CCDirector::sharedDirector()->getWinSize();
	LoadData();
}

void GameGlobals::LoadData()
{
	// add Resources folder to search path. This is necessary when releasing for win32
	CCFileUtils::sharedFileUtils()->addSearchPath("Resources");
}

ccColor4F GameGlobals::GetRandomColor()
{
	return ccc4f(0.3f + CCRANDOM_0_1() * 0.7f, 0.3f + CCRANDOM_0_1() * 0.7f, 0.3f + CCRANDOM_0_1() * 0.7f, 1.0f);
}