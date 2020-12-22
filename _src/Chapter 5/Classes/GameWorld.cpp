#include "GameWorld.h"
#include "GLES-Render.h"
#include "BackgroundManager.h"
#include "Clown.h"
#include "Collectible.h"
#include "Popups.h"

GameWorld::GameWorld()
{
	sprite_batch_node_ = NULL;
	world_ = NULL;
	game_object_layer_ = NULL;
	background_manager_ = NULL;
	clown_ = NULL;
	wall_ = NULL;
	contact_normal_ = b2Vec2_zero;
	platform_ = NULL;
	base_platform_ = NULL;
	pool_collectibles_ = NULL;
	active_collectibles_ = NULL;
	num_collectibles_active_ = 0;
	has_game_begun_ = false;
	touch_start_ = CCPointZero;
	touch_end_ = CCPointZero;
	score_label_ = NULL;
	score_ = 0;
	is_popup_active_ = false;
	distance_travelled_ = 0.0f;
#ifdef ENABLE_DEBUG_DRAW
	debug_draw_ = NULL;
#endif
}

GameWorld::~GameWorld()
{}

CCScene* GameWorld::scene()
{
    CCScene *scene = CCScene::create();
    GameWorld *layer = GameWorld::create();
    scene->addChild(layer);
    return scene;
}

bool GameWorld::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }
    
	CreateWorld();
	CreateGame();
    return true;
}

void GameWorld::CreateWorld()
{
	// create world
	b2Vec2 gravity;
	gravity.Set(0, -10.0f);
	world_ = new b2World(gravity);
	// tell world we want to listen for collisions
	world_->SetContactListener(this);

	// create the moving container that will hold all the game elements
	game_object_layer_ = CCNode::create();
	addChild(game_object_layer_, E_LAYER_FOREGROUND);

#ifdef ENABLE_DEBUG_DRAW
	debug_draw_ = new GLESDebugDraw(PTM_RATIO);
	world_->SetDebugDraw(debug_draw_);
	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;
//	  flags += b2Draw::e_jointBit;
//    flags += b2Draw::e_aabbBit;
//    flags += b2Draw::e_pairBit;
//    flags += b2Draw::e_centerOfMassBit;
	debug_draw_->SetFlags(flags);
#endif
}

void GameWorld::DestroyWorld()
{
#ifdef ENABLE_DEBUG_DRAW
	CC_SAFE_DELETE(debug_draw_);
	debug_draw_ = NULL;
#endif
	CC_SAFE_DELETE(world_);
	world_ = NULL;
	CC_SAFE_RELEASE_NULL(pool_collectibles_);
	CC_SAFE_RELEASE_NULL(active_collectibles_);
}

void GameWorld::CreateGame()
{
	// create the environment
	background_manager_ = BackgroundManager::create();
	background_manager_->game_world_ = this;
	addChild(background_manager_, E_LAYER_BACKGROUND);

	// create & add the batch node
	sprite_batch_node_ = CCSpriteBatchNode::create("cjtexset_01.png", 128);
	game_object_layer_->addChild(sprite_batch_node_);
	
	CreateWall();
	CreateClown();
	CreateBasePlatform();
	CreatePlatform();
	CreateCollectibles();
	CreateHUD();

	// enable touch & accelerometer
	setTouchEnabled(true);
	setAccelerometerEnabled(true);

	// everything created, start updating
	scheduleUpdate();
}

void GameWorld::CreateWall()
{
	// wall will be a static body placed at the origin
	b2BodyDef wall_def;
	wall_def.position.Set(0, 0);
	wall_ = world_->CreateBody(&wall_def);

	// get variables for the wall edges
	float left_wall = SCREEN_TO_WORLD(WALL_WIDTH);
	float right_wall = SCREEN_TO_WORLD(SCREEN_SIZE.width - WALL_WIDTH);
	float top = SCREEN_TO_WORLD(SCREEN_SIZE.height);
	
	// create and add two fixtures using two edge shapes
	b2EdgeShape wall_shape;
	wall_shape.Set(b2Vec2(left_wall, 0), b2Vec2(left_wall, top));
	wall_->CreateFixture(&wall_shape, 0);
	wall_shape.Set(b2Vec2(right_wall, 0), b2Vec2(right_wall, top));
	wall_->CreateFixture(&wall_shape, 0);
}

