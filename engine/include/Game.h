﻿#pragma once

#include <memory>
#include <string>

#include <SDL.h>

#include "Filesystem.h"
#include "Input.h"
#include "Log.h"
#include "Renderer.h"
#include "Random.h"
#include "Sound.h"
#include "TextureFactory.h"
#include "Timer.h"

namespace engine {
    class Game;

    typedef std::shared_ptr<Game> GamePtr;

    class Updateable;

    class Renderable;

    class Game {
    public:
        class Params {
        public:
            int screenWidth;
            int screenHeight;
            std::string windowTitle;
        };

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

        Input *getInput() {
            return input.get();
        }

        int getFrameCount() const {
            return frameCount;
        }

        /**
        * Returns the seconds elapsed since starting the game with Game::start()
        */
        double getGameTime() const {
            return gameTimer.seconds();
        }

        void startFrame();

        void endFrame();

        void update();

        void registerUpdateable(Updateable *updateable);

        void registerRenderable(Renderable *renderable);

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

        std::unique_ptr<Input> input;

        bool initialized = false;

        int frameCount = 0;

        double lastFrameTimeDelta = 0;

        std::vector<Updateable *> registeredUpdateables;

        std::vector<Renderable *> registeredRenderables;

        Timer frameTimer;

        Timer gameTimer;

        const Params initParams;

        Log logger;
    };
}