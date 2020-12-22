#include "GameWorld.h"
#include "Popups.h"
#include "b2dJson.h"
#include "Fish.h"
#include "Cat.h"
#include "Environment.h"

#ifdef ENABLE_DEBUG_DRAW
#include "GLES-Render.h"
#endif

GameWorld::GameWorld()
{
	is_popup_active_ = false;
	world_ = NULL;
#ifdef ENABLE_DEBUG_DRAW
	debug_draw_ = NULL;
#endif
	fish_list_ = string("");
	boundary_body_ = NULL;
	catapult_body_ = NULL;
	catapult_joint_ = NULL;
	mouse_joint_ = NULL;
	weld_joint_ = NULL;
	is_catapult_released_ = false;
	is_catapult_ready_ = false;
	fish_spawn_point_ = b2Vec2_zero;
	num_throwable_fish_ = 0;
	num_total_fish_ = 0;
	current_fish_ = -1;
	fish_.clear();
	num_cats_ = 0;
	num_dead_cats_ = 0;
	cats_.clear();
	num_game_objects_ = 0;
	game_objects_.clear();
}

GameWorld::~GameWorld()
{
	DestroyWorld();
}

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

	CreateGame();
    return true;
}

void GameWorld::CreateGame()
{
	// create RUBE's parser
	b2dJson* json = new b2dJson();
	CreateWorld(json, GameGlobals::level_number_);

	CreateGameObjects(json);
	CreateFish();
	CreateEnvironment();
	CreateHUD();
	SetFishSpawnPoint();

	setTouchEnabled(true);
	scheduleUpdate();
	scheduleOnce(schedule_selector(GameWorld::SpawnFish), 1.0f);

	// delete RUBE's parser
	delete json;
}

void GameWorld::CreateWorld(b2dJson* json, int level)
{
	// get file data and parse it to get b2dJson
	char buf[32] = {0};
	sprintf(buf, "Level%02d.json", level);
	//sprintf(buf, "testing.json");
	unsigned long size;
	char* data_uc = (char*)CCFileUtils::sharedFileUtils()->getFileData(buf, "rb", &size);

	// error message
	std::string msg = "could not load file";

	world_ = json->readFromString(data_uc, msg);
	// tell world we want to listen for collisions
	world_->SetContactListener(this);

	// delete char buffer
	delete data_uc;

#ifdef ENABLE_DEBUG_DRAW
	debug_draw_ = new GLESDebugDraw(PTM_RATIO);
	world_->SetDebugDraw(debug_draw_);
	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;
	flags += b2Draw::e_jointBit;
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
}

void GameWorld::CreateGameObjects(b2dJson* json)
{
	// save references to a few important bodies
	boundary_body_ = json->getBodyByName(string("ground"));
	catapult_body_ = json->getBodyByName(string("catapult"));
	catapult_joint_ = (b2RevoluteJoint*)json->getJointByName(string("catapult_joint"));

	// extract the list of fish (comma separated string) for this level
	fish_list_ = json->getCustomString((void*)world_, "ListOfFish", "");

	// get all bodies from the world that have a sprite name i.e. GameObjects
	b2Body* body_in_list = world_->GetBodyList();
	while(body_in_list != NULL)
	{
		string sprite_name = json->getCustomString((void*)body_in_list, "SpriteName", "");
		if(sprite_name.compare(""))
		{
			// append file extension...sorry we don't have spritesheets at the moment! :(
			sprite_name = sprite_name + ".png";

			// see if this game object is a cat
			if(json->getCustomBool((void*)body_in_list, "IsCat", false))
			{
				// create the Cat
				Cat* cat = Cat::create(this);
				// save the body
				cat->SetBody(body_in_list);
				// add & save this Cat
				addChild(cat, E_LAYER_CATS);
				cats_.push_back(cat);
				++ num_cats_;
			}
			else
			{
				// create the GameObject with the respective sprite name
				GameObject* game_object = GameObject::create(this, sprite_name.c_str());
				// save the body
				game_object->SetBody(body_in_list);
				// add & save this GameObject
				addChild(game_object, E_LAYER_OBJECTS);
				game_objects_.push_back(game_object);
				++ num_game_objects_;
			}
		}

		// continue checking
		body_in_list = body_in_list->GetNext();
	}

	// reorder catapult to be above the fish
	reorderChild((CCSprite*)catapult_body_->GetUserData(), E_LAYER_FISH + 1);
}

