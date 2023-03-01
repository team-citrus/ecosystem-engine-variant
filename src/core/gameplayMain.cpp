/*
*   name: src/core/gameplayMain.cpp
*   origin: Citrus Engine
*   purpose: Provides the code for gameplayMain()
*   author: https://github.com/ComradeYellowCitrusFruit
*   license: LGPL-3.0-only
*/

#define _INTERNALS_ENGINE_THREAD_MAIN_

#include "core/errno.hpp"
#include "core/scene.hpp"
#include "core/scene_int.hpp"
#include "core/component.hpp"
#include "core/Main.hpp"
#include "core/workQueue.hpp"
#include "core/object.hpp"
#include "core/sync.hpp"
#include "core/simd.h"

namespace engine
{
    namespace internals
    {
        workQueue rQueue;
        scene *curScene;
        map<scene, int> scenes;

        // Run the gameplay code
        int gameplayMain()
        {
            engine::clearErrorcode();

            // Physics and render lock gameplay
            while(isRenderExecuting.load()) spinlock_pause();
            while(isPhysicsExecuting.load()) spinlock_pause();

            isGameplayExecuting.store(true);

            // TODO: There is probably something we are missing here.

            for(int i = 0; i < curScene->objects.getCount(); i++)
            {
                for(int j = 0; j < curScene->objects[i].cCount; j++)
                {
                    curScene->objects[i].components[j]->update();
                    engine::clearErrorcode();
                }
            }

            executeQueue(rQueue);
            isGameplayExecuting.store(false);

            removeErrorcodeForThread();
        }
    };
};

// For Rust interop, no need to put these in a header
extern "C"
{
    // Add a function to a queue executed during engine::internals::gameplayMain()
    // @note: Do not use outside of Rust code.
    void addToQueue(void *ptr)
    {
        engine::internals::rQueue.push((engine::internals::queuePtr)ptr);
    }

    // Remove a function from a queue executed during engine::internals::gameplayMain()
    // @note: Do nut use outside of Rust code.
    void rmFromQueue(void *ptr)
    {
        engine::internals::queuePtr fnPtr = (engine::internals::queuePtr)ptr;
        for(int i = 0; i < engine::internals::rQueue.getCount(); i++)
        {
            if(engine::internals::rQueue[i] == fnPtr)
            {
                engine::internals::rQueue.rm(i);
                return;
            }
        }
    }
}