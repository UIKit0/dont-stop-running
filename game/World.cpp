#include "World.h"

using namespace engine;

class RotatingSpriteRenderable : Renderable {
public:
    void render(Renderer *renderer) {
    }

    Vec2 position;
    double rotation;
    std::unique_ptr<Texture> sprite;
};

World::World(GamePtr game, TileEnginePtr tileEngine) :
        tileEngine(tileEngine),
        game(game),
        logger("Game") {
    runner = std::unique_ptr<Runner>{new Runner{this}};

    game->registerUpdateable(runner.get());

    background = game->getTextureFactory()->loadImage("data/bg.png");
    backgroundFar = game->getTextureFactory()->loadImage("data/bg_far.png");
    backgroundMid = game->getTextureFactory()->loadImage("data/bg_mid.png");
    backgroundNear = game->getTextureFactory()->loadImage("data/bg_near.png");
    sawTexture = game->getTextureFactory()->loadImage("data/saw.png");

    stepSoundSamples.push_back(game->getSound()->loadSample("data/footstep.wav"));
    stepSoundSamples.push_back(game->getSound()->loadSample("data/footstep2.wav"));

    game->getSound()->setSampleVolume(stepSoundSamples[0].get(), 1.0);
    game->getSound()->setSampleVolume(stepSoundSamples[1].get(), 1.0);

    // add initial block
    createBlock(0, 2048, 256);

    runner->setPosition(Vec2{160, 256 - runner->getSize().y});

    runnerTrail = std::unique_ptr<RunnerTrail>(new RunnerTrail{this});
};

void World::render(Renderer *renderer) {
    static Vec2 sawAccel{0, 0};
    static Vec2 sawPos{100, 100};

    drawBackground();

    // draw test saw
    renderer->setTextureAnchor(Renderer::TextureAnchor::CENTER);

    sawAccel = (mousePosition - sawPos) * 0.1;

    sawPos += sawAccel;

    renderer->drawTexture(sawTexture.get(), sawPos, game->getGameTime() * 300);

    renderer->setTextureAnchor(Renderer::TextureAnchor::TOP_LEFT);

    drawBlocks();

    renderer->drawTexture(runnerTrail->getTrailTexture(), Vec2{0, 0});

    drawRunner();

    drawStats({10, 10});
}

void World::update(double timeDelta) {

    if (game->getInput()->keyIsDown(Key::SPACE) || game->getInput()->mouseButtonIsDown(MouseButton::LEFT)) {
        runner->addJumpForce();
    }

    if (runner->isInAir()) {
        stats.timeInAir += timeDelta;
    } else {
        stepPlayTime += timeDelta;
    }

    position.x = runner->getPosition().x - 160;

    runnerTrail->update(timeDelta);

    stats.metersRan = runner->getPosition().x / PIXELS_PER_METER;

#if 0
    if (stepPlayTime > stepPeriod) {
        stepPlayTime -= stepPeriod;
        stepPeriod = game->getRandom()->nextInt(150, 180) / 1000.0;

        game->getSound()->playSampleOnce(stepSoundSamples[currentStepSample].get());

        currentStepSample += 1;
        if (currentStepSample >= stepSoundSamples.size()) {
            currentStepSample = 0;
            std::random_shuffle(begin(stepSoundSamples), end(stepSoundSamples));
        }
    }
#endif

    if (shouldGenerateNewBlock()) {
        generateNewBlock();
    }

}

void World::drawBlocks() {
    for (const auto &block : blocks) {
        drawBlock(block);
    }
}

void World::drawRunner() {
    auto runnerPosition = runner->getPosition();
    auto runnerSize = runner->getSize();

    // render runner
    game->getRenderer()->setColor(0x601030FF);
    game->getRenderer()->fillRect(Rect2(runner->getPosition().x - position.x, runnerPosition.y, runnerSize.x, runnerSize.y));
}

void World::drawBackground() {
    static const double BG_FAR_SPEED = 4.0;
    static const double BG_MID_SPEED = 2.4;
    static const double BG_NEAR_SPEED = 1.2;

    game->getRenderer()->drawTexture(background.get(), Vec2{0, 0});

    drawTilingBackgroundTexture(backgroundFar.get(), -position.x / BG_FAR_SPEED);
    drawTilingBackgroundTexture(backgroundMid.get(), -position.x / BG_MID_SPEED);
    drawTilingBackgroundTexture(backgroundNear.get(), -position.x / BG_NEAR_SPEED);
}

