#include "Terrain.h"

Terrain::Terrain()
{
	world_ = NULL;
	body_ = NULL;
	sprite_ = NULL;
	offset_x_ = 0.0f;
	first_time_ = true;
	from_key_point_ = -1;
	to_key_point_ = -1;
	prev_from_key_point_ = -1;
	prev_to_key_point_ = -1;
	num_hill_key_points_ = 0;
	num_border_vertices_ = 0;
	num_hill_vertices_ = 0;
}

Terrain::~Terrain()
{
	CC_SAFE_RELEASE_NULL(sprite_);
}

Terrain* Terrain::create(b2World* world, float start_x)
{
	Terrain* terrain = new Terrain();
	if(terrain && terrain->init(world, start_x))
	{
		terrain->autorelease();
		return terrain;
	}
	CC_SAFE_DELETE(terrain);
	return NULL;
}

bool Terrain::init(b2World* world, float start_x)
{
	if(!CCNode::init())
		return false;

	// save instance of the world
	world_ = world;

	// select between a type of stripe
	EStripeType stripe_type = (EStripeType)((int)(CCRANDOM_0_1() * (E_STRIPE_SLOPE_DOWN + 1)));
	// generate the stiped sprite
	sprite_ = GenerateStripedSprite(stripe_type , 8);
	// retain for use since we won't be adding it
	sprite_->retain();
	// setup the texture to repeat and stick to the edge
	ccTexParams tex_params;
	tex_params.minFilter = GL_LINEAR;
    tex_params.magFilter = GL_LINEAR;
	tex_params.wrapS = GL_REPEAT;
    tex_params.wrapT = GL_CLAMP_TO_EDGE;
	sprite_->getTexture()->setTexParameters(&tex_params);

	// generate the hill & its curve
	GenerateHillKeyPoints(start_x);
	GenerateBorderVertices();
	// generate a body & a fixture for the hill
	if(world_)
		CreateBody();

	// load the shader for position & texture
	setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTexture));
	return true;
}

void Terrain::Update(CCPoint penguin_position)
{
	// determine current height & minimum height
	float height = penguin_position.y;
	const float min_height = SCREEN_SIZE.height * 0.6f;
	height = (height < min_height) ? min_height : height;
	
	// scale only if penguin is above SCREEN_SIZE.height * 0.6f
	float scale = min_height / height;
	setScale(scale * 1.25f);
	// update scrolling
	SetOffsetX(penguin_position.x);

	// check if terrain has left screen
	if(from_key_point_ >= MAX_HILL_KEY_POINTS - 1 && to_key_point_ >= MAX_HILL_KEY_POINTS - 1)
	{
		// reset the old data
		Reset();
		// create a new hill a couple of screens ahead of the penguin
		init(world_, penguin_position.x + SCREEN_SIZE.width * 2);
	}
}

void Terrain::SetOffsetX(float offset_x)
{
	// update only if offset is different or if its a new game
	if (offset_x_ != offset_x || first_time_)
	{
		first_time_ = false;
		offset_x_ = offset_x;
		// leave some gap so the penguin is not stuck to the left of the screen
		setPositionX(SCREEN_SIZE.width / 8 - offset_x_ * m_fScaleX);

		// reset the drawable vertices of the hill
		ResetVertices();
	}
}

void Terrain::Reset()
{
	// release the sprite...we'll generate a new one
	CC_SAFE_RELEASE_NULL(sprite_);
	// destroy the fixture...we'll generate a new one
	body_->DestroyFixture(body_->GetFixtureList());

	// reset all varibales
	from_key_point_ = -1;
	to_key_point_ = -1;
	prev_from_key_point_ = -1;
	prev_to_key_point_ = -1;
	num_hill_key_points_ = 0;
	num_border_vertices_ = 0;
	num_hill_vertices_ = 0;
}

CCSprite* Terrain::GenerateStripedSprite(EStripeType stripe_type, int num_stripes)
{
	// create a texture that we can draw into
	CCRenderTexture* render_texture = CCRenderTexture::create(STRIPE_TEXTURE_SIZE, STRIPE_TEXTURE_SIZE);
	// begin with pure black
	render_texture->beginWithClear(0.0f, 0.0f, 0.0f, 0.0f);
	
	RenderStripes(stripe_type, num_stripes);
	RenderGradient();
	RenderHighlight();
	RenderTopBorder();
	RenderNoise();
	
	render_texture->end();
	// create a sprite out of the rendered texture & return it
	return CCSprite::createWithTexture(render_texture->getSprite()->getTexture());
}

