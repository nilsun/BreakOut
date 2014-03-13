#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "chipmunk.h"

typedef enum {
    kSpriteTypeBoard = 0,
    kSpriteTypeScoreBall,
    kSpriteTypeCrazyBall
}SpriteType;


class HelloWorld : public cocos2d::CCLayer
{
public:
    
    void ccTouchesMoved(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);
    
    void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);
    
    virtual bool init();

    static cocos2d::CCScene* scene();
    
    void menuCloseCallback(CCObject* pSender);
    
    void updateScore();
    
    CREATE_FUNC(HelloWorld);
    
private:
    
    int m_score;
    
    cpSpace* m_pSpace;
    
    cocos2d::CCSprite* m_boardSprite;
    
    cocos2d::CCSprite* m_ballSprite;
    
    cocos2d::CCLabelTTF* m_scoreLabel;

    void initPhysicSystem();
    
    void initLayerUIElements();
    
    void update(float delta);
    
    cocos2d::CCSprite* addBoardSprite();
    
    cocos2d::CCSprite* addCrazyBallSprite(cocos2d::CCSprite* boardSprite);
    
    void addScoreBallSpriteAtPoint(cocos2d::CCPoint point);
    
    void addSpriteToPhysicSystem(cocos2d::CCSprite* sprite, cocos2d::CCPoint point, SpriteType type);
    
    void showGameOver();
    
//    cpVect randomVelocity();
};

#endif // __HELLOWORLD_SCENE_H__
