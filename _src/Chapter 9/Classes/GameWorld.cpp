#include "GameWorld.h"
#include "Popups.h"
#include "Tower.h"
#include "Enemy.h"
#include "TowerMenu.h"
#include "GestureLayer.h"

GameWorld::GameWorld()
{
	time_scale_ = 0.0f;
	tiled_map_ = NULL;
	tmx_layer_ = NULL;
	num_enemy_walk_points_ = 0;
	enemy_walk_points_.clear();
	num_towers_ = 0;
	towers_.clear();
	num_waves_ = 0;
	curr_wave_index_ = 0;
	is_wave_starting_ = false;
	curr_wave_ = NULL;
	waves_.clear();
	waves_label_ = NULL;
	cash_ = 0;
	cash_label_ = NULL;
	lives_left_ = 0;
	pumpkin_position_ = CCPointZero;
	pumpkin_ = NULL;
	lives_label_ = NULL;
	tower_menu_ = NULL;
	grid_node_ = NULL;
	hud_menu_ = NULL;
	gesture_layer_ = NULL;
	is_popup_active_ = false;
}

GameWorld::~GameWorld()
{
	for(int i = 0; i < num_waves_; ++i)
	{
		delete waves_[i];
	}
	waves_.clear();
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
	// initialise variables
	time_scale_ = 1.0f;
	lives_left_ = 20;
	curr_wave_index_ = -1;
	// TODO: remove this when more levels are added
	GameGlobals::level_number_ = 0;

	// create & add the green background
	CCSprite* bg = CCSprite::create("bg.png");
	bg->setAnchorPoint(CCPointZero);
	addChild(bg, E_LAYER_BACKGROUND);

	CreateTiledMap();
	CreateGrid();
	CreateWalkPoints();
	CreateWaves();
	CreateHUD();
	CreatePumpkin();

	// create & add the tower placement & maaintenance menus
	tower_menu_ = TowerMenu::create(this);
	addChild(tower_menu_, E_LAYER_HUD + 1);

	// create & add the gesture layer
	gesture_layer_ = GestureLayer::create(this, callfuncO_selector(GameWorld::OnGestureReceived));
	addChild(gesture_layer_);

	// start the first wave in a few seconds
	scheduleOnce(schedule_selector(GameWorld::StartNextWave), 2.0f);
	scheduleUpdate();
	// reset the scheduler's time scale
	CCDirector::sharedDirector()->getScheduler()->setTimeScale(time_scale_);
}

void GameWorld::CreateTiledMap()
{
	// generate level filename
	char buf[128] = {0};
	sprintf(buf, "level_%02d.tmx", GameGlobals::level_number_ + 1);
	
	// create & add the tiled map
	tiled_map_ = CCTMXTiledMap::create(buf);
	addChild(tiled_map_, E_LAYER_BACKGROUND+1);
	tiled_map_->setPosition(ccp(SCREEN_SIZE.width/2 - tiled_map_->getContentSize().width/2, SCREEN_SIZE.height/2 - tiled_map_->getContentSize().height/2));
	// save a reference to the TMX layer
	tmx_layer_ = tiled_map_->layerNamed("EnemyPath");
}

void GameWorld::CreateGrid()
{
	// create a node to draw the grid
	grid_node_ = CCDrawNode::create();
	grid_node_->setVisible(false);
	// position it relative to the tiled map
	grid_node_->setPosition(ccpAdd(tiled_map_->getPosition(), ccp(0, 100)));
	addChild(grid_node_, E_LAYER_BACKGROUND);
	CCSize map_size = tiled_map_->getMapSize();
	
	int counter = 0;
	// run a loop to alternatively draw a semi-transparent black square indicating a tile
	for(int i = 0; i < ((int)map_size.width); ++i)
	{
		for(int j = 0; j < ((int)map_size.height); ++j)
		{
			++ counter;
			if( (i % 2 == 0 && counter % 2 == 0) || (i % 2 != 0 && counter % 2 != 0) )
			{
				continue;
			}

			CCPoint vertices[4] = {ccp( i * TILE_SIZE + 2, SCREEN_SIZE.height - j * TILE_SIZE - 2 ),
				ccp( (i+1) * TILE_SIZE - 2, SCREEN_SIZE.height - j * TILE_SIZE - 2 ),
				ccp( (i+1) * TILE_SIZE - 2, SCREEN_SIZE.height - (j+1) * TILE_SIZE + 2 ),
				ccp( i * TILE_SIZE + 2, SCREEN_SIZE.height - (j+1) * TILE_SIZE + 2 )};
			grid_node_->drawPolygon(vertices, 4, ccc4f(0.0f, 0.0f, 0.0f, 0.12f), 0, ccc4f(0.0f, 0.0f, 0.0f, 0.0f));
		}
	}
}

