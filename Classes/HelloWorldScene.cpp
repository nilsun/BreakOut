#include "HelloWorldScene.h"
#include "CCPhysicsSprite.h"

USING_NS_CC;

#define SCORE_BALL_MAX_ROW 4
#define SCORE_BALL_MAX_LINE 51
#define PARENT_NODE_TAG 1

using namespace cocos2d::extension;

CCScene* HelloWorld::scene()
{
    CCScene *scene = CCScene::create();
    HelloWorld *layer = HelloWorld::create();
    layer->setTouchEnabled(true);
    scene->addChild(layer);
    
    return scene;
}

bool HelloWorld::init()
{
    if (!CCLayer::init())
    {
        return false;
    }
    
    m_score = 0;
    
    initPhysicSystem();
    initLayerUIElements();
    scheduleUpdate();

    return true;
}

#pragma mark - Handle Collision

void HelloWorld::update(float delta)
{
    int steps = 2;
    float dt = CCDirector::sharedDirector()->getAnimationInterval() / (float)steps;
    for(int i = 0; i < steps; i++)
    {
        cpSpaceStep(m_pSpace, dt);
    }
    
    if (m_ballSprite->getPosition().y < 0)
    {
        showGameOver();
    }
}

void postStepRemove(cpSpace *space, cpShape *shape, void *unused)
{
	CCPhysicsSprite *sprite = (CCPhysicsSprite*)shape->data;
	cpSpaceRemoveShape(space, shape);
	cpShapeFree(shape);
	cpSpaceRemoveBody(space, sprite->getCPBody());
	cpBodyFree(sprite->getCPBody());
    sprite->removeFromParentAndCleanup(true);
}

cpBool ballCollision(cpArbiter *arb, cpSpace *space, void *data)
{
	CP_ARBITER_GET_SHAPES(arb, a, b);
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, b, NULL);
    
    HelloWorld *layer = (HelloWorld*)data;
    layer->updateScore();
    
    return true;
}

#pragma mark - Initializaiton

void HelloWorld::initPhysicSystem()
{
#if CC_ENABLE_CHIPMUNK_INTEGRATION
    
    cpInitChipmunk();
    m_pSpace = cpSpaceNew();
    m_pSpace->gravity = cpv(0, 0);
    
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    
    int wallNum = 3;
    cpShape *walls[wallNum];
    
    //bottom
//    walls[0] = cpSegmentShapeNew(m_pSpace->staticBody,
//                                    cpvzero,
//                                    cpv(visibleSize.width, 0), 0.0f);
    //top
    walls[0] = cpSegmentShapeNew(m_pSpace->staticBody,
                                    cpv(0, visibleSize.height),
                                    cpv(visibleSize.width, visibleSize.height), 0.0f);
    //left
    walls[1] = cpSegmentShapeNew(m_pSpace->staticBody,
                                    cpvzero,
                                    cpv(0, visibleSize.height), 0.0f);
    //right
    walls[2] = cpSegmentShapeNew(m_pSpace->staticBody,
                                    cpv(visibleSize.width, 0),
                                    cpv(visibleSize.width, visibleSize.height), 0.0f);
    for(int wIndex = 0; wIndex < wallNum; wIndex++)
    {
        walls[wIndex]->e = 1.0f;
        walls[wIndex]->u = 1.0f;
        cpSpaceAddStaticShape(m_pSpace, walls[wIndex]);
    }

    cpSpaceAddCollisionHandler(m_pSpace, kSpriteTypeCrazyBall, kSpriteTypeScoreBall, ballCollision, NULL, NULL, NULL, this);
    cpSpaceAddCollisionHandler(m_pSpace, kSpriteTypeCrazyBall, kSpriteTypeBoard, 0, NULL, NULL, NULL, NULL);

#endif
}

void HelloWorld::initLayerUIElements()
{
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
    
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                                          "CloseNormal.png",
                                                          "CloseSelected.png",
                                                          this,
                                                          menu_selector(HelloWorld::menuCloseCallback));
	pCloseItem->setPosition(ccp(origin.x + visibleSize.width - pCloseItem->getContentSize().width / 2 ,
                                origin.y + pCloseItem->getContentSize().height / 2));
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu, 1);
    
    m_scoreLabel = CCLabelTTF::create("Score: 0", "Arial", 40);
    m_scoreLabel->setPosition(ccp(origin.x + visibleSize.width - 120,
                                  origin.y + 80));
    this->addChild(m_scoreLabel, 1);
    
    CCNode* parentNode = CCNode::create();
    this->addChild(parentNode, 0, PARENT_NODE_TAG);
    
    m_boardSprite = addBoardSprite();
    m_ballSprite = addCrazyBallSprite(m_boardSprite);
    
    //add score balls
    float ballWidth = m_ballSprite->getContentSize().width;
    int marginCount = SCORE_BALL_MAX_LINE - 1;
    int marginWidth = visibleSize.width - SCORE_BALL_MAX_LINE * ballWidth;
    int marginBetweenBall = marginWidth / marginCount;
    auto scoreBallSprite = CCSprite::create("CrazyBall.png");
    float scoreBallWidth = scoreBallSprite->getContentSize().width;
    float scoreBallHeight = scoreBallSprite->getContentSize().width;
    
    for (int row = 0; row < SCORE_BALL_MAX_ROW; row++)
    {
        for (int line = 0; line < SCORE_BALL_MAX_LINE; line++)
        {
            float xPos = 7 + scoreBallWidth / 2 + line * (scoreBallWidth + marginBetweenBall);
            float yPos = visibleSize.height - scoreBallHeight / 2 - row * (scoreBallHeight + 2);
            addScoreBallSpriteAtPoint(ccp(xPos, yPos));
        }
    }
}

