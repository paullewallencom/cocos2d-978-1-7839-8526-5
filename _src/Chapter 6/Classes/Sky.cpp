#include "Sky.h"

Sky::~Sky()
{}

Sky* Sky::create()
{
	Sky* sky = new Sky();
	if(sky && sky->init())
	{
		sky->autorelease();
		return sky;
	}
	CC_SAFE_DELETE(sky);
	return NULL;
}

bool Sky::init()
{
	if(!CCNode::init())
		return false;

	sprite_ = NULL;
	clouds_ = NULL;
	offset_x_ = 0.0f;

	// generate a blue, noisy texture for the sky
	sprite_ = GenerateSkySprite();
	// setup the texture to repeat
	ccTexParams tex_params;
	tex_params.minFilter = GL_NEAREST;
    tex_params.magFilter = GL_NEAREST;
	tex_params.wrapS = GL_REPEAT;
    tex_params.wrapT = GL_REPEAT;
	sprite_->getTexture()->setTexParameters(&tex_params);
	sprite_->setTextureRect(CCRectMake(0, 0, (SCREEN_SIZE.width / SCREEN_SIZE.height * 1024), 1024));
	// position the sprite at the centre and add it
	sprite_->setPosition(ccp(SCREEN_SIZE.width/2, SCREEN_SIZE.height/2));
	addChild(sprite_);

	// create some clouds
	clouds_ = CCSprite::create("clouds.png");
	// setup the cloud texture to repeat
	tex_params.minFilter = GL_NEAREST;
	tex_params.magFilter = GL_NEAREST;
	tex_params.wrapS = GL_REPEAT;
	tex_params.wrapT = GL_REPEAT;
	clouds_->getTexture()->setTexParameters(&tex_params);
	clouds_->setTextureRect(CCRectMake(0, 0, (SCREEN_SIZE.width / SCREEN_SIZE.height * 1024), 256));
	// position the clouds at the top of the screen and add it
	clouds_->setPosition(ccp(SCREEN_SIZE.width/2, SCREEN_SIZE.height * 0.9f));
	addChild(clouds_);

	return true;
}

CCSprite* Sky::GenerateSkySprite()
{
	float texture_size = 1024;
	// create a texture that we can draw into
	CCRenderTexture* render_texture = CCRenderTexture::create(texture_size, texture_size);
	// begin with a nice shade of blue
	render_texture->beginWithClear(0.4549f, 0.6471f, 0.8863f, 1.0f);

	// create a sprite with readymade noise
	CCSprite* noise = CCSprite::create("noise2.png");
	// set the proper blend function
	ccBlendFunc blend_func;
	blend_func.src = GL_DST_COLOR;
	blend_func.dst = GL_ZERO;
	noise->setBlendFunc(blend_func);
	// position the sprite at the centre of the sky texture
	noise->setPosition(ccp(texture_size/2, texture_size/2));
	// scale it up to the size of the sky texture
	noise->setScale(2);
	// call visit to render the sprite
	noise->visit();
	
	render_texture->end();
	// create a sprite out of the rendered texture & return it
	return CCSprite::createWithTexture(render_texture->getSprite()->getTexture());
}

void Sky::Update(CCPoint penguin_position, float terrain_offset_x, float terrain_scale)
{
	// update the offset
	float offset_x = terrain_offset_x * 0.2f;
	if (offset_x_ != offset_x)
	{
		offset_x_ = offset_x;
		// update the texture rect to scroll the sky & clouds
		sprite_->setTextureRect(CCRectMake(offset_x_, 0, sprite_->getTextureRect().size.width, sprite_->getTextureRect().size.height));
		clouds_->setTextureRect(CCRectMake(offset_x_ * 2, 0, clouds_->getTextureRect().size.width, clouds_->getTextureRect().size.height));
	}

	// calculate new scale factor & process only if new scale factor is different
	float scale = 1.0f - (1.0f - terrain_scale) * 0.3f;
	if (m_fScaleX != scale)
	{
		// calculate minimum scale
		float min_scale = SCREEN_SIZE.height / 1024.0f;
		if (scale < min_scale)
		{
			// set scale
			sprite_->setScale(min_scale);
			clouds_->setScale(min_scale);
			// lower the clouds as you scale them down...this will give an illusion of height
			clouds_->setPositionY(SCREEN_SIZE.height * 0.9f - (1.0f - min_scale) * clouds_->getTextureRect().size.height * 2);
		}
		else
		{
			// set scale
			sprite_->setScale(scale);
			clouds_->setScale(scale);
			// lower the clouds as you scale them down...this will give an illusion of height
			clouds_->setPositionY(SCREEN_SIZE.height * 0.9f - (1.0f - scale) * clouds_->getTextureRect().size.height * 2);
		}
	}
}