void Terrain::RenderStripes(EStripeType stripe_type, int num_stripes)
{
	// allocate memory for the position & colour arrays
	ccVertex2F* vertices = (ccVertex2F*)malloc(sizeof(ccVertex2F) * num_stripes * 6);
	ccColor4F* colors = (ccColor4F*)malloc(sizeof(ccColor4F) * num_stripes * 6);

	// initialise variables
	int num_vertices = 0;
	float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f, dx = 0.0f, dy = 0.0f;

	// select between two colours or many colours
	bool two_colors = (CCRANDOM_MINUS1_1() > 0);
	ccColor4F color1 = GameGlobals::GetRandomColor();
	ccColor4F color2 = GameGlobals::GetRandomColor();
	ccColor4F c;

	if(stripe_type == E_STRIPE_HORIZONTAL)
	{
		// initialise variables for the horizontal stripe
		dx = 0;
		dy = (float)STRIPE_TEXTURE_SIZE / (float)num_stripes;
		
		x1 = 0;
		y1 = 0;
		
		x2 = STRIPE_TEXTURE_SIZE;
		y2 = 0;
		
		// generate position & colour for each vertex of the stripe
		for (int i = 0; i < num_stripes; ++ i)
		{
			c = two_colors ? (i%2 ? color1 : color2) : GameGlobals::GetRandomColor();

			colors[num_vertices] = c;
			vertices[num_vertices ++] = vertex2(x1, y1);
			colors[num_vertices] = c;
			vertices[num_vertices ++] = vertex2(x2, y2);
			colors[num_vertices] = c;
			vertices[num_vertices ++] = vertex2(x1, y1 + dy);
			colors[num_vertices] = c;
			vertices[num_vertices ++] = vertices[num_vertices - 2];
			colors[num_vertices] = c;
			vertices[num_vertices ++] = vertices[num_vertices - 2];
			colors[num_vertices] = c;
			vertices[num_vertices ++] = vertex2(x2, y2 + dy);

			y1 += dy;
			y2 += dy;
		}
	}
	else
	{
		// initialise variables based on type of stripe
		dx = (float)STRIPE_TEXTURE_SIZE * 2 / (float)num_stripes;
		dy = 0;
		
		x1 = -STRIPE_TEXTURE_SIZE;
		y1 = (stripe_type == E_STRIPE_SLOPE_DOWN) ? 0 : STRIPE_TEXTURE_SIZE;
		
		x2 = 0;
		y2 = (stripe_type == E_STRIPE_SLOPE_DOWN) ? STRIPE_TEXTURE_SIZE : 0;
		
		// generate position & colours for two stripes at a time
		for (int i = 0; i < num_stripes / 2; ++ i)
		{
			c = two_colors ? (i%2 ? color1 : color2) : GameGlobals::GetRandomColor();

			for(int j = 0; j < 2; ++ j)
			{
				colors[num_vertices] = c;
				vertices[num_vertices ++] = vertex2(x1 + j * STRIPE_TEXTURE_SIZE, y1);
				colors[num_vertices] = c;
				vertices[num_vertices ++] = vertex2(x1 + j * STRIPE_TEXTURE_SIZE + dx, y1);
				colors[num_vertices] = c;
				vertices[num_vertices ++] = vertex2(x2 + j * STRIPE_TEXTURE_SIZE, y2);
				colors[num_vertices] = c;
				vertices[num_vertices ++] = vertices[num_vertices - 2];
				colors[num_vertices] = c;
				vertices[num_vertices ++] = vertices[num_vertices - 2];
				colors[num_vertices] = c;
				vertices[num_vertices ++] = vertex2(x2 + j * STRIPE_TEXTURE_SIZE + dx, y2);
			}

			x1 += dx;
			x2 += dx;
		}
	}

	// we're dealing with position & colour data here
	setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionColor));
	CC_NODE_DRAW_SETUP();

	// enable position & colour attributes
	ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position | kCCVertexAttribFlag_Color);
	// pass position & colour data
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_FLOAT, GL_TRUE, 0, colors);
	// set the blend function
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	// draw it...GL_TRIANGLES style!
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)num_vertices);

	// free what we allocated on top
	free(vertices);
	free(colors);
}

