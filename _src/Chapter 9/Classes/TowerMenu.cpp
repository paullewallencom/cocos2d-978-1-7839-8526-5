#include "TowerMenu.h"
#include "GameWorld.h"

TowerMenu::TowerMenu()
{
	game_world_ = NULL;
	placement_node_ = NULL;
	placement_buttons_.clear();
	maintenance_node_ = NULL;
	upgrade_button_ = NULL;
	upgrade_label_ = NULL;
	sell_button_ = NULL;
	sell_label_ = NULL;
}

TowerMenu::~TowerMenu()
{}

TowerMenu* TowerMenu::create(GameWorld* game_world)
{
	TowerMenu* tower_menu = new TowerMenu();
	if(tower_menu && tower_menu->init(game_world))
	{
		tower_menu->autorelease();
		return tower_menu;
	}
	CC_SAFE_DELETE(tower_menu);
	return NULL;
}

bool TowerMenu::init(GameWorld* game_world)
{
	if(!CCNode::init())
	{
		return false;
	}
	
	game_world_ = game_world;
	CreatePlacementMenu();
	CreateMaintenanceMenu();
	return true;
}

void TowerMenu::CreatePlacementMenu()
{
	placement_node_ = CCNode::create();
	placement_node_->setContentSize(CCSizeMake(400, 400));
	placement_node_->setScale(0.0f);
	placement_node_->setVisible(false);
	addChild(placement_node_);

	CCMenu* placement_menu = CCMenu::create();
	placement_menu->setPosition(CCPointZero);
	placement_node_->addChild(placement_menu);

	// create a button to place the tower
	CCMenuItemSprite* tower_btn = CreateButton("TD_tw updateicon_01.png", ccp(-150, 50), 0, menu_selector(GameWorld::OnTowerButtonClicked));
	placement_menu->addChild(tower_btn);
	placement_buttons_.push_back(tower_btn);
	placement_node_->addChild(CreateLabel(GameGlobals::tower_data_sets_[0]->tower_data_[0]->cost_, "Arial", 24, ccp(-150, 20)));
	
	// create a sprite to demonstrate the gesture needed to spawn this tower
	CCSprite* swipe_tutorial = CCSprite::create("swipe_left.png");
	swipe_tutorial->setPosition(ccp(-180, 150));
	placement_node_->addChild(swipe_tutorial);

	// create a button to place the tower
	tower_btn = CreateButton("TD_tw updateicon_02.png", ccp(0, 150), 1, menu_selector(GameWorld::OnTowerButtonClicked));
	placement_menu->addChild(tower_btn);
	placement_buttons_.push_back(tower_btn);
	placement_node_->addChild(CreateLabel(GameGlobals::tower_data_sets_[1]->tower_data_[0]->cost_, "Arial", 24, ccp(0, 125)));
	
	// create a sprite to demonstrate the gesture needed to spawn this tower
	swipe_tutorial = CCSprite::create("swipe_up.png");
	swipe_tutorial->setPosition(ccp(0, 275));
	placement_node_->addChild(swipe_tutorial);
	
	// create a button to place the tower
	tower_btn = CreateButton("TD_tw updateicon_03.png", ccp(150, 50), 2, menu_selector(GameWorld::OnTowerButtonClicked));
	placement_menu->addChild(tower_btn);
	placement_buttons_.push_back(tower_btn);
	placement_node_->addChild(CreateLabel(GameGlobals::tower_data_sets_[2]->tower_data_[0]->cost_, "Arial", 24, ccp(150, 20)));
	
	// create a sprite to demonstrate the gesture needed to spawn this tower
	swipe_tutorial = CCSprite::create("swipe_right.png");
	swipe_tutorial->setPosition(ccp(180, 150));
	placement_node_->addChild(swipe_tutorial);
	
	// create a sprite to highlight the targeted tile
	CCSprite* highlight = CCSprite::create("tw_adtoicon.png");
	placement_node_->addChild(highlight);
}

