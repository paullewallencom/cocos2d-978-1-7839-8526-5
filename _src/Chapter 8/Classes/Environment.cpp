#include "Environment.h"

Environment::Environment()
{}

Environment::~Environment()
{}

Environment* Environment::create()
{
	Environment* environment = new Environment();
	if(environment && environment->init())
	{
		environment->autorelease();
		return environment;
	}
	CC_SAFE_DELETE(environment);
	return NULL;
}

bool Environment::init()
{
	if(!CCNode::init())
		return false;

	CreateSky();
	CreateHills();
	CreatePier();
	CreateBoat();

	return true;
}

void Environment::CreateSky()
{
	// create the blue gradient for the sky that is half the size of the screen
	CCLayerGradient* sky = CCLayerGradient::create(ccc4(135, 219, 255, 255), ccc4(158, 245, 255, 255));
	sky->setContentSize(CCSizeMake(SCREEN_SIZE.width, SCREEN_SIZE.height * 0.5f));
	sky->setPosition(ccp(0, SCREEN_SIZE.height * 0.5f));
	addChild(sky);

	// create clouds
	CreateCloud(120.0f, "CH_04.png");
	CreateCloud(130.0f, "CH_04.png");
	CreateCloud(120.0f, "CH_03.png");
	CreateCloud(130.0f, "CH_03.png");
	CreateCloud(100.0f, "CH_02.png");
	CreateCloud(95.0f, "CH_01.png");

	// create the blue gradient for the water that is half the size of the screen
	CCLayerGradient* water = CCLayerGradient::create(ccc4(63, 159, 183, 255), ccc4(104, 198, 184, 255));
	water->setContentSize(CCSizeMake(SCREEN_SIZE.width, SCREEN_SIZE.height * 0.5f));
	addChild(water);
}

void Environment::CreateCloud(float duration, const char* frame_name)
{
	// randomly position the clouds in the upper half of the screen
	CCPoint position = ccp(SCREEN_SIZE.width * CCRANDOM_0_1(), 450.0f + SCREEN_SIZE.height * 0.45f * CCRANDOM_0_1());

	CCSprite* cloud = CCSprite::create(frame_name);
	cloud->setPosition(position);
	addChild(cloud);

	// duration 1 -> move from starting point to left edge of screen
	float duration1 = (position.x / (SCREEN_SIZE.width + cloud->getContentSize().width)) * duration;
	// duration 2 -> move from right edge of screen to starting point
	float duration2 = duration - duration1;

	// animate the cloud's movement -> start point-TO-left edge-TO-right edge-TO-start point
	CCMoveTo* move_left1 = CCMoveTo::create(duration1, ccp(-cloud->getContentSize().width, position.y));
	CCPlace* place_right = CCPlace::create(ccp(SCREEN_SIZE.width + cloud->getContentSize().width, position.y));
	CCMoveTo* move_left2 = CCMoveTo::create(duration2, position);
	CCSequence* cloud_movement = CCSequence::create(move_left1, place_right, move_left2, NULL);
	
	// repeat forever
	cloud->runAction(CCRepeatForever::create(cloud_movement));
}

void Environment::CreateHills()
{
	ccTexParams tex_params;
	// setup the hill texture to repeat
	tex_params.minFilter = GL_NEAREST;
	tex_params.magFilter = GL_NEAREST;
	tex_params.wrapS = GL_REPEAT;
	tex_params.wrapT = GL_REPEAT;

	CCSprite* hills = CCSprite::create("bg_03.png");
	hills->getTexture()->setTexParameters(&tex_params);
	hills->setTextureRect(CCRectMake(0, 0, SCREEN_SIZE.width, hills->getContentSize().height));
	
	hills->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.53f));
	addChild(hills);
}

void Environment::CreatePier()
{
	// create two pillars for the pier
	CCSprite* pier_base_pillar = NULL;
	pier_base_pillar = CCSprite::create("platform_banboo2.png");
	pier_base_pillar->setPosition(ccp(780.0f, 0.0f));
	addChild(pier_base_pillar);

	pier_base_pillar = CCSprite::create("platform_banboo2.png");
	pier_base_pillar->setPosition(ccp(1100.0f, 0.0f));
	addChild(pier_base_pillar);

	// create a batch node for the pier floor
	CCSpriteBatchNode* pier_base_batch = CCSpriteBatchNode::create("platform_base.png", 10);
	addChild(pier_base_batch);

	CCPoint offset = ccp(SCREEN_SIZE.width, SCREEN_SIZE.height * 0.2f);
	for(int i = 0; i < 10; ++i)
	{
		CCSprite* pier_base = CCSprite::create("platform_base.png");
		pier_base->setPosition(ccp(offset.x - (i * 65), offset.y));
		pier_base_batch->addChild(pier_base);
	}
}

void Environment::CreateBoat()
{
	// the launch boat
	CCSprite* boat = CCSprite::create("boat_base.png");
	boat->setPosition(ccp(25.0f, 100.0f));
	addChild(boat);
}
