/*
*   name: src/core/gameplayMain.cpp
*   origin: Citrus Engine
*   purpose: Provides the code for gameplayMain()
*   author: https://github.com/ComradeYellowCitrusFruit
*   license: LGPL-3.0-only
*/

#define _INTERNALS_ENGINE_THREAD_MAIN_
#define __CITRUS_ENGINE_SOURCE_FILE__

#include "core/os.h"

#ifdef CITRUS_ENGINE_WINDOWS
#include <Windows.h>
#endif

#include <cstring>
#include "core/input.hpp"
#include "core/extensions.h"
#include "core/errno.hpp"
#include "core/scene.hpp"
#include "core/scene_int.hpp"
#include "core/component.hpp"
#include "core/Main.hpp"
#include "core/workQueue.hpp"
#include "core/object.hpp"
#include "core/sync.hpp"
#include "core/simd.h"
#include "core/XLibglobals.hpp"

static ALWAYS_INLINE void waste50CPUCycles()
{
	asm volatile(
		// Wastes 15 cycles a piece, ideally

		// 15 cycles wasted
		"mov %r8, %r9;"
		"mov %r10, %r11;"
		"mov %rax, %rcx;"
		// 30 cycles wasted
		"mov %r8, %r9;"
		"mov %r10, %r11;"
		"mov %rax, %rcx;"
		// 45 cycles wasted
		"mov %r8, %r9;"
		"mov %r10, %r11;"
		"mov %rax, %rcx;"
		// 50 cycles wasted
		"mov %r8, %rdx;"
	);
}

namespace engine
{
	namespace internals
	{
		WorkQueue rQueue;
		Scene *curScene;
		Map<Scene, int> scenes;

		MUTEX std::atomic_uint32_t objectCtr;
		MUTEX std::atomic_uint32_t objectCnt;

		// Run the gameplay code
		NEVER_INLINE int gameplayMain()
		{
			while(true)
			{
				// TODO: Cordinate with render to yield to the OS temporarily.
				while(isRenderExecuting.load())
					spinlock_pause();
				isGameplayExecuting.store(true);

				engine::clearErrorcode();
				objectCtr.store(0);

				#ifdef CITRUS_ENGINE_WINDOWS

				// Input system stuff
				zmm_memcpy(prevInput, currentInput, 4);
				GetKeyboardState(currentInput);

				#else

				// TODO: Get the start of the current frame
				// TODO: Handle possibility of overflow
				ymm_memcpy(prevInput, currentInput, 1);
				XQueryKeymap(engine::internals::display, currentInput);

				#endif

				// We count the number of objects executed compared to the number of objects.
				// Render can use that to determine when to yield to the OS and for how long.

				objectCnt.store(objects.getCount());
				for(int i = 0; i < curScene->objects.getCount(); i++)
				{
					if(curScene->objects[i].markedForDeath)
					{
						curScene->objects[i].~Object();
						curScene->objects.rm(i);
						continue;
					}

					Vec<Component*> components = curScene->objects[i].getComponents();
					for(int j = 0; j < components.getCount(); j++)
					{
						components[j]->update();
						engine::clearErrorcode();
					}
					objectCtr++;
				}

				isGameplayExecuting.store(false);
				waste50CPUCycles();
			}
		}
	};
};