void Terrain::RenderGradient()
{
	// declare arrays for position & colour data
	ccVertex2F vertices[4];
	ccColor4F colors[4];

	// gradient will be light on top & dark at the bottom
	vertices[0] = vertex2(0, 0);
	vertices[1] = vertex2(STRIPE_TEXTURE_SIZE, 0);
	vertices[2] = vertex2(0, STRIPE_TEXTURE_SIZE);
	vertices[3] = vertex2(STRIPE_TEXTURE_SIZE, STRIPE_TEXTURE_SIZE);
	colors[0] = ccc4f(0.0f, 0.0f, 0.0f, 0.0f);
	colors[1] = ccc4f(0.0f, 0.0f, 0.0f, 0.0f);
	colors[2] = ccc4f(0.0f, 0.0f, 0.0f, 0.75f);
	colors[3] = ccc4f(0.0f, 0.0f, 0.0f, 0.75f);

	// we're dealing with position & colour data here
	setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionColor));
	CC_NODE_DRAW_SETUP();

	// enable position & colour attributes
	ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position | kCCVertexAttribFlag_Color);
	// pass position & colour data
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_FLOAT, GL_FALSE, 0, colors);
	// draw it...GL_TRIANGLE_STRIP style!
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Terrain::RenderHighlight()
{
	// declare arrays for position & colour data
	ccVertex2F vertices[4];
	ccColor4F colors[4];

	// highlight will be yellowish on top & nothing at the bottom
	vertices[0] = vertex2(0, 0);
	vertices[1] = vertex2(STRIPE_TEXTURE_SIZE, 0);
	vertices[2] = vertex2(0, STRIPE_TEXTURE_SIZE/3);
	vertices[3] = vertex2(STRIPE_TEXTURE_SIZE, STRIPE_TEXTURE_SIZE/3);
	colors[0] = ccc4f(1.0f, 1.0f, 0.5f, 0.4f);
	colors[1] = ccc4f(1.0f, 1.0f, 0.5f, 0.4f);
	colors[2] = ccc4f(1.0f, 1.0f, 0.5f, 0.0f);
	colors[3] = ccc4f(1.0f, 1.0f, 0.5f, 0.0f);

	// we're dealing with position & colour data here
	setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionColor));
	CC_NODE_DRAW_SETUP();

	// enable position & colour attributes
	ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position | kCCVertexAttribFlag_Color);
	// pass position & colour data
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_FLOAT, GL_FALSE, 0, colors);
	// set the blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// draw it...GL_TRIANGLE_STRIP style!
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Terrain::RenderTopBorder()
{
	// declare arrays for position & colour data
	ccVertex2F vertices[4];
	ccColor4F colors[4];

	// semi-transparent grey border to indicate the surface of the hill
	vertices[0] = vertex2(0, 0);
	vertices[1] = vertex2(STRIPE_TEXTURE_SIZE, 0);
	vertices[2] = vertex2(0, 3);
	vertices[3] = vertex2(STRIPE_TEXTURE_SIZE, 3);
	colors[0] = ccc4f(0.3255f, 0.3255f, 0.3255f, 0.75f);
	colors[1] = ccc4f(0.3255f, 0.3255f, 0.3255f, 0.75f);
	colors[2] = ccc4f(0.3255f, 0.3255f, 0.3255f, 0.75f);
	colors[3] = ccc4f(0.3255f, 0.3255f, 0.3255f, 0.75f);

	// we're dealing with position & colour data here
	setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionColor));
	CC_NODE_DRAW_SETUP();

	// enable position & colour attributes
	ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position | kCCVertexAttribFlag_Color);
	// pass position & colour data
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_FLOAT, GL_FALSE, 0, colors);
	// set the blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// draw it...GL_TRIANGLE_STRIP style!
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Terrain::RenderNoise()
{
	// create a sprite with readymade noise
	CCSprite* noise = CCSprite::create("noise1.png");
	// set the proper blend function
	ccBlendFunc blend_func;
	blend_func.src = GL_DST_COLOR;
	blend_func.dst = GL_ZERO;
	noise->setBlendFunc(blend_func);
	// position the sprite at the centre of the texture
	noise->setPosition(ccp(STRIPE_TEXTURE_SIZE/2, STRIPE_TEXTURE_SIZE/2));
	// call visit to render the sprite...twice gives added contrast
	noise->visit();
	noise->visit();
}

