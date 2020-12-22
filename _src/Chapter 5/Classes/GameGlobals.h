#ifndef GAME_GLOBALS_H_
#define GAME_GLOBALS_H_

#include "cocos2d.h"
#include "Box2D\Box2D.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

using namespace std;

#define SCREEN_SIZE GameGlobals::screen_size_
#define SOUND_ENGINE CocosDenshion::SimpleAudioEngine::sharedEngine()

#define PTM_RATIO 128
#define SCREEN_TO_WORLD(value) (float)(value)/PTM_RATIO
#define WORLD_TO_SCREEN(value) (float)(value)*PTM_RATIO

#define WALL_WIDTH 64
#define MAX_PLATFORM_WIDTH 600
#define PLATFORM_IMPULSE 20
#define PLATFORM_ANIMATION "platform_animation"
#define MAX_COLLECTIBLES 25
#define ROCKET_DURATION 3
#define BALLOON_DURATION 5

// uncomment this to enable debug drawing using GLESDebugDraw
//#define ENABLE_DEBUG_DRAW

// enum used for proper z-ordering
enum EZorder
{
	E_LAYER_BACKGROUND = 0,
	E_LAYER_FOREGROUND = 2,
	E_LAYER_COLLECTIBLES = 4,
	E_LAYER_CLOWN = 6,
	E_LAYER_PLATFORM = 8,
	E_LAYER_HUD = 10,
	E_LAYER_POPUP = 12,
};

enum EGameObjectType
{
	E_GAME_OBJECT_NONE = 0,
	E_GAME_OBJECT_CLOWN,
	E_GAME_OBJECT_PLATFORM,
	E_GAME_OBJECT_COLLECTIBLE,
	E_GAME_OBJECT_ROCKET,
	E_GAME_OBJECT_BALLOON,
};

enum EClownState
{
	E_CLOWN_NONE = 0,
	E_CLOWN_UP,
	E_CLOWN_DOWN,
	E_CLOWN_BOUNCE,
	E_CLOWN_ROCKET,
	E_CLOWN_BALLOON,
};

class GameGlobals
{
public:
	GameGlobals(void){};
	~GameGlobals(void){};

	// initialise common global data here...called when application finishes launching
	static void Init();
	// load initial/all game data here
	static void LoadData();

	// save screen size for fast access
	static CCSize screen_size_;

	// function takes comma separated string & returns vector of values
	static vector<float> GetFloatListFromString(string input);
	// function takes comma separated string & returns CCPoint
	static CCPoint GetPointFromString(string input);
};

#endif // GAME_GLOBALS_H_
