#ifndef _OBSIDIAN2D_CORE_ENGINE_
#define _OBSIDIAN2D_CORE_ENGINE_
    #define OBSIDIAN2D_VERSION 0.0.1

    #ifdef __linux__ 
        #define LIKELY(x)   __builtin_expect((x),1)
        #define UNLIKELY(x) __builtin_expect((x),0)
    #elif _WIN32
        #define LIKELY(x)   x
        #define UNLIKELY(x) x
    #endif
    

#include "Obsidian2D/Util/Time.h"

#include "Obsidian2D/Util/Logger.h"
#include "Obsidian2D/Util/Loggable.h"

#include "Obsidian2D/Core/Error.h"

#include "Obsidian2D/Core/Registry.h"
#include "Obsidian2D/Core/App.h"


#include "Obsidian2D/Renderer/window.h"

namespace Obsidian2D
{
    namespace Core
    {
        class Engine : public Loggable
        {
        private:

        protected:
            Registry registry;

        public:
            Engine()
            {

                //TODO: Make logger prefix which class/module in the hierarchy calls it

                this->registerLogCallback([](const std::string& line) {
                    Obsidian2D::Util::Logger::write("[Obsidian2D::Core::Engine] " + line);
                });

                registry.registerLogCallback([](const std::string& line) {
                    Obsidian2D::Util::Logger::write("[Obsidian2D::Core::Registry] " + line);
                });
            }

            int start(Obsidian2D::Core::App* app)
            {
                this->log("start()");

                app->registerLogCallback([](const std::string& line) {
                    Obsidian2D::Util::Logger::write("[Obsidian2D::Core::App] " + line);
                });

                //TODO: Check for consistent scenes like pause menu and stuff

                //Pre initialization
                app->init();

                /* Register GameStates/Scenes */

                /* Register routes, configs, default scene */
                Obsidian2D::Core::App::Config config = app->getConfig();

                //TODO: Create renderer base class and render blank screen
				Obsidian2D::Renderer::Window* window = new Obsidian2D::Renderer::Window(800, 600);
                
                window->registerLogCallback([](const std::string& line) {
                    Obsidian2D::Util::Logger::write("[Obsidian2D::Renderer::Window] " + line);
                });

				window->bootstrap();

                /* Init defualt scene, register objects*/

                /* Game Loop */
                while(LIKELY(false)) {
                    //Loop trought entities and execute update
                }

                this->log("Cleaning up memory");
                free(app);
				app = nullptr;

				window->destroyCommandBuffers();
				window->destroyDevice();
                window->destroyWindow();
                window->destroyInstance();
				free(window);

                return Obsidian2D::Error::None;
            }
        };
    }
}

#endif //_OBSIDIAN2D_CORE_ENGINE_