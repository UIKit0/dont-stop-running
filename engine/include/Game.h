﻿#pragma once

#include <memory>
#include <string>

#include <SDL.h>

#include "Filesystem.h"
#include "Log.h"
#include "Renderer.h"
#include "Random.h"
#include "TextureFactory.h"
#include "Timer.h"

namespace engine {
    class Game;

    typedef std::shared_ptr<Game> GamePtr;

    class Sound;

    class Game {
    public:
        class Params {
        public:
            int screenWidth;
            int screenHeight;
            std::string windowTitle;
        };

        class IUpdateable {
        public:
            virtual void update(double timeDelta) = 0;

            virtual ~IUpdateable() {
            };
        };

        typedef std::shared_ptr<IUpdateable> IUpdateablePtr;

        class IDrawable {
        public:
            virtual void draw() = 0;

            virtual ~IDrawable() {
            };
        };

        typedef std::shared_ptr<IDrawable> IDrawablePtr;

    public:
        Game(const Params &params);

        Game(const Game &other) = delete;

        ~Game();

        bool start();

        void stop();

        SDL_Window *getWindow() const {
            return mainWindow;
        }

        int getScreenHeight() const {
            return frameBuffer->h;
        }

        int getScreenWidth() const {
            return frameBuffer->w;
        }

        bool isInitialized() const {
            return initialized;
        }

        Filesystem *getFilesystem() {
            return filesystem.get();
        }

        Renderer *getRenderer() {
            return renderer.get();
        }

        TextureFactory *getTextureFactory() {
            return imageFactory.get();
        }

        Random *getRandom() {
            return random.get();
        }

        Sound *getSound() {
            return sound.get();
        }

        int getFrameCount() const {
            return frameCount;
        }

        void startFrame();

        void endFrame();

        void update();

        void registerUpdateable(const IUpdateablePtr &updateable);

        void registerDrawable(const IDrawablePtr &drawable);

    private:
        bool initSDL();

        void stopSDL();

        bool initSDLImage();

        void stopSDLImage();

        bool initSDLTTF();

        void stopSDLTTF();

        SDL_Window *mainWindow;

        SDL_Surface *frameBuffer;

        std::unique_ptr<Random> random;

        std::unique_ptr<Renderer> renderer;

        std::unique_ptr<Sound> sound;

        std::unique_ptr<Filesystem> filesystem;

        std::unique_ptr<TextureFactory> imageFactory;

        bool initialized = false;

        int frameCount = 0;

        double lastFrameTimeDelta = 0;

        std::vector<IUpdateablePtr> registeredUpdateables;

        std::vector<IDrawablePtr> registeredDrawables;

        Timer timer;

        const Params initParams;

        Log logger;
    };

}