#include "Shield.h"
#include "GameWorld.h"
#include "Player.h"

Shield* Shield::create(GameWorld* instance)
{
	Shield* shield = new Shield();
	if(shield && shield->init(instance))
	{
		shield->autorelease();
		return shield;
	}
	CC_SAFE_DELETE(shield);
	return NULL;
}

bool Shield::init(GameWorld* instance)
{
	if(!PowerUp::init(instance))
		return false;
	
	// generate vertices for a hexagon
	vector<CCPoint> vertices;
	GameGlobals::GetRegularPolygonVertices(vertices, 6, POWERUP_ICON_INNER_RADIUS, M_PI/6);
	// draw a hexagon with cyan border
	drawPolygon(&vertices[0], 6, ccc4f(0, 0, 0, 0), 3, ccc4f(0, 0.96862f, 1, 1));

	return true;
}

void Shield::Update()
{
	if(!is_active_)
	{
		PowerUp::Update();
	}
	else
	{
		// after activation, shield will follow the player
		setPosition(game_world_->player_->getPosition());
		setRotation(game_world_->player_->getRotation());
	}
}

void Shield::Tick()
{
	if(is_active_)
	{
		-- shield_time_left_;

		// deactivate the shield when it's time is over
		if(shield_time_left_ <= 0)
		{
			Deactivate();
		}
		// start blinking the shield when there are just two seconds left
		else if(shield_time_left_ == 2)
		{
			CCActionInterval* blink = CCBlink::create(2.0f, 8);
			blink->setTag(SHIELD_BLINK_TAG);
			runAction(blink);
		}
	}
	else
	{
		PowerUp::Tick();
	}
}

void Shield::Activate()
{
	if(is_active_)
		return;

	// if a shield already exists on the player,
	if(game_world_->player_->GetShield())
	{
		// reset the existing shield
		game_world_->player_->GetShield()->Reset();
		// deactivate self
		Deactivate();
		removeFromParentAndCleanup(true);
	}
	// else if shield doesn't exist on the player
	else
	{
		PowerUp::Activate();

		// set the shield duration
		shield_time_left_ = SHIELD_DURATION;
		setScale(0);

		// generate & draw a bigger cyan hexagon
		vector<CCPoint> vertices;
		GameGlobals::GetRegularPolygonVertices(vertices, 6, PLAYER_RADIUS * 2.5f);
		drawPolygon(&vertices[0], 6, ccc4f(0, 0, 0, 0), 4, ccc4f(0, 0.96862f, 1, 1));

		// animate the activation & life of the shield
		runAction(CCEaseBounceOut::create(CCScaleTo::create(0.25f, 1.0f)));
		runAction(CCRepeatForever::create(CCSequence::createWithTwoActions(CCEaseSineOut::create(CCScaleTo::create(0.25f, 1.15f)), CCEaseSineOut::create(CCScaleTo::create(0.25f, 1.0f)))));

		// inform the player that it now has a shield around it
		game_world_->player_->SetShield(this);
	}

	SOUND_ENGINE->playEffect("shield.wav");
}

void Shield::Deactivate()
{
	PowerUp::Deactivate();

	if(is_active_)
	{
		// inform the player that it no longer has a shield around it
		game_world_->player_->SetShield(NULL);
	}
}


void Shield::Reset()
{
	// reset the shield duration
	shield_time_left_ = SHIELD_DURATION;
	// stop any blinking action & show the shield if it was hidden due to the blink
	stopActionByTag(SHIELD_BLINK_TAG);
	setVisible(true);
}

