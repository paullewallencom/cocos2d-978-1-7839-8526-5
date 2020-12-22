var MAX_SCROLLING_SPEED = 6;
var CASTLE_SPRITE_Y = -50;
var SILHOUETTE_SPRITE_Y = 100;
var MAX_STARS = 15;

function FairytaleManager(parent)
{
    // save reference to GameWorld
    this.parent = parent;
    this.screenSize = parent.screenSize;
    // initialise variables
    this.castleSpriteSize = cc.SIZE_ZERO;
    this.castleSprites = [];
    this.lastCastleIndex = 0;
    this.silhouetteSpriteSize = cc.SIZE_ZERO;
    this.silhouetteSprites = [];
    this.lastSilhouetteIndex = 0;
}

FairytaleManager.prototype.init = function() {
    // this makes a nice midnight sky
    var background = cc.LayerGradient.create(cc.c4b(15, 15, 25, 255), cc.c4b(84, 83, 104, 255));
    this.parent.addChild(background, E_ZORDER.E_LAYER_BG);

    this.createCastle();
    this.createSilhouette();
    this.createStars();
};

FairytaleManager.prototype.createCastle = function() {
    // record size of the castle wall sprite
    this.castleSpriteSize = cc.SpriteFrameCache.getInstance().getSpriteFrame("dhbase").getOriginalSize();
    // initial position
    var nextPosition = this.castleSpriteSize.width * 0.5;
    // fill up one & a half screen
    while(nextPosition < this.screenSize.width * 1.5)
    {
        // create castle wall sprite and add it to the parent's batch node
        var castleSprite = cc.Sprite.createWithSpriteFrameName("dhbase");
        castleSprite.setPosition(cc.p(nextPosition, CASTLE_SPRITE_Y));
        this.parent.spriteBatchNode.addChild(castleSprite, E_ZORDER.E_LAYER_CASTLE);
        // store this sprite...we need to update it
        this.castleSprites.push(castleSprite);
        // the next wall depends on this variable
        nextPosition += this.castleSpriteSize.width;
    }
    // we need this to position the next wall sprite
    this.lastCastleIndex = this.castleSprites.length-1;
};

FairytaleManager.prototype.createSilhouette = function() {
    // record size of the silhouette sprite
    this.silhouetteSpriteSize = cc.SpriteFrameCache.getInstance().getSpriteFrame("dhbush").getOriginalSize();
    // initial position
    var nextPosition = 0;
    // fill up one & a half screen
    while(nextPosition < this.screenSize.width * 1.5)
    {
        // create silhouette sprite and add it to the parent's batch node
        var silhouetteSprite = cc.Sprite.createWithSpriteFrameName("dhbush");
        silhouetteSprite.setPosition(nextPosition, SILHOUETTE_SPRITE_Y);
        this.parent.spriteBatchNode.addChild(silhouetteSprite, E_ZORDER.E_LAYER_SILHOUETTE);
        // store this sprite...we need to update it
        this.silhouetteSprites.push(silhouetteSprite);
        // the next silhouette depends on this variable
        nextPosition += this.silhouetteSpriteSize.width;
    }
    // we need this to position the next silhouette sprite
    this.lastSilhouetteIndex = this.silhouetteSprites.length-1;
};

FairytaleManager.prototype.createStars = function() {
    // random number of stars...this night sky always changes
    var numStars = MAX_STARS + Math.floor(Math.random() * MAX_STARS);
    for(var i = 0; i < numStars; ++i)
    {
        var star = null;
        // either big star or small
        if(Math.random() > 0.5)
            star = cc.Sprite.createWithSpriteFrameName("dhstar1");
        else
            star = cc.Sprite.createWithSpriteFrameName("dhstar2");

        // random position
        var position = cc.p(Math.random() * this.screenSize.width, Math.random() * this.screenSize.height);
        star.setPosition(position);
        // twinkle twinkle randomly star
        var duration = 1 + Math.random() * 2;
        var action = cc.RepeatForever.create(cc.Sequence.create(cc.DelayTime.create(duration*2), cc.FadeOut.create(duration), cc.FadeIn.create(duration)));
        star.runAction(action);
        // add this too the batch node as well
        this.parent.spriteBatchNode.addChild(star);
    }
};

FairytaleManager.prototype.update = function() {
    this.updateCastle();
    this.updateSilhouette();
};

FairytaleManager.prototype.updateCastle = function(){
    for(var i = 0; i < this.castleSprites.length; ++i)
    {
        // first update the position based on the scroll speed
        var castleSprite = this.castleSprites[i];
        castleSprite.setPosition(castleSprite.getPositionX() - MAX_SCROLLING_SPEED, castleSprite.getPositionY());

        // check if the sprite has gone completely out of the left edge of the screen
        if(castleSprite.getPositionX() < (this.castleSpriteSize.width * -0.5))
        {
            // reposition it after the last wall sprite
            var positionX = this.castleSprites[this.lastCastleIndex].getPositionX() + this.castleSpriteSize.width - MAX_SCROLLING_SPEED;
            castleSprite.setPosition(positionX, castleSprite.getPositionY());
            // this sprite now becomes the new last wall
            this.lastCastleIndex = i;
        }
    }
};

FairytaleManager.prototype.updateSilhouette = function(){
    for(var i = 0; i < this.silhouetteSprites.length; ++i)
    {
        // first update the position based on the scroll speed
        var silhouetteSprite = this.silhouetteSprites[i];
        silhouetteSprite.setPosition(this.silhouetteSprites[i].getPositionX() - MAX_SCROLLING_SPEED*0.3, silhouetteSprite.getPositionY());

        // check if the sprite has gone completely out of the left edge of the screen
        if(silhouetteSprite.getPositionX() < (this.silhouetteSpriteSize.width * -0.5))
        {
            // reposition it after the last silhouette sprite
            var positionX = this.silhouetteSprites[this.lastSilhouetteIndex].getPositionX() + this.silhouetteSpriteSize.width - MAX_SCROLLING_SPEED*0.3;
            silhouetteSprite.setPosition(positionX, silhouetteSprite.getPositionY());
            // this sprite now becomes the new last silhouette
            this.lastSilhouetteIndex = i;
        }
    }
};