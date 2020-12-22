var VERT_GAP_BWN_TOWERS = 300;

function Tower(position)
{
    this.lowerSprite = null;
    this.upperSprite = null;
    this.position = position;
}

function TowerManager(gameWorld)
{
    // save reference to GameWorld
    this.gameWorld = gameWorld;
    this.screenSize = gameWorld.screenSize;
    // initialise variables
    this.towers = [];
    this.towerSpriteSize = cc.SIZE_ZERO;
    this.firstTowerIndex = 0;
    this.lastTowerIndex = 0;
}

TowerManager.prototype.init = function() {
    // record size of the tower's sprite
    this.towerSpriteSize = cc.SpriteFrameCache.getInstance().getSpriteFrame("opst_02").getOriginalSize();

    // create the first pair of towers
    // they should be two whole screens away from the dragon
    var initialPosition = cc.p(this.screenSize.width*2, this.screenSize.height*0.5);
    this.firstTowerIndex = 0;
    this.createTower(initialPosition);
    // create the remaining towers
    this.lastTowerIndex = 0;
    this.createTower(this.getNextTowerPosition());
    this.lastTowerIndex = 1;
    this.createTower(this.getNextTowerPosition());
    this.lastTowerIndex = 2;
};

TowerManager.prototype.createTower = function(position) {
    // create a new tower and add it to the array
    var tower = new Tower(position);
    this.towers.push(tower);

    // create lower tower sprite & add it to GameWorld's batch node
    tower.lowerSprite = cc.Sprite.createWithSpriteFrameName("opst_02");
    tower.lowerSprite.setPositionX(position.x);
    tower.lowerSprite.setPositionY( position.y + VERT_GAP_BWN_TOWERS * -0.5 + this.towerSpriteSize.height * -0.5 );
    this.gameWorld.spriteBatchNode.addChild(tower.lowerSprite, E_ZORDER.E_LAYER_TOWER);

    // create upper tower sprite & add it to GameWorld's batch node
    tower.upperSprite = cc.Sprite.createWithSpriteFrameName("opst_01");
    tower.upperSprite.setPositionX(position.x);
    tower.upperSprite.setPositionY( position.y + VERT_GAP_BWN_TOWERS * 0.5 + this.towerSpriteSize.height * 0.5 );
    this.gameWorld.spriteBatchNode.addChild(tower.upperSprite, E_ZORDER.E_LAYER_TOWER);
};

TowerManager.prototype.update = function(){
    var tower = null;
    for(var i = 0; i < this.towers.length; ++i)
    {
        tower = this.towers[i];
        // first update the position of the tower
        tower.position.x -= MAX_SCROLLING_SPEED;
        tower.lowerSprite.setPosition(tower.position.x, tower.lowerSprite.getPositionY());
        tower.upperSprite.setPosition(tower.position.x, tower.upperSprite.getPositionY());

        // if the tower has moved out of the screen, reposition them at the end
        if(tower.position.x < this.towerSpriteSize.width * -0.5)
        {
            this.repositionTower(i);
            // this tower now becomes the tower at the end
            this.lastTowerIndex = i;
            // that means some other tower has become first
            this.firstTowerIndex = ((i+1) >= this.towers.length) ? 0:(i+1);
        }
    }
};

TowerManager.prototype.repositionTower = function(index) {
    var tower = this.towers[index];
    // update tower's position and sprites
    tower.position = this.getNextTowerPosition();
    tower.lowerSprite.setPosition(tower.position.x, tower.position.y + VERT_GAP_BWN_TOWERS * -0.5 + this.towerSpriteSize.height * -0.5);
    tower.upperSprite.setPosition(tower.position.x, tower.position.y + VERT_GAP_BWN_TOWERS * 0.5 + this.towerSpriteSize.height * 0.5);
};

TowerManager.prototype.getNextTowerPosition = function() {
    // randomly select either above or below last tower
    var isAbove = (Math.random() > 0.5);
    var offset = Math.random() * VERT_GAP_BWN_TOWERS * 0.75;
    offset *= (isAbove) ? 1:-1;

    // new position calculated by adding to last tower's position
    var newPositionX = this.towers[this.lastTowerIndex].position.x + this.screenSize.width*0.5;
    var newPositionY = this.towers[this.lastTowerIndex].position.y + offset;

    // limit the point to stay within 30-80% of the screen
    if(newPositionY >= this.screenSize.height * 0.8)
        newPositionY -= VERT_GAP_BWN_TOWERS;
    else if(newPositionY <= this.screenSize.height * 0.3)
        newPositionY += VERT_GAP_BWN_TOWERS;

    // return the new tower position
    return cc.p(newPositionX, newPositionY);
};

TowerManager.prototype.getFrontTower = function() {
    return this.towers[this.firstTowerIndex];
};