void GameWorld::CreateClown()
{
	// clown will be a dynamic body
	b2BodyDef clown_def;
	clown_def.type = b2_dynamicBody;
	// clown will start off at the centre of the screen
	clown_def.position.Set(SCREEN_TO_WORLD(SCREEN_SIZE.width/2), SCREEN_TO_WORLD(SCREEN_SIZE.height/2.75));
	b2Body* clown_body = world_->CreateBody(&clown_def);

	// create clown, set physics body & add to batch node
	clown_ = Clown::create(this);
	clown_->SetBody(clown_body);
	sprite_batch_node_->addChild(clown_, E_LAYER_CLOWN);
}

void GameWorld::CreateBasePlatform()
{
	// base platform will be a static body
	b2BodyDef platform_def;
	platform_def.position.Set(SCREEN_TO_WORLD(416), SCREEN_TO_WORLD(172));
	b2Body* base_platform_body = world_->CreateBody(&platform_def);

	// create an edge slightly above the bottom of the screen
	b2EdgeShape base_platform_shape;
	base_platform_shape.Set(b2Vec2(SCREEN_TO_WORLD(-SCREEN_SIZE.width), 0.45f), b2Vec2(SCREEN_TO_WORLD(SCREEN_SIZE.width), 0.45f));
	b2FixtureDef base_platform_fixture_def;
	base_platform_fixture_def.shape = &base_platform_shape;
	// give the base platform perfectly elastic collision response
	base_platform_fixture_def.restitution = 1.0f;
	base_platform_body->CreateFixture(&base_platform_fixture_def);

	// create base platform, set physics body & add to batch node
	base_platform_ = GameObject::create(this, "cjtrapm01.png");
	base_platform_->SetBody(base_platform_body);
	base_platform_->SetType(E_GAME_OBJECT_PLATFORM);
	sprite_batch_node_->addChild(base_platform_, E_LAYER_CLOWN - 1);
}

void GameWorld::CreatePlatform()
{
	// platform will be a static body
	b2BodyDef platform_def;
	platform_def.position.Set(0, 0);
	b2Body* platform_body = world_->CreateBody(&platform_def);

	// create platform, set physics body & add to batch node
	platform_ = GameObject::create(this, "cjump01.png");
	platform_->setAnchorPoint(ccp(0, 0.25f));
	platform_->setVisible(false);
	platform_->SetBody(platform_body);
	platform_->SetType(E_GAME_OBJECT_PLATFORM);
	sprite_batch_node_->addChild(platform_, E_LAYER_PLATFORM);
}

void GameWorld::CreateCollectibles()
{
	// create the pool and active containers
	pool_collectibles_ = CCArray::createWithCapacity(MAX_COLLECTIBLES);
	pool_collectibles_->retain();
	active_collectibles_ = CCArray::createWithCapacity(MAX_COLLECTIBLES);
	active_collectibles_->retain();

	// all collectibles will be static bodies
	b2BodyDef body_def;
	body_def.type = b2_staticBody;
	body_def.position.Set(SCREEN_TO_WORLD(-1 * WALL_WIDTH), SCREEN_TO_WORLD(-1 * WALL_WIDTH));

	for(int i = 0; i < MAX_COLLECTIBLES; ++i)
	{
		// ensure there is one balloon and one rocket
		EGameObjectType type = (i == 1) ? E_GAME_OBJECT_BALLOON : ( (i == 0) ? E_GAME_OBJECT_ROCKET : E_GAME_OBJECT_COLLECTIBLE );
		// create collectible, set physics body & add to the pool
		Collectible* collectible = Collectible::create(this, type);
		collectible->SetBody(world_->CreateBody(&body_def));
		pool_collectibles_->addObject(collectible);
	}
}

void GameWorld::CreateHUD()
{
	// create & add score label
	char buf[8] = {0};
	sprintf(buf, "Score:%d", score_);
	score_label_ = CCLabelBMFont::create(buf, "jcfont.fnt");
	score_label_->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.925f));
	addChild(score_label_, E_LAYER_HUD);

	// create & add the pause menu containing pause button
	CCMenuItemSprite* pause_button = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("cjbpause.png"), CCSprite::createWithSpriteFrameName("cjbpause.png"), this, menu_selector(GameWorld::OnPauseClicked));
	pause_button->setPosition(ccp(SCREEN_SIZE.width*0.9f, SCREEN_SIZE.height*0.925f));
	CCMenu* menu = CCMenu::create(pause_button, NULL);
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu, E_LAYER_HUD);
}