void GameWorld::CreateWalkPoints()
{
	// parse the list of objects
	CCTMXObjectGroup* object_group = tiled_map_->objectGroupNamed("PathObjects");
	CCArray* objects = object_group->getObjects();
	int num_objects = objects->count();
	CCDictionary* object = NULL;

	for(int i = 0; i < num_objects; ++i)
	{
		object = (CCDictionary*)(objects->objectAtIndex(i));
		// save each WalkPoint's position for enemies to use
		if(strstr(object->valueForKey("name")->getCString(), "WalkPoint") != NULL)
		{
			enemy_walk_points_.push_back(ccp(object->valueForKey("x")->floatValue(), object->valueForKey("y")->floatValue()));
		}
	}

	num_enemy_walk_points_ = enemy_walk_points_.size();
}

void GameWorld::CreateWaves()
{
	// generate level filename
	char buf[128] = {0};
	sprintf(buf, "level_%02d.xml", GameGlobals::level_number_ + 1);

	// read file
	unsigned long size;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(buf, "rb", &size);

	// parse file
	tinyxml2::XMLDocument xml_document;
	tinyxml2::XMLError xml_result = xml_document.Parse(data, size);

	CC_SAFE_DELETE(data);

	// print the error if parsing was unsuccessful
	if(xml_result != tinyxml2::XML_SUCCESS)
	{
		CCLOGERROR("Error:%d while reading %s", xml_result, buf);
		return;
	}

	tinyxml2::XMLNode* level_node = xml_document.FirstChild();
	// save the initial cash for this level
	cash_ = level_node->ToElement()->IntAttribute("cash");

	tinyxml2::XMLElement* wave_element = NULL;
	// loop through each Wave tag
	for(tinyxml2::XMLNode* wave_node = level_node->FirstChild(); wave_node != NULL; wave_node = wave_node->NextSibling())
	{
		wave_element = wave_node->ToElement();
		// get list of enemy indices
		vector<int> enemy_list = GameGlobals::GetIntListFromString(string(wave_element->Attribute("enemy_list")));

		// createa a new Wave object
		Wave* wave = new Wave();
		// save the spawn delay & list of enemies for this wave
		wave->num_enemies_ = enemy_list.size();
		wave->spawn_delay_ = wave_element->FloatAttribute("spawn_delay");
		// create all enemies in advance
		for(int i = 0; i < wave->num_enemies_; ++i)
		{
			Enemy* enemy = Enemy::create(this, enemy_list[i]);
			wave->enemies_.push_back(enemy);
			addChild(enemy, E_LAYER_ENEMY);
		}

		++ num_waves_;
		waves_.push_back(wave);
	}
}

