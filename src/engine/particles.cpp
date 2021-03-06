#include "particles.h"

#include <bx/rng.h>
#include <bits/algorithmfwd.h>

#include "random.h"
#include "asset_loader.h"
#include "assets.h"

namespace dsr {
    static U32RNG sRNG;

    Particle ParticleSystem::sParticlePool[dsr::kParticleLimit];

    void Particle::update(S64 dt) {
        if (lifeTime <= 0) {
            return;
        }

        if (currentLife >= lifeTime) {
            alpha.current = 0;
            return;
        }

        F32 t = 1.0f - (lifeTime - currentLife) / (F32) lifeTime;

        // update position
        speed.update(t);
        glm::vec2 offset;
        offset = direction * speed.current;
        position += offset;

        rotation.update(t);

        scale.update(t);

        alpha.update(t);

        color.update(t);

        currentLife += dt;
    }

    void ParticleGenerator::update(S64 /*dt*/) {
        // todo: use spawn rate
        spawnParticle();
        spawnParticle();
        spawnParticle();
        spawnParticle();
        spawnParticle();
    }

    void ParticleGenerator::spawnParticle() {
        lastSpawnTime = dsr::TimeUnit::counter();

        if (parentSystem == nullptr) {
            return;
        }

        Particle *p = parentSystem->allocateParticle();

        if (p == nullptr) {
            return;
        }

        p->currentLife = 0;

        // generate lifetime
        p->lifeTime = params.lifetime.distribute(bx::frnd(&sRNG));

        // todo: use generator spawn area
        // generate position
        p->position = glm::vec2(bx::frnd(&sRNG), bx::frnd(&sRNG));
        p->position *= generatorSpawnRadius;
        p->position += generatorPosition;

        // generate direction
        F32 angle = generatorSpawnArc * (bx::frnd(&sRNG) - 0.5f);
        F32 s = std::sin(angle);
        F32 c = std::cos(angle);
        p->direction.x = generatorSpawnDirection.x * c - s * generatorSpawnDirection.y;
        p->direction.y = generatorSpawnDirection.y * c + s * generatorSpawnDirection.x;

        p->direction = glm::normalize(p->direction);

        // generate speed
        p->speed.start = params.startSpeed.distribute(bx::frnd(&sRNG));
        p->speed.end = params.endSpeed.distribute(bx::frnd(&sRNG));

        // generate scale
        p->scale.start = params.startScale.distribute(bx::frnd(&sRNG));
        p->scale.end = params.endScale.distribute(bx::frnd(&sRNG));

        // generate rotation
        p->rotation.start = params.startRotation.distribute(bx::frnd(&sRNG));
        p->rotation.end = params.endRotation.distribute(bx::frnd(&sRNG));

        // generate alpha
        p->alpha.start = params.startAlpha.distribute(bx::frnd(&sRNG));
        p->alpha.end = params.endAlpha.distribute(bx::frnd(&sRNG));

        // generate color
        p->color.start = params.startColor.distribute(bx::frnd(&sRNG));
        p->color.end = params.endColor.distribute(bx::frnd(&sRNG));
        p->color.interpolation = lerpColor;
    }

    void ParticleSystem::addGenerator(ParticleGenerator *generator) {
        generator->setParentSystem(this);
        generators.push_back(generator);
    }

    void ParticleSystem::removeGenerator(ParticleGenerator &generator) {
        // todo
    }

    void ParticleSystem::update(S64 dt) {
        // update generators
        for (auto &generator : generators) {
            generator->update(dt);
        }

        // update particles
        for (U32 i = 0; i < kParticleLimit; ++i) {
            sParticlePool[i].update(dt);
        }
    }

    void ParticleSystem::fillInstanceBuffer() {
        U8 stride = sizeof(F32) * 12;
        instanceDataBuffer = bgfx::allocInstanceDataBuffer(kParticleLimit, stride);
        U8 *data = instanceDataBuffer->data;
        for (U32 i = 0; i < kParticleLimit; ++i) {
            F32 *fdata = (F32 *) data;

            // i_data0
            // position
            fdata[0] = sParticlePool[i].position.x;
            fdata[1] = sParticlePool[i].position.y;
            fdata[2] = 0.0;
            fdata[3] = 0.0;     // unused

            // i_data1
            // color
            fdata[4] = redChannel(sParticlePool[i].color.current) / 255.0f;
            fdata[5] = greenChannel(sParticlePool[i].color.current) / 255.0f;
            fdata[6] = blueChannel(sParticlePool[i].color.current) / 255.0f;
            fdata[7] = sParticlePool[i].alpha.current;

            // i_data2
            fdata[8] = sParticlePool[i].scale.current; // scale
            fdata[9] = sParticlePool[i].rotation.current; // rotation
            fdata[10] = 0.0; // unused
            fdata[11] = 0.0; // unused

            data += stride;
        }
    }

    void ParticleSystem::render() {
        static F32 sVertexData[] = {-1, 1, 0, -1, -1, 0, 1, -1, 0, 1, 1, 0};
        static U16 sIndexData[] = {0, 1, 2, 0, 2, 3};

        bgfx::VertexDecl vertexDecl;
        vertexDecl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .end();

        auto vertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(sVertexData, sizeof(sVertexData)), vertexDecl);
        auto indexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(sIndexData, sizeof(sIndexData)));
        fillInstanceBuffer();

        bgfx::setProgram(Assets::instance().findShader("particles")->getHandle());
        bgfx::setVertexBuffer(vertexBufferHandle);
        bgfx::setIndexBuffer(indexBufferHandle);
        bgfx::setInstanceDataBuffer(instanceDataBuffer);
        bgfx::setState(BGFX_STATE_RGB_WRITE |
                BGFX_STATE_ALPHA_WRITE |
                BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_DST_ALPHA) |
                BGFX_STATE_BLEND_EQUATION_SEPARATE(BGFX_STATE_BLEND_EQUATION_ADD, BGFX_STATE_BLEND_EQUATION_MAX));

        bgfx::destroyVertexBuffer(vertexBufferHandle);
        bgfx::destroyIndexBuffer(indexBufferHandle);

        bgfx::submit(0);
    }

    Particle *ParticleSystem::allocateParticle() {
        uint32_t freePosition = 0;

        while (freePosition < dsr::kParticleLimit) {
            Particle p = sParticlePool[freePosition];
            if (p.currentLife >= p.lifeTime) {
                return &sParticlePool[freePosition];
            }

            freePosition += 1;
        }

        return nullptr;
    }

    ParticleGenerator::ParticleGenerator() {
        params.startColor.distribution = lerpColor;
        params.endColor.distribution = lerpColor;
    }

    U32 lerpColor(U32 a, U32 b, F32 t) {
        U32 r = 0;
        r += dsr::lerp<U8>((U8) ((a >> 24) & 0xff), (U8) ((b >> 24) & 0xff), t) << 24;
        r += dsr::lerp<U8>((U8) ((a >> 16) & 0xff), (U8) ((b >> 16) & 0xff), t) << 16;
        r += dsr::lerp<U8>((U8) ((a >> 8) & 0xff), (U8) ((b >> 8) & 0xff), t) << 8;
        r += dsr::lerp<U8>((U8) ((a >> 0) & 0xff), (U8) ((b >> 0) & 0xff), t) << 0;

        return r;
    }
}