#ifdef ENABLE_DEBUG_DRAW
void GameWorld::draw()
{
	CCLayer::draw();
	ccGLEnableVertexAttribs( kCCVertexAttribFlag_Position );
	kmGLPushMatrix();
	world_->DrawDebugData();
	kmGLPopMatrix();
}
#endif

void GameWorld::update(float dt)
{
	// update the world
	world_->Step(dt, 8, 3);

	// update all game objects
	clown_->Update();
	if(base_platform_)
	{
		base_platform_->Update();
	}
	
	for(int i = 0; i < num_collectibles_active_; ++i)
	{
		((Collectible*)active_collectibles_->objectAtIndex(i))->Update();
	}

	// update platform if user is dragging one
	if(platform_->isVisible() && !touch_start_.equals(CCPointZero))
	{
		platform_->setPosition(game_object_layer_->convertToNodeSpace(touch_start_));
	}

	// walls must move along with clown
	wall_->SetTransform(b2Vec2(0, clown_->GetBody()->GetPosition().y - SCREEN_TO_WORLD(SCREEN_SIZE.height/2)), 0);
	// background must scroll with respect to clown
	background_manager_->Update( has_game_begun_ ? ((clown_->GetBody()->GetLinearVelocity().y) * -1) : 0 );

	// game_object_layer_ must move in opposite direction of clown
	// subtract SCREEN_SIZE.height/2 so that clown always stays in centre of the screen
	float position_y = -1 * (clown_->getPositionY() - SCREEN_SIZE.height/2);
	game_object_layer_->setPositionY( position_y > 0 ? 0 : position_y );

	UpdateCounters();
}

void GameWorld::UpdateCounters()
{
	// check if clown has moved higher
	int new_distance_travelled = clown_->GetBody()->GetPosition().y - SCREEN_TO_WORLD(SCREEN_SIZE.height/2);
	if(new_distance_travelled > distance_travelled_)
	{
		// add score for every meter covered
		AddScore(5 * (new_distance_travelled - distance_travelled_));
		distance_travelled_ = new_distance_travelled;

		// add a collectible every 5 meters
		if(distance_travelled_ % 5 == 0)
		{
			// add a rocket or balloon every 100 meters
			AddCollectible(distance_travelled_ % 100 == 0);
		}
	}
}

void GameWorld::ccTouchesBegan(CCSet* set, CCEvent* event)
{
	// don't accept touch when clown is in these states
	if(clown_->GetState() == E_CLOWN_ROCKET ||
		clown_->GetState() == E_CLOWN_BALLOON ||
		clown_->GetState() == E_CLOWN_UP)
		return;

	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_start_ = CCDirector::sharedDirector()->convertToGL(touch_point);
	
	// remove any previously added platforms
	RemovePlatform();
	// convert touch coordinates with respect to game_object_layer_ and position the platform there
	platform_->setPosition(game_object_layer_->convertToNodeSpace(touch_start_));
	platform_->setVisible(true);
	platform_->setScaleX(0);
}

void GameWorld::ccTouchesMoved(CCSet* set, CCEvent* event)
{
	// don't accept touch when clown is in these states
	if(clown_->GetState() == E_CLOWN_ROCKET ||
		clown_->GetState() == E_CLOWN_BALLOON ||
		clown_->GetState() == E_CLOWN_UP)
		return;

	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_end_ = CCDirector::sharedDirector()->convertToGL(touch_point);

	// manipulate anchor point so the platform is correctly oriented
	platform_->setAnchorPoint( touch_end_.x >= touch_start_.x ? ccp(0, 0.5f) : ccp(1, 0.5f) );
	float length = ccpDistance(touch_end_, touch_start_);
	// scale the platform according to user input
	platform_->setScaleX(length / platform_->getContentSize().width);
	// manipulate rotation so that platform doesn't appear upside down
	float angle = CC_RADIANS_TO_DEGREES(-1 * ccpToAngle(ccpSub(touch_end_, touch_start_)));
	platform_->setRotation( touch_end_.x >= touch_start_.x ? angle : angle + 180 );
}

void GameWorld::ccTouchesEnded(CCSet* set, CCEvent* event)
{
	// don't accept touch when clown is in these states
	if(clown_->GetState() == E_CLOWN_ROCKET ||
		clown_->GetState() == E_CLOWN_BALLOON ||
		clown_->GetState() == E_CLOWN_UP)
		return;

	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_end_ = CCDirector::sharedDirector()->convertToGL(touch_point);
	
	platform_->setPosition(game_object_layer_->convertToNodeSpace(touch_start_));
	// user input has finished...time to add the platform's fixture
	AddPlatform(game_object_layer_->convertToNodeSpace(touch_start_), game_object_layer_->convertToNodeSpace(touch_end_));
	touch_start_ = touch_end_ = CCPointZero;
}

