#include "MissileLauncher.h"
#include "GameWorld.h"
#include "Missile.h"
#include "Player.h"
#include "Enemy.h"

MissileLauncher* MissileLauncher::create(GameWorld* instance)
{
	MissileLauncher* missile_launcher = new MissileLauncher();
	if(missile_launcher && missile_launcher->init(instance))
	{
		missile_launcher->autorelease();
		return missile_launcher;
	}
	CC_SAFE_DELETE(missile_launcher);
	return NULL;
}

bool MissileLauncher::init(GameWorld* instance)
{
	if(!PowerUp::init(instance))
		return false;
	
	vector<CCPoint> vertices1;
	vector<CCPoint> vertices2;
	vector<CCPoint> vertices;

	// get two regular pentagons, one smaller than the other and with a slightly advance rotation
	GameGlobals::GetRegularPolygonVertices(vertices1, 5, POWERUP_ICON_INNER_RADIUS - 6, M_PI * -2/20);
	GameGlobals::GetRegularPolygonVertices(vertices2, 5, POWERUP_ICON_INNER_RADIUS, M_PI * 2/20);
	
	// run a loop to splice the pentagons together to form a star
	for(int i = 0; i < 5; ++i)
	{
		vertices.push_back(vertices1[i]);
		vertices.push_back(vertices2[i]);
	}

	// draw the star shaped polygon with yellow border
	drawPolygon(&vertices[0], 10, ccc4f(0, 0, 0, 0), 2, ccc4f(0.88235, 0.96078, 0, 1));

	return true;
}

void MissileLauncher::Update()
{
	if(!is_active_)
	{
		PowerUp::Update();
	}
}

void MissileLauncher::Activate()
{
	if(is_active_)
		return;

	PowerUp::Activate();

	// generate a target for each missile
	vector<CCPoint> target = GenerateTargets();
	// generate an initial direction vertor for each missile
	vector<CCPoint> initial_direction;
	GameGlobals::GetRegularPolygonVertices(initial_direction, 5, SCREEN_SIZE.width/4, M_PI * 2/20);

	for(int i = 0; i < 5; ++i)
	{
		// create a missile with a target, initial direction & speed
		Missile* missile = Missile::createWithTarget(game_world_, target[i], ccpMult(initial_direction[i].normalize(), MISSILE_SPEED));
		// position the missile over the launcher
		missile->setPosition(m_obPosition);
		game_world_->AddMissile(missile);
	}

	SOUND_ENGINE->playEffect("missile.wav");
	
	PowerUp::Deactivate();
}

vector<CCPoint> MissileLauncher::GenerateTargets()
{
	vector<CCPoint> target_points;
	target_points.clear();

	int targets_found = 0;

	int num_enemies = game_world_->enemies_->count();
	// loop through the first 5 enemies within GameWorld & save their positions
	for(int i = 0; i < num_enemies && targets_found < 5; ++i)
	{
		Enemy* enemy = (Enemy*)game_world_->enemies_->objectAtIndex(i);
		target_points.push_back(enemy->getPosition());
		++ targets_found;
	}

	// if less than 5 enemies were found, fill up with random positions within the boundary
	while(targets_found < 5)
	{
		target_points.push_back(CCPoint(CCRANDOM_0_1() * (game_world_->boundary_rect_.origin.x + game_world_->boundary_rect_.size.width) , CCRANDOM_0_1() * (game_world_->boundary_rect_.origin.y + game_world_->boundary_rect_.size.height)));
		++ targets_found;
	}

	return target_points;
}