void GameWorld::CreateHUD()
{
	CCSprite* sprite = CCSprite::create("TD_hudbg.png");
	sprite->setPosition(ccp(640, 759));
	addChild(sprite, E_LAYER_HUD);

	sprite = CCSprite::create("TD_pumbtn.png");
	sprite->setPosition(ccp(100, 755));
	addChild(sprite, E_LAYER_HUD);

	sprite = CCSprite::create("TD_bgbtn.png");
	sprite->setPosition(ccp(500, 755));
	addChild(sprite, E_LAYER_HUD);

	// create the cash label
	char buf[64] = {0};
	sprintf(buf, "%d", cash_);
	cash_label_ = CCLabelTTF::create(buf, "Arial", 40);
	cash_label_->setPosition(ccp(190, 755));
	addChild(cash_label_, E_LAYER_HUD);

	// create the waves label
	sprintf(buf, "1/%d", num_waves_);
	waves_label_ = CCLabelTTF::create(buf, "Arial", 40);
	waves_label_->setPosition(ccp(500, 755));
	addChild(waves_label_, E_LAYER_HUD);

	CCLabelTTF* label = CCLabelTTF::create("Waves", "Arial", 40);
	label->setPosition(ccp(650, 755));
	addChild(label, E_LAYER_HUD);

	// create & add the menu
	hud_menu_ = CCMenu::create();
	hud_menu_->setAnchorPoint(CCPointZero);
	hud_menu_->setPosition(CCPointZero);
	addChild(hud_menu_, E_LAYER_HUD);

	// create the speed toggle
	CCMenuItemSprite* menu_item_1 = CCMenuItemSprite::create(CCSprite::create("TD_2Xbtn.png"), CCSprite::create("TD_2Xbtn.png"));
	CCMenuItemSprite* menu_item_2 = CCMenuItemSprite::create(CCSprite::create("TD_1Xbtn.png"), CCSprite::create("TD_1Xbtn.png"));
	CCMenuItemToggle* speed_toggle = CCMenuItemToggle::createWithTarget(this, menu_selector(GameWorld::OnToggleSpeedClicked), menu_item_1, menu_item_2, NULL);
	speed_toggle->setPosition(ccp(900, 755));
	hud_menu_->addChild(speed_toggle);

	// create & add the pause button
	CCMenuItemSprite* button = CCMenuItemSprite::create(CCSprite::create("TD_pausebtn.png"), CCSprite::create("TD_pausebtn.png"), this, menu_selector(GameWorld::OnPauseClicked));
	button->setPosition(ccp(1220, 755));
	hud_menu_->addChild(button);
}

void GameWorld::CreatePumpkin()
{
	// fetch the Pumpkin object from the tiled map
	CCTMXObjectGroup* object_group = tiled_map_->objectGroupNamed("PathObjects");
	CCDictionary* pumpkin_object = object_group->objectNamed("Pumpkin");
	
	if(pumpkin_object)
	{
		// create the sprite for the pumpkin
		pumpkin_ = CCSprite::create("TD_pumkin_01.png");
		pumpkin_->setPosition(ccp(pumpkin_object->valueForKey("x")->floatValue(), pumpkin_object->valueForKey("y")->floatValue()));
		addChild(pumpkin_, E_LAYER_TOWER);

		// create the sprite for the lives
		CCSprite* life_frame = CCSprite::create("TD_livebg.png");
		life_frame->setPosition(ccp(pumpkin_->getPositionX() - pumpkin_->getContentSize().width/2, pumpkin_->getPositionY() + pumpkin_->getContentSize().height/2));
		addChild(life_frame, E_LAYER_TOWER);

		// create the label for the lives
		char buf[8] = {0};
		sprintf(buf, "%02d", lives_left_);
		lives_label_ = CCLabelTTF::create(buf, "Arial", 32);
		lives_label_->setAnchorPoint(ccp(0.15f, 0.5f));
		lives_label_->setColor(ccc3(255, 255, 189));
		lives_label_->setPosition(life_frame->getPosition());
		addChild(lives_label_, E_LAYER_TOWER);
	}
}

void GameWorld::PlaceTower(int type, CCPoint position)
{
	// can the player afford this tower?
	if(cash_ < GameGlobals::tower_data_sets_[type]->tower_data_[0]->cost_)
	{
		return;
	}

	// create a new Tower object & add it into the vector of towers
	Tower* tower = Tower::create(this, type, position);
	addChild(tower, E_LAYER_TOWER);
	++ num_towers_;
	towers_.push_back(tower);

	// save tower's information into the tile map
	position = tiled_map_->convertToNodeSpace(position);
	CCPoint tile_coord = ccp(GET_COL_FOR_X(position.x), GET_ROW_FOR_Y(position.y, MAX_ROWS));
	tmx_layer_->setTileGID(TOWER_GID + (num_towers_ - 1), tile_coord);

	// debit cash
	UpdateCash(-tower->GetCost());
	UpdateHUD();
	// show the range for this tower
	tower->ShowRange();

	// hide the grid now that the tower has been placed
	grid_node_->setVisible(false);
}

