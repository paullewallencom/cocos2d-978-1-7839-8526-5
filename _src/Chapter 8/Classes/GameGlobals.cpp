#include "GameGlobals.h"

CCSize GameGlobals::screen_size_ = CCSizeZero;
int GameGlobals::level_number_ = 1;

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

	// load sprite sheet/s

	// load sound effects & background music
}

// creates a b2Body* with a rectangle shaped fixture
b2Body* GameGlobals::CreateBoxBody(b2World* world, float width, float height)
{
	b2PolygonShape shape;
	shape.SetAsBox(width * 0.5f, height * 0.5f);
	
	b2BodyDef body_def;
	body_def.type = b2_dynamicBody;
	b2Body* body = world->CreateBody(&body_def);
	body->CreateFixture(&shape, 1.0f);
	return body;
}

// creates a b2Body* with a circle shaped fixture
b2Body* GameGlobals::CreateCircleBody(b2World* world, float radius)
{
	b2CircleShape shape;
	shape.m_radius = radius;

	b2BodyDef body_def;
	body_def.type = b2_dynamicBody;
	b2Body* body = world->CreateBody(&body_def);
	body->CreateFixture(&shape, 1.0f);
	return body;
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