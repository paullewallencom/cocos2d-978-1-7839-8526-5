#include "GameGlobals.h"

CCSize GameGlobals::screen_size_ = CCSizeZero;

// arrays to store the frequency of formations for each skill level
const int GameGlobals::skill1_formations[] = {0, 4};
const int GameGlobals::skill2_formations[] = {4, 4, 4, 4, 1, 1, 1, 2, 2, 2};
const int GameGlobals::skill3_formations[] = {4, 4, 4, 8, 8, 1, 1, 2, 2, 5, 5, 5, 6, 6, 6, 3, 3};
const int GameGlobals::skill4_formations[] = {4, 4, 8, 8, 8, 5, 5, 5, 6, 6, 6, 3, 3, 3, 7, 7, 7};
const int GameGlobals::skill5_formations[] = {8, 8, 8, 3, 3, 3, 5, 5, 6, 6, 9, 9, 10, 10, 7, 7, 7};
const int GameGlobals::skill6_formations[] = {8, 8, 8, 5, 5, 6, 6, 9, 9, 10, 10, 7, 7, 7, 11, 11, 11};
const int GameGlobals::powerup_frequency[] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 2};

void GameGlobals::Init()
{
	// initialise pseud-random number generator
	srand(time(0));
	screen_size_ = CCDirector::sharedDirector()->getWinSize();
	LoadData();
}

void GameGlobals::LoadData()
{
	// add Resources folder to search path. This is necessary when releasing for win32
	CCFileUtils::sharedFileUtils()->addSearchPath("Resources");
	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("iutexure.plist");

	// load sound effects & background music
	SOUND_ENGINE->preloadEffect("big_blast.wav");
	SOUND_ENGINE->preloadEffect("blast_player.wav");
	SOUND_ENGINE->preloadEffect("enemy_death.wav");
	SOUND_ENGINE->preloadEffect("game_over.wav");
	SOUND_ENGINE->preloadEffect("missile.wav");
	SOUND_ENGINE->preloadEffect("shield.wav");
	SOUND_ENGINE->preloadEffect("small_blast.wav");
}

void GameGlobals::GetRegularPolygonVertices(vector<CCPoint> &vertices, int num_vertices, float circum_radius)
{
	GetRegularPolygonVertices(vertices, num_vertices, circum_radius, 0.0f);
}

void GameGlobals::GetRegularPolygonVertices(vector<CCPoint> &vertices, int num_vertices, float circum_radius, float start_angle)
{
	vertices.clear();
	float delta_theta = 2 * M_PI / num_vertices;
	float theta = start_angle;
	for(int i = 0; i < num_vertices; ++i, theta += delta_theta)
	{
		vertices.push_back(CCPoint(circum_radius * cosf(theta), circum_radius * sinf(theta)));
	}
}

int GameGlobals::GetNumVerticesForFormation(EEnemyFormation type)
{
	switch(type)
	{
	case E_FORMATION_RANDOM_EASY:
		return (int)(1 + CCRANDOM_0_1() * 5);
	case E_FORMATION_RANDOM_MEDIUM:
		return (int)(5 + CCRANDOM_0_1() * 10);
	case E_FORMATION_RANDOM_HARD:
		return (int)(10 + CCRANDOM_0_1() * 15);
	case E_FORMATION_VERTICAL_EASY:
		return 6;
	case E_FORMATION_VERTICAL_MEDIUM:
		return 10;
	case E_FORMATION_VERTICAL_HARD:
		return 14;
	case E_FORMATION_HORIZONTAL_EASY:
		return 10;
	case E_FORMATION_HORIZONTAL_MEDIUM:
		return 14;
	case E_FORMATION_HORIZONTAL_HARD:
		return 20;
	case E_FORMATION_POLYGON_EASY:
		return (5 + (int)(CCRANDOM_0_1() * 6));
	case E_FORMATION_POLYGON_MEDIUM:
		return 2 * (5 + (int)(CCRANDOM_0_1() * 5));
	case E_FORMATION_POLYGON_HARD:
		return 3 * (5 + (int)(CCRANDOM_0_1() * 4));
	}

	return 0;
}

