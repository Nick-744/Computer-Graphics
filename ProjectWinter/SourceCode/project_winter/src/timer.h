#ifndef TIMER_H
#define TIMER_H

#include <GLFW/glfw3.h>

class Timer
{
public:
    Timer() : active(false), finished(false), startTime(0.0) {}

    void start()
    {
        if (!active && !finished)
        {
            active    = true;
            startTime = glfwGetTime();
        }
    }

    bool hasFinished(double seconds)
    {
        if (finished) return true;
        if (!active)  return false;

        if (glfwGetTime() - startTime >= seconds)
        {
            active   = false;
            finished = true;
            return true;
        }

        return false;
    }

    void reset()
    {
        active    = false;
        finished  = false;
        startTime = 0.0;
    }

private:
    bool active;
    bool finished;
    double startTime;
};

#endif