void GameWorld::CreateFish()
{
	Fish* fish = NULL;
	// get integer list from comma separated string
	vector<int> fish_type_vec = GameGlobals::GetIntListFromString(fish_list_);
	// this list consists of throwable fish
	num_throwable_fish_ = fish_type_vec.size();
	// initially, we will only have throwable fish
	num_total_fish_ = num_throwable_fish_;

	for(int i = 0; i < num_throwable_fish_; ++i)
	{
		// create each type of fish
		EFishType fish_type = (EFishType)fish_type_vec[i];
		switch(fish_type)
		{
		case E_FISH_SIMPLE:
			fish = Fish::create(this);
			break;
		case E_FISH_SHOOTING:
			fish = ShootingFish::create(this);
			break;
		case E_FISH_SPLITTING:
			fish = SplittingFish::create(this);
			break;
		case E_FISH_BOMBING:
			fish = BombingFish::create(this);
			break;
		case E_FISH_EXPLODING:
			fish = ExplodingFish::create(this);
			break;
		}

		if(fish != NULL)
		{
			// tell this fish it is throwable...default is false
			fish->SetIsThrowable(true);
			// initially no Update processing
			fish->setVisible(false);
			// add & save this fish
			addChild(fish, E_LAYER_FISH);
			fish_.push_back(fish);
		}

		fish = NULL;
	}
}

void GameWorld::CreateEnvironment()
{
	// create the background environment
	Environment* environment = Environment::create();
	addChild(environment, E_LAYER_BACKGROUND);

	// add objects that are in front of the foreground
	CCSprite* boat = CCSprite::create("boat_base.png");
	boat->setPosition(ccp(25.0f, 100.0f));
	addChild(boat, E_LAYER_FISH + 1);

	CCLayerColor* water = CCLayerColor::create(ccc4(101, 196, 184, 128), SCREEN_SIZE.width, 75);
	addChild(water, E_LAYER_FISH + 1);
}

void GameWorld::CreateHUD()
{
	// create & add the pause button's menu
	CCMenu* menu = CCMenu::create();
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu, E_LAYER_HUD);

	// create & add the pause button
	CCMenuItemSprite* pause_button = CCMenuItemSprite::create(CCSprite::create("pause_button.png"), CCSprite::create("pause_button.png"), this, menu_selector(GameWorld::OnPauseClicked));
	pause_button->setPosition(ccp(SCREEN_SIZE.width * 0.95f, SCREEN_SIZE.height * 0.9f));
	menu->addChild(pause_button);
}

void GameWorld::SetFishSpawnPoint()
{
	// search for the catapult's ledge fixture
	b2Fixture* catapult_ledge = catapult_body_->GetFixtureList();
	while(catapult_ledge != NULL)
	{
		if(catapult_ledge->GetType() == b2Shape::e_chain)
		{
			break;
		}
		catapult_ledge = catapult_ledge->GetNext();
	}

	// oops!
	if(catapult_ledge == NULL)
	{
		CCLOGERROR("Could not find catapult ledge");
		return;
	}

	// save the outer vertex of the catapult's ledge
	b2ChainShape* catapult_ledge_shape = (b2ChainShape*)catapult_ledge->GetShape();
	fish_spawn_point_ = catapult_ledge_shape->m_vertices[1];
	fish_spawn_point_ += catapult_body_->GetPosition();
}

