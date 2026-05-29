#include "AppDelegate.h"
#include "View/GameScene.h"

USING_NS_CC;

static const Size DESIGN_RES(1080, 2080);

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs() {
    GLContextAttrs attrs = {8, 8, 8, 8, 24, 8, 0};
    GLView::setGLContextAttrs(attrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    Director* director = Director::getInstance();
    GLView* glview = director->getOpenGLView();

    if (!glview) {
        glview = GLViewImpl::createWithRect("CardGame",
            Rect(0, 0, DESIGN_RES.width, DESIGN_RES.height), 0.5f);
        director->setOpenGLView(glview);
    }

    director->setDisplayStats(false);
    director->setAnimationInterval(1.0f / 60.0f);

    glview->setDesignResolutionSize(
        DESIGN_RES.width, DESIGN_RES.height, ResolutionPolicy::FIXED_WIDTH);

    FileUtils::getInstance()->addSearchPath("res");
    FileUtils::getInstance()->addSearchPath("../Resources/res");
    FileUtils::getInstance()->addSearchPath("Resources/res");

    Scene* scene = GameScene::CreateScene();
    director->runWithScene(scene);
    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();
}
