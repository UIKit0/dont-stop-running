#pragma once

#include <Engine.h>
#include <Entity.h>
#include <Timer.h>
#include <Physics.h>
#include <Vec2.h>

class World;

class Runner : public engine::Entity {
public:
    explicit Runner(World *world) :
            velocity{410, 0},
            isJumping(false),
            world(world),
            size(20, 40) {
    }

    void update(double delta) override;

    engine::Vec2 getPosition() const {
        return position;
    }

    engine::Vec2 getSize() const {
        return size;
    }

    void addJumpForce();

    void increaseSpeed() {
        velocity.x += 2.0;
    }

    void decreaseSpeed() {
        velocity.x -= 2.0;
    }

    void increaseGravity() {
        gravity += 10.0;
    }

    void decreaseGravity() {
        gravity -= 10.0;
    }

    void increaseJumpVelocity() {
        longJumpStartVelocity += -10.0;
    }

    void decreaseJumpVelocity() {
        longJumpStartVelocity -= -10.0;
    }

    void setSpeed(double speed) {
        velocity.x = speed;
    }

    void setGravity(double gravity) {
        this->gravity = gravity;
    }

    void setJumpVelocity(double velocity) {
        longJumpStartVelocity = velocity;
    }

    void setPosition(const engine::Vec2 &position) {
        this->position = position;
    }

    bool isInAir();

private:
    void startJump();

    void resetJump();

    double getDistanceToGround();

private:
    double gravity = 3000.0;

    double longJumpStartVelocity = -700.0;

    double shortJumpStartVelocity = -200.0;

    engine::Vec2 velocity;

    bool canJump = true;

    bool isJumping = false;

    engine::Timer jumpTimer;

    World *world;

    engine::Vec2 position;

    engine::Vec2 size;
};