void GameWorld::SpawnFish(float dt)
{
	// DON'T spawn if current_fish_ is out of bounds AND
	// if the current fish has not been fired
	if(current_fish_ >= 0 && current_fish_ < num_throwable_fish_ && fish_[current_fish_]->GetHasBeenFired() == false)
	{
		return;
	}

	// if there are no fish left, its game over!
	if(++ current_fish_ >= num_throwable_fish_)
	{
		GameOver();
		return;
	}

	// spawn the current fish
	fish_[current_fish_]->Spawn();
	
	// weld the fish to the catapult...else it might fall off
	b2WeldJointDef weld_jd;
	weld_jd.Initialize(fish_[current_fish_]->GetBody(), catapult_body_, fish_spawn_point_);
	weld_joint_ = (b2WeldJoint*)world_->CreateJoint(&weld_jd);
	
	// welding complete, catapult ready
	is_catapult_ready_ = true;
}

void GameWorld::AddFish(Fish* fish)
{
	// this function is called by SplittingFish & BombingFish from their Touch()
	if(fish == NULL)
	{
		CCLOGERROR("Bad call to AddFish");
		return;
	}

	// increment the counter, add & save this fish
	++ num_total_fish_;
	addChild(fish);
	fish_.push_back(fish);
}

void GameWorld::ReleaseFish()
{
	// catapult cannot be used till the next fish has spawned
	is_catapult_released_ = false;
	is_catapult_ready_ = false;
	// tell the fish its do or die now!
	fish_[current_fish_]->SetHasBeenFired(true);
}

void GameWorld::CatHasDied()
{
	// if all the cats are dead, the level has been completed
	if(++num_dead_cats_ >= num_cats_)
	{
		LevelComplete();
	}
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

	// update all fish
	for(int i = 0; i < num_total_fish_; ++i)
	{
		fish_[i]->Update();
	}

	// update all cats
	for(int i = 0; i < num_cats_; ++i)
	{
		cats_[i]->Update();
	}

	// update all GameObjects
	for(int i = 0; i < num_game_objects_; ++i)
	{
		game_objects_[i]->Update();
	}

	// has the catapult been released?
	if(is_catapult_released_)
	{
		// fish is released only after the catapult has returned back to the initial angle
		if(catapult_joint_->GetJointAngle() <= catapult_joint_->GetLowerLimit())
		{
			ReleaseFish();
		}
	}
}

void GameWorld::ccTouchesBegan(CCSet* set, CCEvent* event)
{
	if(is_catapult_ready_ && !mouse_joint_)
	{
		CCTouch* touch = (CCTouch*)(*set->begin());
		CCPoint touch_point = touch->getLocationInView();
		touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

		// convert from screen to physics co-ordinates
		b2Vec2 touch_world_point = b2Vec2(SCREEN_TO_WORLD(touch_point.x), SCREEN_TO_WORLD(touch_point.y));
		// only accept touches to the left of the catapult
		if(touch_world_point.x < catapult_body_->GetPosition().x)
		{
			// define the mouse joint's properties
			b2MouseJointDef mouse_jd;
			mouse_jd.bodyA = boundary_body_;
			mouse_jd.bodyB = catapult_body_;
			mouse_jd.target = touch_world_point;
			mouse_jd.maxForce = 10000;

			// create the mouse joint
			mouse_joint_ = (b2MouseJoint*)world_->CreateJoint(&mouse_jd);
		}
	}

	// touch the current fish
	if(current_fish_ >= 0)
	{
		fish_[current_fish_]->Touch();
	}
}

void GameWorld::ccTouchesMoved(CCSet* set, CCEvent* event)
{
	if(!mouse_joint_)
		return;

	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	// move the mouse joint
	mouse_joint_->SetTarget(b2Vec2(SCREEN_TO_WORLD(touch_point.x), SCREEN_TO_WORLD(touch_point.y)));
}

void GameWorld::ccTouchesEnded(CCSet* set, CCEvent* event)
{
	if(mouse_joint_)
	{
		// only release the catapult if it has been pulled beyond the minimum angle
		if(catapult_joint_->GetJointAngle() > MINIMUM_CATAPULT_ANGLE)
		{
			is_catapult_released_ = true;

			// destroy the weld joint
			if(weld_joint_)
			{
				world_->DestroyJoint(weld_joint_);
				weld_joint_ = NULL;
			}
		}

		// destroy the mouse joint
		world_->DestroyJoint(mouse_joint_);
		mouse_joint_ = NULL;
	}
}