void GameWorld::SellTower(int index)
{
	// calculate the net worth of this tower
	int total_cost = 0;
	for(int i = 0; i <= towers_[index]->GetLevel(); ++i)
	{
		total_cost += GameGlobals::tower_data_sets_[towers_[index]->GetType()]->tower_data_[i]->cost_;
	}
	// credit cash
	UpdateCash((int)(total_cost / 2));

	// erase tower's information from the tile map
	CCPoint position = tiled_map_->convertToNodeSpace(towers_[index]->getPosition());
	CCPoint tile_coord = ccp(GET_COL_FOR_X(position.x), GET_ROW_FOR_Y(position.y, MAX_ROWS));
	tmx_layer_->setTileGID(0, tile_coord);

	// sell the tower & erase it from the vector
	towers_[index]->Sell();
	towers_.erase(towers_.begin() + index);
	-- num_towers_;
}

void GameWorld::StartNextWave(float dt)
{
	// increment the current wave index
	++ curr_wave_index_;
	// are there any waves left?
	if(curr_wave_index_ >= num_waves_)
	{
		// level has completed
		GameOver(true);
	}
	else
	{
		// start the next wave in a few seconds
		curr_wave_ = waves_[curr_wave_index_];
		schedule(schedule_selector(GameWorld::SpawnEnemy), curr_wave_->spawn_delay_);
		UpdateHUD();

		GameGlobals::ScaleLabel(waves_label_);
	}
}

void GameWorld::SpawnEnemy(float dt)
{
	// have all enemies of this wave been spawned?
	if(curr_wave_->num_enemies_spawned_ >= curr_wave_->num_enemies_)
	{
		// wave has finished starting
		is_wave_starting_ = false;
		// stop spawning enemies
		unschedule(schedule_selector(GameWorld::SpawnEnemy));
		return;
	}
	// fetch the next enemy in the list
	Enemy* enemy = curr_wave_->enemies_[curr_wave_->num_enemies_spawned_ ++];
	// tell the enemy where to go
	enemy->SetWalkPoints(num_enemy_walk_points_, enemy_walk_points_);
	// tell the enemy to start walking
	enemy->StartWalking();
	curr_wave_->num_enemies_walking_ ++;
}

void GameWorld::EnemyAtTheGates(Enemy* enemy)
{
	// this is called when the enemy has reached the pumpkin
	ReduceLives(enemy->GetDamage());
	curr_wave_->num_enemies_walking_ --;
}

void GameWorld::EnemyDown(Enemy* enemy)
{
	// this is called when the enemy 
	UpdateCash(enemy->GetReward());
	curr_wave_->num_enemies_walking_ --;
}

void GameWorld::ReduceLives(int amount)
{
	// reduce lives
	-- lives_left_;
	switch(lives_left_)
	{
	// change the pumpkin's appearance based on how many lives are left
	case 15:
		pumpkin_->initWithFile("TD_pumkin_02.png");
		break;
	case 10:
		pumpkin_->initWithFile("TD_pumkin_03.png");
		break;
	case 5:
		pumpkin_->initWithFile("TD_pumkin_04.png");
		break;
	case 0:
		// all lives over...level failed
		GameOver(false);
		break;
	}
	// update the lives label
	char buf[8] = {0};
	sprintf(buf, "%d", lives_left_);
	lives_label_->setString(buf);
	GameGlobals::ScaleLabel(lives_label_);
}

void GameWorld::UpdateCash(int amount)
{
	cash_ += amount;
	UpdateHUD();

	GameGlobals::ScaleLabel(cash_label_);
}

void GameWorld::update(float dt)
{
	// check if current wave has completed
	CheckWaveCompletion();

	// update all towers
	for(int i = 0; i < num_towers_; ++i)
	{
		towers_[i]->Update();
	}
}

void GameWorld::UpdateHUD()
{
	char buf[64] = {0};
	// update cash label
	sprintf(buf, "%d", cash_);
	cash_label_->setString(buf);

	// update waves label
	sprintf(buf, "%d/%d", curr_wave_index_+1, num_waves_);
	waves_label_->setString(buf);
}