void TowerMenu::CreateMaintenanceMenu()
{
	maintenance_node_ = CCNode::create();
	maintenance_node_->setContentSize(CCSizeMake(300, 300));
	maintenance_node_->setScale(0.0f);
	maintenance_node_->setVisible(false);
	addChild(maintenance_node_);

	CCMenu* maintenance_menu = CCMenu::create();
	maintenance_menu->setPosition(CCPointZero);
	maintenance_node_->addChild(maintenance_menu);

	// create a button to upgrade the tower
	upgrade_button_ = CreateButton("TD_tupdate01.png", ccp(0, 100), 0, menu_selector(GameWorld::OnUpgradeTowerClicked));
	maintenance_menu->addChild(upgrade_button_);
	upgrade_label_ = CreateLabel(0, "Arial", 24, ccp(0, 70));
	maintenance_node_->addChild(upgrade_label_);

	// create a button to sell the tower
	sell_button_ = CreateButton("TD_tucancel02.png", ccp(0, -100), 0, menu_selector(GameWorld::OnSellTowerClicked));
	maintenance_menu->addChild(sell_button_);
	sell_label_ = CreateLabel(0, "Arial", 24, ccp(0, -130));
	maintenance_node_->addChild(sell_label_);
}

void TowerMenu::ShowPlacementMenu(CCPoint position)
{
	// animate the menu
	placement_node_->runAction(CCSequence::create(CCShow::create(), CCPlace::create(position), CCEaseBackOut::create(CCScaleTo::create(0.2f, 1.0f)), NULL));
	EnablePlacementButtons();
}

void TowerMenu::HidePlacementMenu()
{
	// animate the menu
	placement_node_->runAction(CCSequence::create(CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.0f)), CCHide::create(), CCPlace::create(CCPointZero), NULL));
}

void TowerMenu::EnablePlacementButtons()
{
	// check if a tower button should be enabled based on whether the player can afford it
	for(int i = 0; i < 3; ++i)
	{
		bool enabled = (game_world_->GetCash() >= GameGlobals::tower_data_sets_[i]->tower_data_[0]->cost_);
		placement_buttons_[i]->setEnabled(enabled);
		// change colour since we don't have a different sprite for disabled state
		placement_buttons_[i]->setColor(enabled ? ccc3(255, 255, 255) : ccc3(128, 128, 128));
	}
}

void TowerMenu::ShowMaintenanceMenu(CCPoint position, int tower_index, int tower_type, int tower_level)
{
	char buf[16] = {0};
	// set the appropriate price into the upgrade label
	if(tower_level < NUM_TOWER_UPGRADES - 1)
	{
		sprintf(buf, "%d", GameGlobals::tower_data_sets_[tower_type]->tower_data_[tower_level + 1]->cost_);
	}
	else
	{
		sprintf(buf,"-");
	}
	upgrade_label_->setString(buf);

	int total_cost = 0;
	// calculate the price into the sell label
	for(int i = 0; i <= tower_level; ++i)
	{
		// resale value will be half of the total expenditure
		// total expenditure includes initial price and upgrades
		total_cost += GameGlobals::tower_data_sets_[tower_type]->tower_data_[i]->cost_;
	}
	sprintf(buf, "%d", ((int)(total_cost / 2)));
	sell_label_->setString(buf);

	// save tower information into the upgrade & sell buttons
	upgrade_button_->setTag(tower_index);
	sell_button_->setTag(tower_index);

	maintenance_node_->runAction(CCSequence::create(CCShow::create(), CCPlace::create(position), CCEaseBackOut::create(CCScaleTo::create(0.2f, 1.0f)), NULL));
	EnableMaintenanceButtons(tower_type, tower_level);
}

void TowerMenu::HideMaintenanceMenu()
{
	maintenance_node_->runAction(CCSequence::create(CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.0f)), CCHide::create(), CCPlace::create(CCPointZero), NULL));
}

void TowerMenu::EnableMaintenanceButtons(int tower_type, int tower_level)
{
	// check if the upgrade button should be enabled based on whether the player can afford it
	bool enabled = (game_world_->GetCash() >= GameGlobals::tower_data_sets_[tower_type]->tower_data_[tower_level + 1]->cost_);
	upgrade_button_->setEnabled(enabled);
	upgrade_button_->setColor(enabled ? ccc3(255, 255, 255) : ccc3(128, 128, 128));
}

CCMenuItemSprite* TowerMenu::CreateButton(const char* sprite_name, CCPoint position, int tag, SEL_MenuHandler handler)
{
	CCMenuItemSprite* button = CCMenuItemSprite::create(CCSprite::create(sprite_name), CCSprite::create(sprite_name), game_world_, handler);
	button->setTag(tag);
	button->setPosition(position);
	return button;
}

CCLabelTTF* TowerMenu::CreateLabel(int content, const char* font, float size, CCPoint position)
{
	char buf[16] = {0};
	sprintf(buf, "%d", content);
	CCLabelTTF* label = CCLabelTTF::create(buf, font, size);
	label->setPosition(position);
	label->setColor(ccc3(0, 102, 0));
	return label;
}