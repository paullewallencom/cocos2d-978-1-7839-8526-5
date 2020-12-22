#include "BackgroundManager.h"
#include "GameWorld.h"

bool BackgroundManager::init()
{
	if ( !CCNode::init() )
    {
        return false;
    }

	// initialise
	game_world_ = NULL;
	num_bg_pieces_ = 0;
	bg_sprite_ = NULL;
	bg_piece_size_ = CCSizeZero;

	// create & add the batch node
	batch_node_ = CCSpriteBatchNode::create("cjtexset_02.png", 20);
	addChild(batch_node_);

	// create an array to maintain pieces of the background
	bg_pieces_ = CCArray::create();
	bg_pieces_->retain();
	CreateBG();

	// create an array to maintain pieces of the walls
	wall_sprites_ = CCArray::createWithCapacity(4);
	wall_sprites_->retain();
	CreateWall();
	return true;
}

BackgroundManager::~BackgroundManager()
{
	CC_SAFE_RELEASE_NULL(bg_pieces_);
	CC_SAFE_RELEASE_NULL(wall_sprites_);
}

void BackgroundManager::CreateBG()
{
	// create the base platform's background
	bg_sprite_ = CCSprite::createWithSpriteFrameName("cjbg01.png");
	bg_sprite_->setPosition(ccp(SCREEN_SIZE.width/2, SCREEN_SIZE.height/2));
	batch_node_->addChild(bg_sprite_);

	// fill up a screen with the loopable bg pieces
	CCSprite* bg_piece = CCSprite::createWithSpriteFrameName("cjbg02.png");
	bg_piece_size_ = bg_piece->getContentSize();
	bg_piece->setPosition(ccp(SCREEN_SIZE.width/2, bg_sprite_->getPositionY() + bg_sprite_->getContentSize().height/2 + bg_piece_size_.height/2));
	bg_pieces_->addObject(bg_piece);
	batch_node_->addChild(bg_piece);

	// add enough pieces to vertically cover one and a half screens
	num_bg_pieces_ = 1 + ceil( (SCREEN_SIZE.height * 1.5f) / bg_piece_size_.height );
	float position_y = bg_piece->getPositionY();
	for(int i = 1; i < num_bg_pieces_; ++i)
	{
		bg_piece = CCSprite::createWithSpriteFrameName("cjbg02.png");
		bg_piece->setPosition(ccp(SCREEN_SIZE.width/2, position_y + bg_piece_size_.height));
		bg_pieces_->addObject(bg_piece);
		batch_node_->addChild(bg_piece);
		position_y += bg_piece_size_.height;
	}
}

void BackgroundManager::CreateWall()
{
	// set texture parameters to repeat this texture within a sprite
	// for win32, texture must be a power-of-two
	CCTexture2D* texture = CCTextureCache::sharedTextureCache()->addImage("cjsbar01.png");
	ccTexParams texture_param;
	texture_param.magFilter = GL_LINEAR;
	texture_param.minFilter = GL_LINEAR;
	texture_param.wrapS = GL_REPEAT;
	texture_param.wrapT = GL_REPEAT;
	texture->setTexParameters(&texture_param);

	// add enough pieces to vertically cover two screens
	for(int i = 0; i < 4; ++i)
	{
		CCSprite* wall_sprite = CCSprite::createWithTexture(texture, CCRectMake(0, 0, WALL_WIDTH, SCREEN_SIZE.height));
		wall_sprite->setPositionX( i >= 2 ? WALL_WIDTH/2 : SCREEN_SIZE.width - WALL_WIDTH/2 );
		wall_sprite->setPositionY( i % 2 ? SCREEN_SIZE.height * 1.5f : SCREEN_SIZE.height * 0.5f );
		wall_sprites_->addObject(wall_sprite);
		addChild(wall_sprite);
	}
}

void BackgroundManager::Update(float speed)
{
	// scroll the BG and walls
	UpdateBG(speed * 0.1f);
	UpdateWall(speed);
}

void BackgroundManager::UpdateBG(float speed)
{
	// update the base platform's background
	if(bg_sprite_)
	{
		bg_sprite_->setPositionY(bg_sprite_->getPositionY() + speed);
		// remove the base platform's background if it has left the screen
		if(bg_sprite_->getPositionY() <= bg_sprite_->getContentSize().height/-2)
		{
			bg_sprite_->removeFromParentAndCleanup(true);
			bg_sprite_ = NULL;

			// now would be a good time to also remove the base platform itself
			game_world_->RemoveBasePlatform();
		}
	}

	// update the position for all the bg pieces first
	// calculate the position of the highest bg piece
	float highest_position = 0;
	for(int i = 0; i < num_bg_pieces_; ++i)
	{
		CCSprite* bg_piece = (CCSprite*)bg_pieces_->objectAtIndex(i);
		bg_piece->setPositionY(bg_piece->getPositionY() + speed);
		highest_position = (bg_piece->getPositionY() > highest_position) ? bg_piece->getPositionY() : highest_position;
	}

	// find out which bg piece has exited the screen from the bottom
	// place that piece above the highest bg piece
	for(int i = 0; i < num_bg_pieces_; ++i)
	{
		CCSprite* bg_piece = (CCSprite*)bg_pieces_->objectAtIndex(i);
		if(bg_piece->getPositionY() <= bg_piece_size_.height*-2)
		{
			bg_piece->setPositionY(highest_position + bg_piece_size_.height);
		}
	}
}

void BackgroundManager::UpdateWall(float speed)
{
	// first update the position of all the wall pieces
	for(int i = 0; i < 4; ++i)
	{
		CCSprite* wall_sprite = (CCSprite*)wall_sprites_->objectAtIndex(i);
		wall_sprite->setPositionY(wall_sprite->getPositionY() + speed);
	}

	for(int i = 0; i < 2; ++i)
	{
		// get pointers to the a piece of the left and right wall respectively
		CCSprite* wall_sprite1 = (CCSprite*)wall_sprites_->objectAtIndex(i*2);
		CCSprite* wall_sprite2 = (CCSprite*)wall_sprites_->objectAtIndex(i*2 + 1);

		// if a wall piece has exited from below, reposition it at the top
		if(wall_sprite1->getPositionY() <= SCREEN_SIZE.height * -0.5f)
		{
			wall_sprite1->setPositionY(wall_sprite2->getPositionY() + SCREEN_SIZE.height);
		}
		// if a wall piece has exited from above, reposition it at the bottom
		else if(wall_sprite1->getPositionY() >= SCREEN_SIZE.height * 1.5f)
		{
			wall_sprite1->setPositionY(wall_sprite2->getPositionY() - SCREEN_SIZE.height);
		}
		
		// if a wall piece has exited from below, reposition it at the top
		if(wall_sprite2->getPositionY() <= SCREEN_SIZE.height * -0.5f)
		{
			wall_sprite2->setPositionY(wall_sprite1->getPositionY() + SCREEN_SIZE.height);
		}
		// if a wall piece has exited from above, reposition it at the bottom
		else if(wall_sprite2->getPositionY() >= SCREEN_SIZE.height * 1.5f)
		{
			wall_sprite2->setPositionY(wall_sprite1->getPositionY() - SCREEN_SIZE.height);
		}
	}
}