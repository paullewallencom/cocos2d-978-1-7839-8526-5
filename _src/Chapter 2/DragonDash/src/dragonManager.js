var MAX_DRAGON_SPEED = -40;
var FLAP_FORCE = 13;
var ANIMATION_ACTION_TAG = 123;
var MOVEMENT_ACTION_TAG = 121;

function DragonManager(gameWorld)
{
    // save reference to GameWorld
    this.gameWorld = gameWorld;
    this.screenSize = gameWorld.screenSize;
    // initialise variables
    this.dragonSprite = null;
    this.dragonSpeed = cc.POINT_ZERO;
    this.dragonPosition = cc.POINT_ZERO;
    this.mustApplyGravity = false;
}

DragonManager.prototype.init = function() {
    // create sprite and add to GameWorld's sprite batch node
    this.dragonSprite = cc.Sprite.createWithSpriteFrameName("dhch_1");
    this.dragonPosition = cc.p(this.screenSize.width * 0.2, this.screenSize.height * 0.5);
    this.dragonSprite.setPosition(this.dragonPosition);
    this.gameWorld.spriteBatchNode.addChild(this.dragonSprite, E_ZORDER.E_LAYER_PLAYER);

    // fetch flying animation from cache & repeat it on the dragon's  sprite
    var animation = cc.AnimationCache.getInstance().getAnimation("dragonFlying");
    var repeatedAnimation = cc.RepeatForever.create(cc.Animate.create(animation));
    repeatedAnimation.setTag(ANIMATION_ACTION_TAG);
    this.dragonSprite.runAction(repeatedAnimation);

    // create a hover movement and repeat it on the dragon's sprite
    var flySequence = cc.Sequence.create(cc.EaseSineOut.create(cc.MoveBy.create(animation.getDuration()/2, cc.p(0, 10))), cc.EaseSineOut.create(cc.MoveBy.create(animation.getDuration()/2, cc.p(0, -10))));
    var repeatedFlySequence = cc.RepeatForever.create(flySequence);
    repeatedFlySequence.setTag(MOVEMENT_ACTION_TAG);
    this.dragonSprite.runAction(repeatedFlySequence);
};

DragonManager.prototype.onGameStart = function() {
    // hover should stop once the game has started
    this.dragonSprite.stopActionByTag(MOVEMENT_ACTION_TAG);
    // gravity should be applied once the game has started
    this.mustApplyGravity = true;
};

DragonManager.prototype.update = function() {
    // calculate bounding box after applying gravity
    var newAABB = this.dragonSprite.getBoundingBox();
    newAABB.setY(newAABB.getY() + this.dragonSpeed.y);

    // check if the dragon has touched the roof of the castle
    if(newAABB.y <= this.gameWorld.castleRoof)
    {
        // stop downward movement and set position to the roof of the castle
        this.dragonSpeed.y = 0;
        this.dragonPosition.y = this.gameWorld.castleRoof + newAABB.getHeight() * 0.5;

        // dragon must die
        this.dragonDeath();
        // stop the update loop
        this.gameWorld.unscheduleUpdate();
    }
    // apply gravity only if game has started
    else if(this.mustApplyGravity)
    {
        // clamp gravity to a maximum of MAX_DRAGON_SPEED & add it
        this.dragonSpeed.y = ( (this.dragonSpeed.y + GRAVITY) < MAX_DRAGON_SPEED ) ? MAX_DRAGON_SPEED : (this.dragonSpeed.y + GRAVITY);
    }

    // update position
    this.dragonPosition.y += this.dragonSpeed.y;
    this.dragonSprite.setPosition(this.dragonPosition);
};

DragonManager.prototype.dragonFlap = function() {
    // don't flap if dragon will leave the top of the screen
    if(this.dragonPosition.y + FLAP_FORCE >= this.screenSize.height)
        return;

    // add flap force to speed
    this.dragonSpeed.y = FLAP_FORCE;

    cc.AudioEngine.getInstance().playEffect(s_Flap_mp3);
};

DragonManager.prototype.dragonDeath = function() {
    // fall miserably to the roof of the castle
    var rise = cc.EaseSineOut.create(cc.MoveBy.create(0.25, cc.p(0, this.dragonSprite.getContentSize().height)));
    var fall = cc.EaseSineIn.create(cc.MoveTo.create(0.5, cc.p(this.screenSize.width * 0.2, this.gameWorld.castleRoof)));
    // inform GameWorld that dragon is no more :(
    var finish = cc.CallFunc.create(this.gameWorld.onGameOver, this.gameWorld);
    // stop the frame based animation...dragon can't fly once its dead
    this.dragonSprite.stopAllActions();
    this.dragonSprite.runAction(cc.Sequence.create(rise, fall, finish));

    cc.AudioEngine.getInstance().playEffect(s_Crash_mp3);
};