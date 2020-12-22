#include "Cat.h"
#include "GameWorld.h"

Cat::~Cat()
{}

Cat* Cat::create(GameWorld* game_world)
{
	Cat* cat = new Cat();
	if(cat && cat->init(game_world))
	{
		cat->autorelease();
		return cat;
	}
	CC_SAFE_DELETE(cat);
	return NULL;
}

bool Cat::init(GameWorld* game_world)
{
	if(!GameObject::init(game_world, "cat.png"))
		return false;

	return true;
}

void Cat::Update()
{
	// no processing if invisible
	if(!m_bVisible)
		return;

	// call parent class' Update
	GameObject::Update();

	// if maximum impulses on cat exceed a threshold, its time to go!
	if(max_impulse_ > 10.0f)
	{
		Die();
	}
}

void Cat::Die()
{
	// sorry, you can't die more than once!
	if(is_dying_)
	{
		return;
	}

	is_dying_ = true;

	// animate the death & call function Dead afterwards
	CCActionInterval* die = CCEaseBackIn::create(CCScaleTo::create(0.5f, 0.0f));
	CCActionInterval* wait = CCDelayTime::create(2.0f);
	CCActionInstant* dead = CCCallFunc::create(this, callfunc_selector(Cat::Dead));
	runAction(CCSequence::create(die, wait, dead, NULL));
}

void Cat::Dead()
{
	// reposition the body out of the screen
	body_->SetTransform(b2Vec2(SCREEN_TO_WORLD(-SCREEN_SIZE.width * 0.5f), 0), 0);
	// deactivate the body so processing is stopped
	body_->SetActive(false);
	// hide the sprite so Update is stopped
	setVisible(false);

	// inform GameWorld about dying
	game_world_->CatHasDied();
}