void GameWorld::CheckWaveCompletion()
{
	// wave has completed when all enemies have been spawned AND
	// when there are no enemies walking (cuz they're all dead!)
	if(!is_wave_starting_ && curr_wave_ && curr_wave_->num_enemies_spawned_ >= curr_wave_->num_enemies_ && curr_wave_->num_enemies_walking_ <= 0)
	{
		// start the next wave
		is_wave_starting_ = true;
		scheduleOnce(schedule_selector(GameWorld::StartNextWave), 2.0f);
	}
}

void GameWorld::OnGestureReceived(CCObject* sender)
{
	GestureLayer* gesture_layer = (GestureLayer*)sender;
	// call the respective gesture's handler
	switch(gesture_layer->GetGestureType())
	{
	case E_GESTURE_TAP:
		HandleTap(gesture_layer->GetTouchStart());
		break;

	case E_GESTURE_SWIPE_UP:
		HandleSwipeUp();
		break;

	case E_GESTURE_SWIPE_DOWN:
		HandleSwipeDown();
		break;

	case E_GESTURE_SWIPE_LEFT:
		HandleSwipeLeft();
		break;

	case E_GESTURE_SWIPE_RIGHT:
		HandleSwipeRight();
		break;
	}
}

void GameWorld::HandleTap(CCPoint position)
{
	// get the touch coordinates with respect to the tile map
	CCPoint touch_point = tiled_map_->convertToNodeSpace(position);
	CCPoint tile_coord = ccp(GET_COL_FOR_X(touch_point.x), GET_ROW_FOR_Y(touch_point.y, MAX_ROWS));
	touch_point = ccpMult(tile_coord, TILE_SIZE);
	touch_point.y = SCREEN_SIZE.height - touch_point.y;
	
	// check if the touched tile is empty
	int tile_GID = tmx_layer_->tileGIDAt(tile_coord);
	// if the touched tile is empty, show the tower placement menu
	if(tile_GID == 0)
	{
		// check to ensure only one menu is visible at a time
		if(tower_menu_->placement_node_->isVisible() == false && tower_menu_->maintenance_node_->isVisible() == false)
		{
			tower_menu_->ShowPlacementMenu(touch_point);
			// show the grid
			grid_node_->setVisible(true);
		}
	}
	// a tower exists on the touched tile
	else if(tile_GID >= TOWER_GID)
	{
		int tower_index = tile_GID - TOWER_GID;
		// first check bounds and then check to ensure only one menu is visible at a time
		if(tower_index >= 0 && tower_index < num_towers_ && tower_menu_->maintenance_node_->isVisible() == false && tower_menu_->placement_node_->isVisible() == false)
		{
			// show the tower's current range
			towers_[tower_index]->ShowRange();
			tower_menu_->ShowMaintenanceMenu(touch_point, tower_index, towers_[tower_index]->GetType(), towers_[tower_index]->GetLevel());
		}
	}

	// hide the tower placement menu if it is visible
	if(tower_menu_->placement_node_->isVisible())
	{
		tower_menu_->HidePlacementMenu();
		grid_node_->setVisible(false);
	}
	// hide the tower maintenance menu if it is visible
	if(tower_menu_->maintenance_node_->isVisible())
	{
		tower_menu_->HideMaintenanceMenu();
	}
}

void GameWorld::HandleSwipeUp()
{
	// return if the tower placement menu is not active
	if(tower_menu_->placement_node_->isVisible() == false)
	{
		return;
	}

	// place the tower with specified type at specified position
	PlaceTower(1, tower_menu_->placement_node_->getPosition());
	tower_menu_->HidePlacementMenu();
	grid_node_->setVisible(false);
}

void GameWorld::HandleSwipeDown()
{}

void GameWorld::HandleSwipeLeft()
{
	// return if the tower placement menu is not active
	if(tower_menu_->placement_node_->isVisible() == false)
	{
		return;
	}
	
	// place the tower with specified type at specified position
	PlaceTower(0, tower_menu_->placement_node_->getPosition());
	tower_menu_->HidePlacementMenu();
	grid_node_->setVisible(false);
}