void GameWorld::didAccelerate(CCAcceleration* acceleration_value)
{
	// only accept tilt when clown is holding on to the balloon
	if(clown_->GetState() != E_CLOWN_BALLOON)
		return;

	// set linear velocity based on horizontal direction of tilt
	clown_->GetBody()->SetLinearVelocity(b2Vec2(acceleration_value->x * 10, 10.0f));
}

void GameWorld::BeginContact(b2Contact *contact)
{
	b2Body* body_a = contact->GetFixtureA()->GetBody();
	b2Body* body_b = contact->GetFixtureB()->GetBody();

	// only need to observe collisions that involve GameObjects
	if(body_a->GetUserData() == NULL || body_b->GetUserData() == NULL)
	{
		return;
	}

	// identify type of the objects involved in the collision
	EGameObjectType game_object_a_type = ((GameObject*)body_a->GetUserData())->GetType();
	EGameObjectType game_object_b_type = ((GameObject*)body_b->GetUserData())->GetType();

	// if a collision did not involve the clown, it will be ignored
	if(game_object_a_type != E_GAME_OBJECT_CLOWN && game_object_b_type != E_GAME_OBJECT_CLOWN)
		return;

	// separate the clown and the other object
	GameObject* other_object = (game_object_a_type != E_GAME_OBJECT_CLOWN) ? (GameObject*)body_a->GetUserData() : (GameObject*)body_b->GetUserData();

	// based on type, call appropriate collision response function
	switch(other_object->GetType())
	{
	case E_GAME_OBJECT_PLATFORM:
		if(other_object == base_platform_)
		{
			DoBasePlatformCollision();
		}
		else
		{
			OnCollision(contact->GetManifold()->localNormal);
		}		
		break;
	case E_GAME_OBJECT_COLLECTIBLE:
	case E_GAME_OBJECT_ROCKET:
	case E_GAME_OBJECT_BALLOON:
		((Collectible*)other_object)->OnCollision();
		break;
	}
}

void GameWorld::AddPlatform(CCPoint start, CCPoint end)
{
	// ensure the platform has only one edge shaped fixture
	if(platform_->GetBody()->GetFixtureList())
		return;

	// create and add a new fixture based on the user input
	b2EdgeShape platform_shape;
	platform_shape.Set(b2Vec2(SCREEN_TO_WORLD(start.x), SCREEN_TO_WORLD(start.y)), b2Vec2(SCREEN_TO_WORLD(end.x), SCREEN_TO_WORLD(end.y)));
	platform_->GetBody()->CreateFixture(&platform_shape, 0);
}

void GameWorld::RemovePlatform()
{
	// remove the existing fixture
	if(platform_->GetBody()->GetFixtureList())
	{
		platform_->GetBody()->DestroyFixture(platform_->GetBody()->GetFixtureList());
	}
}

void GameWorld::RemoveBasePlatform()
{
	// destroy the body and remove the GameObject
	// this function is called from BackgroundManager
	world_->DestroyBody(base_platform_->GetBody());
	base_platform_->removeFromParentAndCleanup(true);
	base_platform_ = NULL;
}

void GameWorld::AddCollectible(bool special)
{
	// do not exceed the maximum
	if(num_collectibles_active_ >= MAX_COLLECTIBLES)
		return;

	// loop through the pool of collectibles
	Collectible* collectible = NULL;
	int num_pool_collectibles = pool_collectibles_->count();
	for(int i = 0; i < num_pool_collectibles; ++i)
	{
		// if a special collectible is required, return one if available
		collectible = (Collectible*)pool_collectibles_->objectAtIndex(i);
		if(special && (collectible->GetType() == E_GAME_OBJECT_ROCKET || collectible->GetType() == E_GAME_OBJECT_BALLOON))
			break;
		else if(!special && collectible->GetType() != E_GAME_OBJECT_ROCKET && collectible->GetType() != E_GAME_OBJECT_BALLOON)
			break;
	}

	// add the collectible to the batch node
	sprite_batch_node_->addChild(collectible, E_LAYER_COLLECTIBLES);

	// remove the collectible from the pool and add it to the active list
	pool_collectibles_->removeObject(collectible);
	active_collectibles_->addObject(collectible);
	++ num_collectibles_active_;

	// position the collectible & then initialise it
	b2Vec2 position;
	position.x = SCREEN_TO_WORLD(WALL_WIDTH * 1.5f) + CCRANDOM_0_1() * SCREEN_TO_WORLD(SCREEN_SIZE.width - WALL_WIDTH * 3);
	position.y = distance_travelled_ + SCREEN_TO_WORLD(SCREEN_SIZE.height * 2);
	collectible->Init(position);
}

