#include "Player.h"
#include "GameWorld.h"

Player::Player(GameWorld* game_world)
{
	game_world_ = game_world;
	size_ = CCSizeMake(50, 50);
	lives_ = 0;
	is_respawning_ = false;
}

Player::~Player()
{}

Player* Player::createWithSpriteFrameName(GameWorld* game_world, const char* frame_name)
{
	Player* player = new Player(game_world);
	if(player && player->initWithSpriteFrameName(frame_name))
	{
		player->autorelease();
		return player;
	}
	CC_SAFE_DELETE(player);
	return NULL;
}

void Player::Enter()
{
	// initially position the player below the screen
	setPosition(ccp(SCREEN_SIZE.width * 0.5, SCREEN_SIZE.height * -0.1));

	// animate the move into the screen
	CCActionInterval* movement = CCEaseBackOut::create(CCMoveTo::create(1.0f, ccp(SCREEN_SIZE.width * 0.5, SCREEN_SIZE.height * 0.1)));
	CCActionInstant* movement_over = CCCallFunc::create(this, callfunc_selector(Player::EnterFinished));
	runAction(CCSequence::createWithTwoActions(movement, movement_over));
}

void Player::EnterFinished()
{
	// player has entered, now start the game
	game_world_->StartGame();
}

void Player::Leave()
{
	// no enemies left, fly out the top edge of the screen
	CCActionInterval* movement = CCEaseBackIn::create(CCMoveBy::create(1.0f, ccp(0, SCREEN_SIZE.height)));
	runAction(movement);
}

void Player::Die()
{
	// first reduce lives
	lives_ = (--lives_ < 0) ? 0 : lives_;

	// respawn only if there are lives remaining
	is_respawning_ = (lives_ > 0);

	// animate the death :(
	CCActionInterval* death = CCSpawn::createWithTwoActions(CCFadeOut::create(0.5f), CCScaleTo::create(0.5f, 1.5f));
	// call the appropriate callback based on lives remaining
	CCActionInstant* after_death = (lives_ <= 0) ? (CCCallFunc::create(this, callfunc_selector(Player::OnAllLivesFinished))) : (CCCallFunc::create(this, callfunc_selector(Player::Respawn)));
	runAction(CCSequence::createWithTwoActions(death, after_death));

	// play a particle...a sad one :(
	CCParticleSystemQuad* explosion = CCParticleSystemQuad::create("explosion.plist");
	explosion->setAutoRemoveOnFinish(true);
	explosion->setPosition(m_obPosition);
	game_world_->addChild(explosion);

	SOUND_ENGINE->playEffect("blast_player.wav");
}

void Player::Respawn()
{
	// reset the position, opacity & scale
	setPosition(ccp(SCREEN_SIZE.width * 0.5, SCREEN_SIZE.height * 0.1));
	setOpacity(255);
	setScale(0.0f);

	// animate the respawn
	CCSpawn* respawn = CCSpawn::createWithTwoActions(CCScaleTo::create(0.5f, 1.0f), CCBlink::create(1.0f, 5));
	CCCallFunc* respawn_complete = CCCallFunc::create(this, callfunc_selector(Player::OnRespawnComplete));
	runAction(CCSequence::createWithTwoActions(respawn, respawn_complete));
}

void Player::OnRespawnComplete()
{
	is_respawning_ = false;
}

void Player::OnAllLivesFinished()
{
	// player is finally dead...for sure...game is now over
	game_world_->GameOver();
}
