#include "GameGlobals.h"

CCSize GameGlobals::screen_size_ = CCSizeZero;

void GameGlobals::Init()
{
	screen_size_ = CCDirector::sharedDirector()->getWinSize();
	LoadData();
}

void GameGlobals::LoadData()
{
	// add Resources folder to search path. This is necessary when releasing for win32
	CCFileUtils::sharedFileUtils()->addSearchPath("Resources");

	// load sprite sheet/s
	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("cjtexset_01.plist");
	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("cjtexset_02.plist");

	// load sound effects & background music
	SOUND_ENGINE->preloadEffect("base_platform.wav");
	SOUND_ENGINE->preloadEffect("bottle_rocket.wav");
	SOUND_ENGINE->preloadEffect("collectible.wav");
	SOUND_ENGINE->preloadEffect("game_over.wav");
	SOUND_ENGINE->preloadEffect("platform.wav");
	
	// create and add platform animation
	CCAnimation* animation = CCAnimation::create();
	animation->setDelayPerUnit(0.05f);
	animation->addSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjump04.png"));
	animation->addSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjump03.png"));
	animation->addSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjump02.png"));
	animation->addSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjump01.png"));
	animation->addSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjump05.png"));
	animation->addSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjump01.png"));
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "platform_animation");
}

// function takes comma separated string & returns vector of values
vector<float> GameGlobals::GetFloatListFromString(string input)
{
	vector<float> result;
	result.clear();

	if(input == "")
		return result;

	stringstream ss(input);
	float i;
	while (ss >> i)
	{
		result.push_back(i);
		if (ss.peek() == ',')
			ss.ignore();
	}
	return result;
}

// function takes comma separated string & returns CCPoint
CCPoint GameGlobals::GetPointFromString(string input)
{
	CCPoint point = CCPointZero;
	if(input == "")
		return point;
	vector<float> list = GetFloatListFromString(input);
	point.x = list[0];
	point.y = list[1];
	return point;
}