inline void World::drawBlock(Rect2 const &block) {
    // Skip invisible blocks
    if (block.x + block.w - position.x < 0) return;

    double screenX = block.x - position.x;

    for (int i = 0; i < block.w / TILE_SIZE; ++i) {
        tileEngine->drawTile(screenX + i * TILE_SIZE, block.y, GRASS_TOP);
    }

    for (int i = 0; i < block.w / TILE_SIZE; ++i) {
        for (int j = 1; j < 100; ++j) {
            if (j * TILE_SIZE + block.y > game->getScreenHeight()) break;
            tileEngine->drawTile(screenX + i * TILE_SIZE, block.y + j * TILE_SIZE, GROUND);
        }
    }
}

void World::drawStats(const Vec2 &position) {
    std::stringstream stringStream;

    stringStream << "Number of jumps: " << stats.numberOfJumps << "\n";
    game->getRenderer()->drawText(position, stringStream.str());

    stringStream.str("");
    stringStream << "Meters ran:  " << (int) (stats.metersRan * 10) / 10.0 << "\n";
    game->getRenderer()->drawText(position + Vec2{0, 20}, stringStream.str());
}

void World::createBlock(int left, int right, int top) {
    addBlock(Rect2(left, top, right, 800));
}

double World::obstacleDistance(Vec2 position) {
    double minDistance = 1000;

    for (auto &block : blocks) {
        if (block.topLeft().y < position.y && block.topLeft().x > position.x) {
            minDistance = std::min(minDistance, block.topLeft().x - position.x);
        }
    }

    return minDistance;
}

double World::floorPosition(Vec2 position) {
    for (auto &block : blocks) {
        if (block.topLeft().x < position.x && block.topRight().x > position.x) {
            return block.topLeft().y;
        }
    }

    return 1000;
}

Runner *World::getRunner() {
    return runner.get();
}

Stats &World::getStats() {
    return stats;
}

void World::displayConstants() {
//    std::cout << "Gravity:             " << runner->gravity << std::endl;
//    std::cout << "Jump start velocity: " << runner->longJumpStartVelocity << std::endl;
//    std::cout << "Speed:               " << runner->velocity.x << std::endl;
}

void World::generateNewBlock() {
    using namespace engine;

    Random *random = game->getRandom();
    Rect2 lastBlock = blocks[blocks.size() - 1];
    double lastBlockOriginalHeight = blockOriginalY[blocks.size() - 1];

    double left = lastBlock.x + lastBlock.w;

    int r1 = random->nextInt(0, 100);
    if (r1 > 30) {
        left += random->nextInt(100, 150);
    }

    double top = std::max(TOP_MIN, lastBlockOriginalHeight + random->nextInt(6) * 20 - 60);
    double width = random->nextInt(6, 12) * TILE_SIZE;

    addBlock(Rect2{left, top, width, 100.0});
}

bool World::shouldGenerateNewBlock() {
    Rect2 lastBlock = blocks[blocks.size() - 1];
    return lastBlock.x + lastBlock.w - position.x < game->getScreenWidth() + 50;
}

void World::addBlock(const Rect2 &block) {
    blocks.push_back(block);

    blockOriginalY.push_back(block.y);

    blockTimeOffset.push_back(game->getRandom()->nextInt(100) / 10.0);

    blockVelocity.push_back(0.005 + game->getRandom()->nextInt(20) / 20000.0);
}

void World::drawTilingBackgroundTexture(Texture *texture, double offset) {
    int roundedX = (int) offset % texture->getWidth();

    if (roundedX > 0) {
        roundedX -= texture->getWidth();
    }

    Rect2 src{std::abs(roundedX), 0,
            std::min(game->getScreenWidth(), texture->getWidth() + roundedX),
            texture->getHeight()};
    Vec2 dst{0, 0};

    while (dst.x < game->getScreenWidth()) {
        game->getRenderer()->drawTexture(texture, dst, &src, 0);
        dst.x += src.w;

        src.x = (int) (src.x + src.w) % texture->getWidth();
        src.w = std::min((int) (game->getScreenWidth() - dst.x), texture->getWidth());
    }
}

engine::Game *World::getGame() {
    return game.get();
}

engine::Vec2 World::getCameraPosition() {
    return position;
}