void GameWorld::RemoveCollectible(Collectible* collectible)
{
	if(num_collectibles_active_ <= 0)
		return;

	-- num_collectibles_active_;
	// remove the collectible from the active list and add it back to the pool
	active_collectibles_->removeObject(collectible);
	pool_collectibles_->addObject(collectible);
	// reset the collectible so it is ready for reuse
	collectible->Reset();
}

void GameWorld::AddScore(int value)
{
	score_ += value;
	char buf[16] = {0};
	sprintf(buf, "Score:%d", score_);
	score_label_->setString(buf);
}

void GameWorld::OnCollision(b2Vec2 contact_normal)
{
	// ignore collisions when the clown is in these states
	switch(clown_->GetState())
	{
	case E_CLOWN_UP:
	case E_CLOWN_ROCKET:
	case E_CLOWN_BALLOON:
		return;
	}

	// stop the clown's movement
	clown_->GetBody()->SetLinearVelocity(b2Vec2_zero);
	// update the clown's state
	clown_->SetState(E_CLOWN_BOUNCE);
	// save the normal at the point of contact
	contact_normal_ = contact_normal;
	// animate the platform
	platform_->runAction(CCSequence::createWithTwoActions(CCAnimate::create(CCAnimationCache::sharedAnimationCache()->animationByName("platform_animation")), CCHide::create()));
	// schedule the actual collision response after a short duration
	scheduleOnce(schedule_selector(GameWorld::DoCollisionResponse), 0.15f);

	has_game_begun_ = true;

	SOUND_ENGINE->playEffect("platform.wav");
}

void GameWorld::DoCollisionResponse(float dt)
{
	// safe checking if the platform's fixture is deleted before this function is called
	if(!platform_->GetBody()->GetFixtureList())
		return;

	// fetch the shape from the platform body's fixture
	b2EdgeShape* platform_shape = (b2EdgeShape*)platform_->GetBody()->GetFixtureList()->GetShape();
	// get the difference vector
	b2Vec2 diff = platform_shape->m_vertex2 - platform_shape->m_vertex1;

	// calculate magnitude of the impulse based on the length of the platform
	float distance = WORLD_TO_SCREEN(diff.Length()) / MAX_PLATFORM_WIDTH;
	float magnitude = PLATFORM_IMPULSE * (1.5f - (distance > 1.0f ? 1.0f : distance));
	// impulse wil be in the direction of the contact normal
	b2Vec2 impulse = contact_normal_;
	// ensure impulse throws the clown up not down
	impulse.y = impulse.y < 0 ? impulse.y * -1 : impulse.y;
	impulse *= magnitude;

	// apply a linear impulse to the center of the clown's body
	clown_->GetBody()->ApplyLinearImpulse(impulse, clown_->GetBody()->GetWorldCenter());

	// collision response finished, now remove the platform
	RemovePlatform();
}

void GameWorld::DoBasePlatformCollision()
{
	base_platform_->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjtrapm02.png"));
	// return to idle frame after short duration
	scheduleOnce(schedule_selector(GameWorld::DoBasePlatformIdle), 0.15f);
	// update the clown's state
	clown_->SetState(E_CLOWN_BOUNCE);

	SOUND_ENGINE->playEffect("base_platform.wav");
}

void GameWorld::DoBasePlatformIdle(float dt)
{
	base_platform_->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjtrapm01.png"));
}

void GameWorld::ResumeGame()
{
	is_popup_active_ = false;
	resumeSchedulerAndActions();
	clown_->Resume();
}

void GameWorld::GameOver()
{
	SOUND_ENGINE->playEffect("game_over.wav");

	unscheduleAllSelectors();
	setTouchEnabled(false);

	// create & add the game over popup
	GameOverPopup* game_over_popup = GameOverPopup::create(this, score_);
	addChild(game_over_popup, E_LAYER_POPUP);
}

void GameWorld::OnPauseClicked(CCObject* sender)
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;

	pauseSchedulerAndActions();
	clown_->Pause();

	// create & add the pause popup
	PausePopup* pause_popup = PausePopup::create(this);
	addChild(pause_popup, E_LAYER_POPUP);
}