vector<CCPoint> GameGlobals::GetEnemyFormation(EEnemyFormation type, CCRect boundary, CCPoint anchor_point)
{
	int num_vertices = GetNumVerticesForFormation(type);
	vector<CCPoint> vertices;
	vertices.clear();

	switch(type)
	{
	case E_FORMATION_RANDOM_EASY:
	case E_FORMATION_RANDOM_MEDIUM:
	case E_FORMATION_RANDOM_HARD:
		GenerateRandomFormation(vertices, num_vertices, boundary);
		break;
	case E_FORMATION_VERTICAL_EASY:
	case E_FORMATION_VERTICAL_MEDIUM:
	case E_FORMATION_VERTICAL_HARD:
		GenerateVerticalFormation(vertices, num_vertices, boundary);
		break;
	case E_FORMATION_HORIZONTAL_EASY:
	case E_FORMATION_HORIZONTAL_MEDIUM:
	case E_FORMATION_HORIZONTAL_HARD:
		GenerateHorizontalFormation(vertices, num_vertices, boundary);
		break;
	case E_FORMATION_POLYGON_EASY:
	case E_FORMATION_POLYGON_MEDIUM:
	case E_FORMATION_POLYGON_HARD:
		GeneratePolygonFormation(type, vertices, num_vertices, boundary, anchor_point);
		break;
	}

	return vertices;
}

void GameGlobals::GenerateRandomFormation(vector<CCPoint> &vertices, int num_vertices, CCRect boundary)
{
	// return random positions within the game's boundary
	for(int i = 0; i < num_vertices; ++i)
	{
		CCPoint vertex = CCPointZero;
		vertex.x = boundary.origin.x + ENEMY_RADIUS*2 + (CCRANDOM_0_1() * (boundary.size.width - ENEMY_RADIUS*4));
		vertex.y = boundary.origin.y + ENEMY_RADIUS*2 + (CCRANDOM_0_1() * (boundary.size.height - ENEMY_RADIUS*4));
		vertices.push_back(vertex);
	}
}

void GameGlobals::GenerateVerticalFormation(vector<CCPoint> &vertices, int num_vertices, CCRect boundary)
{
	// choose between left & right edge of screen
	float start_x = (CCRANDOM_MINUS1_1() > 0) ? boundary.origin.x + ENEMY_RADIUS*2 : boundary.origin.x + boundary.size.width - ENEMY_RADIUS*2;
	CCPoint start_point = CCPoint(start_x, boundary.origin.y + ENEMY_RADIUS*2);
	// calculate vertical distance between adjacent enemies
	float vertical_gap = boundary.size.height / num_vertices;
	for(int i = 0; i < num_vertices; ++i)
	{
		vertices.push_back(CCPoint(start_point.x, start_point.y + i * vertical_gap));
	}
}

void GameGlobals::GenerateHorizontalFormation(vector<CCPoint> &vertices, int num_vertices, CCRect boundary)
{
	// choose between top & bottom edge of screen
	float start_y = (CCRANDOM_MINUS1_1() > 0) ? boundary.origin.y + ENEMY_RADIUS*2 : boundary.origin.y + boundary.size.height - ENEMY_RADIUS*2;
	CCPoint start_point = CCPoint(boundary.origin.x + ENEMY_RADIUS*2, start_y);
	// calculate horizontal distance between adjacent enemies
	float horizontal_gap = boundary.size.width / num_vertices;
	for(int i = 0; i < num_vertices; ++i)
	{
		vertices.push_back(CCPoint(start_point.x + i * horizontal_gap, start_point.y));
	}
}

void GameGlobals::GeneratePolygonFormation(EEnemyFormation type, vector<CCPoint> &vertices, int num_vertices, CCRect boundary, CCPoint anchor_point)
{
	int num_polygons = 1;
	float polygon_radius = SCREEN_SIZE.width/6;
	// formations contain more polygons and are smaller as they increase in difficulty
	switch(type)
	{
	case E_FORMATION_POLYGON_EASY:
		num_polygons = 1;
		polygon_radius = SCREEN_SIZE.width/6;
		break;
	case E_FORMATION_POLYGON_MEDIUM:
		num_polygons = 2;
		polygon_radius = SCREEN_SIZE.width/7;
		break;
	case E_FORMATION_POLYGON_HARD:
		num_polygons = 3;
		polygon_radius = SCREEN_SIZE.width/8;
		break;
	}
	// calculate number of vertices for a single polygon
	int num_vertices_per_polygon = (int)(num_vertices/num_polygons);
	vector<CCPoint> vertices_per_polygon;
	vertices_per_polygon.clear();

	for(int i = 0; i < num_polygons; ++i)
	{
		// get the vertices of a single polygon
		GetRegularPolygonVertices(vertices_per_polygon, num_vertices_per_polygon, (i * ENEMY_RADIUS * 2.5f + polygon_radius));

		// copy the vertices into the final array
		for(int j = 0; j < num_vertices_per_polygon; ++j)
		{
			// the anchor point will act as the center of the polygon
			CCPoint vertex = ccpAdd(vertices_per_polygon[j], anchor_point);
			// exclude any vertices that are out of the boundary
			if(RECT_CONTAINS_CIRCLE(boundary, vertex, ENEMY_RADIUS*2))
			{
				vertices.push_back(vertex);
			}
		}

		vertices_per_polygon.clear();
	}
}
