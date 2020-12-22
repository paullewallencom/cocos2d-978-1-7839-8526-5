#include "GameGlobals.h"

CCSize GameGlobals::screen_size_ = CCSizeZero;
int GameGlobals::level_number_ = 0;

int GameGlobals::num_tower_data_sets_ = 0;
vector<TowerDataSet*> GameGlobals::tower_data_sets_;
int GameGlobals::num_enemy_data_ = 0;
vector<EnemyData*> GameGlobals::enemy_data_;

GameGlobals::GameGlobals()
{
	// initialise tower & enemy libraries
	tower_data_sets_.clear();
	enemy_data_.clear();
}

GameGlobals::~GameGlobals()
{
	// clear tower library
	for(int i = 0; i < num_tower_data_sets_; ++i)
	{
		delete tower_data_sets_[i];
	}
	// clear enemy library
	for(int i = 0; i < num_enemy_data_; ++i)
	{
		delete enemy_data_[i];
	}

	num_tower_data_sets_ = 0;
	tower_data_sets_.clear();
	num_enemy_data_ = 0;
	enemy_data_.clear();
}

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

	// load sound effects & background music

	// create and add animations
	LoadAnimations();

	// load game data
	LoadTowerData();
	LoadEnemyData();
}

void GameGlobals::LoadAnimations()
{
	CCAnimation* animation = NULL;

	// enemy animations
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_010001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_010002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_1");
	
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_020001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_020002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_2");
	
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_030001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_030002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_3");
	
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_040001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_040002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_4");
	
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_050001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_050002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_5");
	
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_060001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_060002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_6");
	
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_070001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_070002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_7");
	
	animation = CCAnimation::create();
	animation->setDelayPerUnit(0.25f);
	animation->setLoops(-1);
	animation->addSpriteFrame(CCSprite::create("enemy_080001.png")->displayFrame());
	animation->addSpriteFrame(CCSprite::create("enemy_080002.png")->displayFrame());
	CCAnimationCache::sharedAnimationCache()->addAnimation(animation, "enemy_8");
}

void GameGlobals::LoadTowerData()
{
	// read file
	unsigned long size;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData("tower_data.xml", "rb", &size);

	// parse file
	tinyxml2::XMLDocument xml_document;
	tinyxml2::XMLError xml_result = xml_document.Parse(data, size);

	CC_SAFE_DELETE(data);

	// print the error if parsing was unsuccessful
	if(xml_result != tinyxml2::XML_SUCCESS)
	{
		CCLOGERROR("Error:%d while reading tower_data.xml", xml_result);
		return;
	}

	tinyxml2::XMLNode* tower_data_set_list = xml_document.FirstChild();
	tinyxml2::XMLElement* tower_data_set_element = NULL;
	tinyxml2::XMLElement* tower_data_element = NULL;
	// loop through each TowerDataSet tag
	for(tinyxml2::XMLNode* tower_data_set_node = tower_data_set_list->FirstChild(); tower_data_set_node != NULL; tower_data_set_node = tower_data_set_node->NextSibling())
	{
		tower_data_set_element = tower_data_set_node->ToElement();

		TowerDataSet* tower_data_set = new TowerDataSet();
		sprintf(tower_data_set->bullet_name_, "%s", tower_data_set_element->Attribute("bullet_name"));
		tower_data_set->is_lightning_ = tower_data_set_element->BoolAttribute("is_lightning");
		tower_data_set->is_rotating_ = tower_data_set_element->BoolAttribute("is_rotating");

		// loop through each TowerData tag
		for(tinyxml2::XMLNode* tower_data_node = tower_data_set_node->FirstChild(); tower_data_node != NULL; tower_data_node = tower_data_node->NextSibling())
		{
			tower_data_element = tower_data_node->ToElement();

			TowerData* tower_data = new TowerData();
			sprintf(tower_data->sprite_name_, "%s", tower_data_element->Attribute("sprite_name"));
			tower_data->range_ = tower_data_element->FloatAttribute("range");
			tower_data->physical_damage_ = tower_data_element->FloatAttribute("physical_damage");
			tower_data->magical_damage_ = tower_data_element->FloatAttribute("magical_damage");
			tower_data->speed_damage_ = tower_data_element->FloatAttribute("speed_damage");
			tower_data->speed_damage_duration_ = tower_data_element->FloatAttribute("speed_damage_duration");
			tower_data->fire_rate_ = tower_data_element->FloatAttribute("fire_rate");
			tower_data->cost_ = tower_data_element->IntAttribute("cost");

			tower_data_set->tower_data_.push_back(tower_data);
		}

		tower_data_sets_.push_back(tower_data_set);
		num_tower_data_sets_ ++;
	}
}

void GameGlobals::LoadEnemyData()
{
	// read file
	unsigned long size;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData("enemy_data.xml", "rb", &size);

	// parse file
	tinyxml2::XMLDocument xml_document;
	tinyxml2::XMLError xml_result = xml_document.Parse(data, size);

	CC_SAFE_DELETE(data);

	// print the error if parsing was unsuccessful
	if(xml_result != tinyxml2::XML_SUCCESS)
	{
		CCLOGERROR("Error:%d while reading enemy_data.xml", xml_result);
		return;
	}

	tinyxml2::XMLNode* enemy_data_list = xml_document.FirstChild();
	tinyxml2::XMLElement* enemy_data_element = NULL;
	// loop through each EnemyData tag
	for(tinyxml2::XMLNode* enemy_data_node = enemy_data_list->FirstChild(); enemy_data_node != NULL; enemy_data_node = enemy_data_node->NextSibling())
	{
		enemy_data_element = enemy_data_node->ToElement();

		EnemyData* enemy_data = new EnemyData();
		sprintf(enemy_data->animation_name_, "%s", enemy_data_element->Attribute("animation"));
		enemy_data->health_ = enemy_data_element->IntAttribute("health");
		enemy_data->armor_ = enemy_data_element->IntAttribute("armor");
		enemy_data->magic_resistance_ = enemy_data_element->IntAttribute("magic_resistance");
		enemy_data->speed_ = enemy_data_element->FloatAttribute("speed");
		enemy_data->damage_ = enemy_data_element->IntAttribute("damage");
		enemy_data->reward_ = enemy_data_element->IntAttribute("reward");
		enemy_data_.push_back(enemy_data);
		num_enemy_data_ ++;
	}
}

void GameGlobals::ScaleLabel(CCLabelTTF* label)
{
	// helper function to animate a label
	label->runAction(CCSequence::create(CCScaleTo::create(0.1f, 1.2f), CCScaleTo::create(0.1f, 1.0f), CCScaleTo::create(0.1f, 1.2f), CCScaleTo::create(0.1f, 1.0f), NULL));
}

// function takes comma separated string & returns vector of values
vector<int> GameGlobals::GetIntListFromString(string input)
{
	vector<int> result;
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