#pragma mark - Add Sprite

CCSprite* HelloWorld::addBoardSprite()
{
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
    
    auto collisionBoardSprite = CCPhysicsSprite::create("CollisionBoard.png");
    float xPos = visibleSize.width / 2 + origin.x;
    float yPos = origin.y + collisionBoardSprite->getContentSize().height / 2;
    addSpriteToPhysicSystem(collisionBoardSprite, ccp(xPos, yPos), kSpriteTypeBoard);
    
    return collisionBoardSprite;
}

CCSprite* HelloWorld::addCrazyBallSprite(cocos2d::CCSprite *boardSprite)
{
    auto crazyBallSprite = CCPhysicsSprite::create("CrazyBall.png");
    float xPos = boardSprite->getPosition().x;
    float yPos = boardSprite->getContentSize().height + crazyBallSprite->getContentSize().height / 2;
    addSpriteToPhysicSystem(crazyBallSprite, ccp(xPos, yPos), kSpriteTypeCrazyBall);
    return crazyBallSprite;
}

void HelloWorld::addScoreBallSpriteAtPoint(cocos2d::CCPoint point)
{
    auto scoreBallSprite = CCPhysicsSprite::create("CrazyBall.png");
    addSpriteToPhysicSystem(scoreBallSprite, point, kSpriteTypeScoreBall);
}

cpVect randomVelocity()
{
    int xDirection =  arc4random() % 2  == 0 ? 1 : -1;
    float xVel = 300 * xDirection - xDirection * (CCRANDOM_0_1() * 300);
    float yVel = 600;
    return cpv(xVel, yVel);
}

void HelloWorld::addSpriteToPhysicSystem(cocos2d::CCSprite* sprite, cocos2d::CCPoint point, SpriteType type)
{
    int vertsNum = 4;
    float width = sprite->getContentSize().width;
    float height = sprite->getContentSize().height;
    
    cpVect verts[] = {
        cpv(width / 2 * -1 , height / 2 * -1),
        cpv(width / 2 * -1, height / 2),
        cpv(width / 2, height / 2),
        cpv(width / 2, height / 2 * -1),
    };
    
    cpVect bodyVelocity = cpvzero;
    cpBody* body;
    cpShape* shape;
    
    switch (type)
    {
        case kSpriteTypeBoard:
        case kSpriteTypeScoreBall:
            body = cpBodyNew(INFINITY, INFINITY);
            shape = cpPolyShapeNew(body, vertsNum, verts, CGPointZero);
            break;
        case kSpriteTypeCrazyBall:
            body = cpBodyNew(1.0f, cpMomentForCircle(1.0, 10, 0, cpvzero));
            shape = cpCircleShapeNew(body, 10, cpvzero);
            bodyVelocity = randomVelocity();
            break;
    }
    body->p = cpv(point.x, point.y);
    body->v = bodyVelocity;
    cpSpaceAddBody(m_pSpace, body);
   
    shape->e = 1.0f;
    shape->u = 0.0f;
    shape->data = sprite;
    shape->collision_type = type;
    body->data = shape;
    cpSpaceAddShape(m_pSpace, shape);
    
    CCNode *parent = getChildByTag(PARENT_NODE_TAG);
    parent->addChild(sprite);
    ((CCPhysicsSprite*)sprite)->setCPBody(body);
    ((CCPhysicsSprite*)sprite)->setPosition(point);
}

#pragma mark - Handle Touch

void HelloWorld::ccTouchesMoved(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent)
{
    CCTouch *touch = (CCTouch*)pTouches->anyObject();
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint curPoint = touch->getLocation();
    CCPoint prePoint = touch->getPreviousLocation();
   
    float xOffset = curPoint.x - prePoint.x;
    float xMax = visibleSize.width - m_boardSprite->getContentSize().width / 2;
    float xMin = m_boardSprite->getContentSize().width / 2;
    float boardXPos = m_boardSprite->getPosition().x + xOffset;
    boardXPos = boardXPos < xMin ? xMin : boardXPos;
    boardXPos = boardXPos > xMax ? xMax : boardXPos;
    m_boardSprite->setPosition(ccp(boardXPos, m_boardSprite->getPosition().y));
}

void HelloWorld::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)
{
    if (m_ballSprite->getPosition().y < 0)
    {
        CCDirector* director = CCDirector::sharedDirector();
        CCScene *newScene = HelloWorld::scene();
        director->replaceScene(newScene);
    }
}

void HelloWorld::showGameOver()
{
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    m_scoreLabel = CCLabelTTF::create("Game Over ! Tap to restart ", "Arial", 50);
    m_scoreLabel->setPosition(ccp(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(m_scoreLabel, 1);
}

void HelloWorld::updateScore()
{
    m_score++;
    char str[] = "Score: %d";
    char buf[20];
    sprintf(buf, str, m_score);
    m_scoreLabel->setString(buf);
}


void HelloWorld::menuCloseCallback(CCObject* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	CCMessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
#else
    CCDirector::sharedDirector()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
#endif
}
