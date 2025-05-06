#include "ThreadPull.h"

struct Task {
    DefferedFuture task;
    std::function<void()> callback;
    std::vector<size_t> notify;
    size_t dependencies_count;
    size_t satisfied_dependencies;
    bool completed; // true if it is completed in threadpull
    bool planned; // true if it is planned in graph
    bool real_planned; // true if it is in threadpull

    Task()
    : dependencies_count(0)
    , satisfied_dependencies(0)
    , completed(false)
    , planned(false)
    , real_planned(false) {
    }

    Task(std::function<void()> callback, DefferedFuture task_)
    : task(std::move(task_))
    , callback(std::move(callback))
    , dependencies_count(0)
    , satisfied_dependencies(0)
    , completed(false)
    , planned(false)
    , real_planned(false) {
    }
};