void GameWorld::BeginContact(b2Contact* contact)
{
	b2Body* body_a = contact->GetFixtureA()->GetBody();
	b2Body* body_b = contact->GetFixtureB()->GetBody();

	// only react to contacts involving GameObjects
	if(body_a->GetUserData() == NULL || body_b->GetUserData() == NULL)
	{
		return;
	}

	// cast to GameObject
	GameObject* game_object_a = (GameObject*)body_a->GetUserData();
	GameObject* game_object_b = (GameObject*)body_b->GetUserData();
	
	// did this contact involve a fish?
	if(game_object_a->GetType() == E_GAME_OBJECT_FISH || game_object_b->GetType() == E_GAME_OBJECT_FISH)
	{
		// call the fish's Hit function but not for if the fish has collided with the catapult itself
		if(current_fish_ >= 0 && body_a != catapult_body_ && body_b != catapult_body_)
		{
			fish_[current_fish_]->Hit();
		}
	}
}

void GameWorld::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
	b2Body* body_a = contact->GetFixtureA()->GetBody();
	b2Body* body_b = contact->GetFixtureB()->GetBody();

	// only react to contacts involving GameObjects
	if(body_a->GetUserData() == NULL || body_b->GetUserData() == NULL)
	{
		return;
	}

	// cast to GameObject
	GameObject* game_object_a = (GameObject*)body_a->GetUserData();
	GameObject* game_object_b = (GameObject*)body_b->GetUserData();

	// did this contact involve a cat?
	if(game_object_a->GetType() == E_GAME_OBJECT_CAT || game_object_b->GetType() == E_GAME_OBJECT_CAT)
	{
		// cast to Cat
		Cat* cat = (Cat*)(game_object_a->GetType() == E_GAME_OBJECT_CAT ? game_object_a : game_object_b);

		// save the maximum impulse on this fixture
		for(int i = 0; i < impulse->count; ++i)
		{
			cat->SetMaxImpulse(impulse->normalImpulses[i]);
		}
	}
}

float32 GameWorld::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
	// a fixture has been intersected, save its body & the point of intersection
	ray_cast_data_.body_ = fixture->GetBody();
	ray_cast_data_.point_ = point;
	// return the fraction...this will help us find the fixture nearest to the start point of the ray
	return fraction;
}

void GameWorld::OnPauseClicked(CCObject* sender)
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;

	pauseSchedulerAndActions();
	setTouchEnabled(false);
	
	// pause game elements here

	// create & add the pause popup
	PausePopup* pause_popup = PausePopup::create(this);
	addChild(pause_popup, E_LAYER_POPUP);
}

void GameWorld::ResumeGame()
{
	is_popup_active_ = false;

	// resume GameWorld update & Tick functions
	resumeSchedulerAndActions();
	setTouchEnabled(true);

	// resume game elements here
}

void GameWorld::LevelComplete()
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;

	is_popup_active_ = true;

	// stop GameWorld update
	unscheduleAllSelectors();
	setTouchEnabled(false);

	// stop game elements here

	// create & add the game over popup
	LevelCompletePopup* level_complete_popup = LevelCompletePopup::create(this);
	addChild(level_complete_popup, E_LAYER_POPUP);
	//SOUND_ENGINE->playEffect("level_complete.wav");
}

void GameWorld::GameOver()
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;

	is_popup_active_ = true;

	// stop GameWorld update
	unscheduleAllSelectors();
	setTouchEnabled(false);

	// stop game elements here

	// create & add the game over popup
	GameOverPopup* game_over_popup = GameOverPopup::create(this);
	addChild(game_over_popup, E_LAYER_POPUP);
	//SOUND_ENGINE->playEffect("game_over.wav");
}