void GameWorld::HandleSwipeRight()
{
	// return if the tower placement menu is not active
	if(tower_menu_->placement_node_->isVisible() == false)
	{
		return;
	}
	
	// place the tower with specified type at specified position
	PlaceTower(2, tower_menu_->placement_node_->getPosition());
	tower_menu_->HidePlacementMenu();
	grid_node_->setVisible(false);
}

void GameWorld::OnTowerButtonClicked(CCObject* sender)
{
	// place the appropriate tower based on which button was pressed
	int tag = ((CCNode*)sender)->getTag();
	PlaceTower(tag, tower_menu_->placement_node_->getPosition());
	tower_menu_->HidePlacementMenu();
	grid_node_->setVisible(false);
}

void GameWorld::OnUpgradeTowerClicked(CCObject* sender)
{
	int index = ((CCNode*)sender)->getTag();
	// check bounds and upgrade the tower
	if(index >= 0 && index < num_towers_)
	{
		towers_[index]->Upgrade();
	}
	tower_menu_->HideMaintenanceMenu();
}

void GameWorld::OnSellTowerClicked(CCObject* sender)
{
	int index = ((CCNode*)sender)->getTag();
	// check bounds and sell the tower
	if(index >= 0 && index < num_towers_)
	{
		SellTower(index);
	}
	tower_menu_->HideMaintenanceMenu();
}

void GameWorld::OnToggleSpeedClicked(CCObject* sender)
{
	time_scale_ = time_scale_ == 1.0f ? 2.0f : 1.0f;
	// toggle the scheduler's time scale
	CCDirector::sharedDirector()->getScheduler()->setTimeScale(time_scale_);
}

void GameWorld::OnPauseClicked(CCObject* sender)
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;
	
	// reset the scheduler's time scale to what it was
	CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.0f);
	gesture_layer_->setTouchEnabled(false);
	hud_menu_->setEnabled(false);
	pauseSchedulerAndActions();
	
	// pause game elements here
	if(curr_wave_)
	{
		for(int i = 0; i < curr_wave_->num_enemies_; ++i)
		{
			curr_wave_->enemies_[i]->pauseSchedulerAndActions();
		}
	}

	for(int i = 0; i < num_towers_; ++i)
	{
		towers_[i]->pauseSchedulerAndActions();
	}

	// create & add the pause popup
	PausePopup* pause_popup = PausePopup::create(this);
	addChild(pause_popup, E_LAYER_POPUP);
}

void GameWorld::ResumeGame()
{
	is_popup_active_ = false;
	
	// set the scheduler's time scale to what it was
	CCDirector::sharedDirector()->getScheduler()->setTimeScale(time_scale_);
	gesture_layer_->setTouchEnabled(true);
	hud_menu_->setEnabled(true);
	resumeSchedulerAndActions();

	// resume game elements here
	if(curr_wave_)
	{
		for(int i = 0; i < curr_wave_->num_enemies_; ++i)
		{
			curr_wave_->enemies_[i]->resumeSchedulerAndActions();
		}
	}

	for(int i = 0; i < num_towers_; ++i)
	{
		towers_[i]->resumeSchedulerAndActions();
	}
}

void GameWorld::GameOver(bool is_level_complete)
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;

	is_popup_active_ = true;

	// reset the scheduler's time scale
	CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.0f);
	// stop GameWorld update
	unscheduleAllSelectors();
	gesture_layer_->setTouchEnabled(false);

	// stop game elements here
	if(curr_wave_)
	{
		for(int i = 0; i < curr_wave_->num_enemies_; ++i)
		{
			curr_wave_->enemies_[i]->stopAllActions();
			curr_wave_->enemies_[i]->unscheduleAllSelectors();
		}
	}

	for(int i = 0; i < num_towers_; ++i)
	{
		towers_[i]->stopAllActions();
		towers_[i]->unscheduleAllSelectors();
	}

	Popup* popup = NULL;
	// show the respective popup
	if(is_level_complete)
	{
		popup = LevelCompletePopup::create(this);
	}
	else
	{
		popup = GameOverPopup::create(this);
	}
	addChild(popup, E_LAYER_POPUP);
}