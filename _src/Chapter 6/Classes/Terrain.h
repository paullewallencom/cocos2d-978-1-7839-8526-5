#ifndef TERRAIN_H_
#define TERRAIN_H_

#include "GameGlobals.h"

class Terrain : public CCNode
{
public:
	Terrain();
	~Terrain();
	static Terrain* create(b2World* world, float start_x);
	virtual bool init(b2World* world, float start_x);

	virtual void draw();
	void Update(CCPoint penguin_position);
	void Reset();
	
	CCSprite* GenerateStripedSprite(EStripeType stripe_type, int num_stripes);
	void RenderStripes(EStripeType stripe_type, int num_sprites);
	void RenderGradient();
	void RenderHighlight();
	void RenderTopBorder();
	void RenderNoise();

	void GenerateHillKeyPoints(float start_x);
	void GenerateBorderVertices();
	void CreateBody();
	void ResetVertices();

	void SetOffsetX(float offset_x);
	inline float GetOffsetX() { return offset_x_; }
	inline ccVertex2F GetCliffKeyPoint() { return hill_key_points_[MAX_HILL_KEY_POINTS - 2]; }
	inline ccVertex2F GetFootKeyPoint() { return hill_key_points_[0]; }

private:
	b2World* world_;
	b2Body* body_;
	CCSprite* sprite_;
	float offset_x_;
	bool first_time_;

	int from_key_point_;
	int to_key_point_;
	int prev_from_key_point_;
	int prev_to_key_point_;
	int num_hill_key_points_;
	ccVertex2F hill_key_points_[MAX_HILL_KEY_POINTS];

	int num_border_vertices_;
	ccVertex2F border_vertices_[MAX_BORDER_VERTICES];

	int num_hill_vertices_;
	ccVertex2F hill_vertices_[MAX_HILL_VERTICES];
	ccVertex2F hill_tex_coords_[MAX_HILL_VERTICES];
};

#endif // TERRAIN_H_
