#ifndef _WIN32

#include <unistd.h>
#define MAIN main(int argc, char const **argv)

#else 

#include <windows.h>
#define MAIN APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) // We have to use the WinMain bullshit for GUI init in windows.

#endif

// main.cpp is an exception to the no STL rule
#include <chrono>
#include <mutex>

#ifdef __STDC_NO_THREADS__

#error The Citrus Engine requires threads.h

#endif

#include <threads.h>
#include <ctime>
#include <csetjmp>
#include <cstdlib>
#include "core/mem.hpp"
#include "core/mem_int.hpp"
#include "core/Main.hpp"
#include "core/scene.hpp"
#include "core/scene_int.hpp"
#include "core/workQueue.hpp"
#include "core/log.hpp"
#include "graphics/vkInit.hpp"
#include "graphics/initGL.hpp"

#define getTimeInMils() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

using namespace engine;
using namespace internals;

bool internals::usingVulkan;
int internals::frameRate;
jmp_buf internals::buf

size_t internals::frameDelta;
size_t internals::frameDur;

size_t internals::physics::physicsDelta;
size_t internals::physics::physicsDur;

thrd_t internals::render;
thrd_t internals::phys;
thrd_t internals::mix;
thrd_t internals::gameplay;

void waitms(size_t mils)
{
    #ifndef _WIN32

    timespec t = { 0, (long)(mils * 1000000ull) };
    nanosleep(&t, NULL); // Doesn't matter if we don't sleep long enough, waitms() is only to make sure we yield to the OS

    #else 

    Sleep(mils); // Rare instance of the Windows version being easier than the POSIX version

    #endif
}

int MAIN
{
    #ifndef _WIN32
    
    pid_t id;
    if((id = fork()) != 0)
    {
        // TODO: hide console
        int ret;
        waitpid(id, &ret, 0);
        // TODO: bring up minimal error code display
    }
    
    #else
    
    int argc;
    char **argv = CommandLineToArgvW(GetCommandLineA(), &junk);
    if(!strcmp(argv[1], "--not-crash-handler"))
    {
        char cmdline[1024];
        STARTUPINFOA sinfo;
        PROCESS_INFORMATION pinfo;
        sprintf(cmdline, "%s --not-crash-handler", arg[0]);

        if(CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &sinfo, &pinfo))
        {
            int wsocode;
            if((wsocode = WaitForSingleObject(pinfo.hProcess, INFINITE)) == WAIT_OBJECT_0)
            {
                int code;
                GetExitCodeProcess(pinfo.hProcess, &code);
                // TODO: bring up minimal error code display
            }
            else
                exit(-1);
        }
        else
        {
            // TODO: Display failure message
            exit(-1);
        }
    }
    
    #endif

    // Initalize everything
    
    initLogging();
    pool = Pool();

    // TODO: Implement settings

    if(usingVulkan)
        Vulkan::vkLoad();
    else
        OpenGL::loadGL();

    // TODO: Add GUI init, and splash screen stuff, splash screen should probably be on a different thread.

    // TODO: Load main scene

    while(true) // Outer loop is run each scene
    {
        // Physics runs on it's own internal timings, and executes some gameplay code
        thrd_create(&internals::phys, internals::physmain, NULL);
        thrd_detach(internals::phys);

        while(true) // Inner loop is run each frame
        {
            size_t frameStart = getTimeInMils();

            // Internally, each thread will handle synchronization
            // They should execute in the following order:
            // Physics  ->        -> Physics
            // Gameplay -> Render
            //          -> Mix
            // Physics locks Gameplay and render and mix
            // Gameplay locks render and mix and physics
            // If gameplay executes at the same time as render and mix bad things will happen

            // TODO: Optimize this

            thrd_create(&internals::gameplay, internals::gameplayMain, NULL);
            thrd_create(&internals::render, internals::draw, NULL);
            thrd_create(&internals::audio, internals::mix, NULL);

            thrd_detach(internals::gameplay);
            thrd_detach(internals::render);
            thrd_detach(internals::audio);

            thrd_join(internals::gameplay, NULL);
            thrd_join(internals::render, NULL);
            thrd_join(internals::audio, NULL);

            if(internals::loadNecesary) break; // If a new scene needs to load, stop handling frames.

            // Handle the waits
            internals::frameDur = getTimeInMils() - frameStart;
            waitms(
                // Approximate the time between frames and wait for that duration
                (frameDelta = (1000 - (frameDur * internals::frameRate))/internals::frameRate)
            );
        }

        thrd_join(internals::phys, NULL); // Physics will load the new scene and then terminate.
    }

    return 0;
}