void Terrain::GenerateHillKeyPoints(float start_x)
{
	// initialise variables
	num_hill_key_points_ = 0;

	float x, y, dx, dy, ny;

	// first hill key point will be a bit outside the left edge of the screen
	x = start_x - SCREEN_SIZE.width * 0.25f;
	y = 0;
	hill_key_points_[num_hill_key_points_ ++] = vertex2(x, y);

	// the first peak
	x = start_x;
	y = SCREEN_SIZE.height * 0.4f;
	hill_key_points_[num_hill_key_points_ ++] = vertex2(x, y);

	// set the minimum & range quantities
	int min_dx = 160, range_dx = 80;
	int min_dy = 60,  range_dy = 60;
	// +1 - going up, -1 - going  down
	float sign = -1; 
	// set the limits
	float max_height = SCREEN_SIZE.height * 0.5f;
	float min_height = SCREEN_SIZE.height * 0.25f;

	// first set of points
	while (num_hill_key_points_ < MAX_HILL_KEY_POINTS - 15)
	{
		dx = CCRANDOM_0_1() * range_dx + min_dx;
		x += dx;
		dy = CCRANDOM_0_1() * range_dy + min_dy;
		ny = y + dy * sign;
		if(ny > max_height) ny = max_height;
		if(ny < min_height) ny = min_height;
		y = ny;
		sign *= -1;
		hill_key_points_[num_hill_key_points_++] = vertex2(x, y);
	}

	// points that will go lower and lower
	min_height = SCREEN_SIZE.height * 0.1f;
	while (num_hill_key_points_ < MAX_HILL_KEY_POINTS - 2)
	{
		dx = CCRANDOM_0_1() * range_dx + min_dx;
		x += dx;
		dy = CCRANDOM_0_1() * range_dy + min_dy;
		ny = ( (y + dy * sign) < hill_key_points_[num_hill_key_points_ - 2].y ) ? (y + dy * sign) : (y + dy * sign * 0.5f);
		if(ny < min_height) ny = min_height;
		y = ny;
		sign *= -1;
		hill_key_points_[num_hill_key_points_++] = vertex2(x, y);
	}

	// finally a nice upward slope...the ramp to launch the penguin
	x += min_dx + range_dx * 3;
	y += min_dy + range_dy * 1.5f;
	hill_key_points_[num_hill_key_points_++] = vertex2(x, y);

	// last point will be way down below
	x += min_dx + range_dx * 1.5f;
	y = 0;
	hill_key_points_[num_hill_key_points_++] = vertex2(x, y);

	// initialise left most & right most key points
	from_key_point_ = 0;
	to_key_point_ = 0;
}

void Terrain::GenerateBorderVertices()
{
	// initialise variables
	num_border_vertices_ = 0;

	ccVertex2F p0, p1, pt0, pt1;
	p0 = hill_key_points_[0];

	for (int i = 1; i < num_hill_key_points_; ++ i)
	{
		p1 = hill_key_points_[i];

		// calculate the number of segments between adjacent key points
		int h_segments = floorf((p1.x - p0.x) / HILL_SEGMENT_WIDTH);
		// calculate delta x
		float dx = (p1.x - p0.x) / h_segments;
		// calculate delta theta
		float da = M_PI / h_segments;
		// calculate x-axis & amplitude for the cosine wave
		float ymid = (p0.y + p1.y) / 2;
		float ampl = (p0.y - p1.y) / 2;
		pt0 = p0;
		border_vertices_[num_border_vertices_++] = pt0;

		// for each segment, calculate x & y coordinate
		for (int j = 1; j < h_segments + 1; ++ j)
		{
			// x coordinate is last coordinate plus delta
			pt1.x = p0.x + j * dx;
			// y coordinate taken from the cosine wave
			pt1.y = ymid + ampl * cosf(da * j);
			border_vertices_[num_border_vertices_ ++] = pt1;
			pt0 = pt1;
		}

		p0 = p1;
	}
}

void Terrain::CreateBody()
{
	// create a body only the first time...after that only create fixture
	if(body_ == NULL)
	{
		b2BodyDef bd;
		bd.position.Set(0, 0);
		body_ = world_->CreateBody(&bd);
	}
	
	// create array for the vertices
	b2Vec2 vertices[MAX_BORDER_VERTICES];
	int num_vertices = 0;
	// loop through border_vertices_, convert screen coordinates to physics coordinates
	for (int i = 0; i < num_border_vertices_; ++ i)
	{
		vertices[num_vertices ++].Set(SCREEN_TO_WORLD(border_vertices_[i].x), SCREEN_TO_WORLD(border_vertices_[i].y));
	}

	// finish up the last two vertices to form a loop
	vertices[num_vertices ++].Set(SCREEN_TO_WORLD(border_vertices_[num_border_vertices_ - 1].x), 0);
	vertices[num_vertices ++].Set(SCREEN_TO_WORLD(border_vertices_[0].x), 0);
	
	// create the chain fixture with above vertices
	b2ChainShape shape;
	shape.CreateChain(vertices, num_vertices);
	body_->CreateFixture(&shape, 0);
}

