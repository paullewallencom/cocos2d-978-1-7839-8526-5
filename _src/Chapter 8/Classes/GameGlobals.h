#ifndef GAME_GLOBALS_H_
#define GAME_GLOBALS_H_

// Global includes go here
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include "Box2D\Box2D.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "Box2D/Box2D.h"
#endif

using namespace cocos2d;
using namespace std;
// Global includes end here

// Helper macros
#define SCREEN_SIZE GameGlobals::screen_size_
#define SOUND_ENGINE CocosDenshion::SimpleAudioEngine::sharedEngine()

#define PTM_RATIO 32
#define SCREEN_TO_WORLD(value) (float)(value)/PTM_RATIO
#define WORLD_TO_SCREEN(value) (float)(value)*PTM_RATIO
//#define ENABLE_DEBUG_DRAW			// uncomment this to enable debug drawing using GLESDebugDraw

#define MINIMUM_CATAPULT_ANGLE 2.0f // minimum joint angle for a valid shot

#define SIMPLE_FISH_RADIUS SCREEN_TO_WORLD(18)		//(15)
#define SHOOTING_FISH_RADIUS SCREEN_TO_WORLD(16)	//(13)
#define SPLITTING_FISH_RADIUS SCREEN_TO_WORLD(23)	//(20)
#define BOMBING_FISH_RADIUS SCREEN_TO_WORLD(18)		//(15)
#define EXPLODING_FISH_RADIUS SCREEN_TO_WORLD(21)	//(18)
#define TINY_FISH_RADIUS SCREEN_TO_WORLD(15)		//(12)

#define MAX_SPLIT_FISHS 5

// enum used for proper z-ordering
enum EZorder
{
	E_LAYER_BACKGROUND = 0,
	E_LAYER_OBJECTS = 2,
	E_LAYER_CATS = 3,
	E_LAYER_FISH = 4,
	E_LAYER_HUD = 6,
	E_LAYER_POPUP = 8,
};

enum EGameObjectType
{
	E_GAME_OBJECT_NONE = 0,
	E_GAME_OBJECT_FISH,
	E_GAME_OBJECT_CAT,
	E_GAME_OBJECT_BLOCK,
};

enum EFishType
{
	E_FISH_NONE = 0,
	E_FISH_SIMPLE,
	E_FISH_SHOOTING,
	E_FISH_SPLITTING,
	E_FISH_BOMBING,
	E_FISH_EXPLODING,
};

struct RayCastData
{
	b2Body* body_;
	b2Vec2 point_;

	RayCastData() { body_ = NULL; point_ = b2Vec2_zero; }
};

// Helper class containing only static members
class GameGlobals
{
private:
	GameGlobals();
	~GameGlobals();

public:
	// initialise common global data here
	// called when application finishes launching
	static void Init();
	// load initial/all game data here
	static void LoadData();

	static CCSize screen_size_;
	static int level_number_;

	static b2Body* CreateBoxBody(b2World* world, float width, float height);
	static b2Body* CreateCircleBody(b2World* world, float radius);

	// function takes comma separated string & returns vector of values
	static vector<int> GetIntListFromString(string input);
};

#endif // GAME_GLOBALS_H_