void Terrain::ResetVertices()
{
	// calculate the area of the hill that is currently visible plus a buffer of 0.125 * screen width
	float left_side = offset_x_ - SCREEN_SIZE.width * 0.125f / m_fScaleX;
	float right_side = offset_x_ + SCREEN_SIZE.width * 1.125f / m_fScaleX;
	
	// loop to calculate the left most key point
	while (hill_key_points_[from_key_point_ + 1].x < left_side)
	{
		from_key_point_ ++;
		if (from_key_point_ > num_hill_key_points_ - 1) {
			from_key_point_ = num_hill_key_points_ - 1;
			break;
		}
	}

	// loop to calculate the right most key point
	while (hill_key_points_[to_key_point_].x < right_side)
	{
		to_key_point_ ++;
		if (to_key_point_ > num_hill_key_points_ - 1) {
			to_key_point_ = num_hill_key_points_ - 1;
			break;
		}
	}

	// only loop if visible key points have changed
	if (prev_from_key_point_ != from_key_point_ || prev_to_key_point_ != to_key_point_)
	{
		// initialise variables
		num_hill_vertices_ = 0;
		ccVertex2F p0, p1, pt0, pt1;
		p0 = hill_key_points_[from_key_point_];

		// calculate curve vertices from left most to right most key point
		for(int i = from_key_point_ + 1; i < to_key_point_ + 1; ++ i)
		{
			p1 = hill_key_points_[i];

			// calculate the number of segments between adjacent key points
			int h_segments = floorf((p1.x - p0.x) / HILL_SEGMENT_WIDTH);
			int v_segments = 1;
			// calculate delta x
			float dx = (p1.x - p0.x) / h_segments;
			// calculate delta theta
			float da = M_PI / h_segments;
			// calculate x-axis & amplitude for the cosine wave
			float ymid = (p0.y + p1.y) / 2;
			float ampl = (p0.y - p1.y) / 2;
			pt0 = p0;

			// calculate vertices for each segment
			for(int j = 1; j < h_segments + 1; ++ j)
			{
				pt1.x = p0.x + j * dx;
				pt1.y = ymid + ampl * cosf(da * j);

				// calculate vertices for two triangles...cuz we render using GL_TRIANGLE_STRIP
				for(int k = 0; k < v_segments + 1; ++ k)
				{
					hill_vertices_[num_hill_vertices_] = vertex2(pt0.x, pt0.y - (float)STRIPE_TEXTURE_SIZE / v_segments * k);
					hill_tex_coords_[num_hill_vertices_++] = vertex2(pt0.x / (float)STRIPE_TEXTURE_SIZE, (float)k / v_segments);
					hill_vertices_[num_hill_vertices_] = vertex2(pt1.x, pt1.y - (float)STRIPE_TEXTURE_SIZE / v_segments * k);
					hill_tex_coords_[num_hill_vertices_++] = vertex2(pt1.x / (float)STRIPE_TEXTURE_SIZE, (float)k / v_segments);
				}
				pt0 = pt1;
			}

			p0 = p1;
		}

		// update previous left most & right most visible key points
		prev_from_key_point_ = from_key_point_;
		prev_to_key_point_ = to_key_point_;
	}
}

void Terrain::draw()
{
#ifdef ENABLE_DEBUG_DRAW
	ccGLEnableVertexAttribs( kCCVertexAttribFlag_Position );
	kmGLPushMatrix();
	world_->DrawDebugData();
	kmGLPopMatrix();
#else

	// can't render without a sprite
	if(sprite_ == NULL)
	{
		return;
	}

	CC_NODE_DRAW_SETUP();

	// bind the texture for this node
	ccGLBindTexture2D(sprite_->getTexture()->getName());
	// enable position & colour attributes
	ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position | kCCVertexAttribFlag_TexCoords);
	// pass position & colour data
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, hill_vertices_);
	glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, hill_tex_coords_);
	// draw it...GL_TRIANGLE_STRIP style!
	glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)num_hill_vertices_);
#endif
}
