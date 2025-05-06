#include "Task.h"

#include <cassert>
#include <functional>

class DynamicTasksGraph {
 public:
    DynamicTasksGraph(int threads_number)
    : pull(threads_number) {
    }

    // arguments like in std::thread but with callback
    template <typename Func, typename ...Args>
    size_t create_task(std::function<void()> callback, const Func& task_funcion, Args&&... args) {
        std::lock_guard<std::mutex> lk(states_mutex);
        tasks.emplace_back(callback, DefferedFuture(task_funcion, args...));
        return tasks.size() - 1;
    }

    // task "to" will depend from "from"
    void add_dependency(size_t to, size_t from) {
        std::lock_guard<std::mutex> lk(states_mutex);
        assert(tasks[to].real_planned == false);
        if (!tasks[from].completed) {
            tasks[from].notify.push_back(to);
            ++tasks[to].dependencies_count;
        }
    }

    void plan_task(size_t index) {
        std::lock_guard<std::mutex> lk(states_mutex);
        assert(tasks[index].planned == false);
        tasks[index].planned = true;
        if (tasks[index].dependencies_count == tasks[index].satisfied_dependencies) {
            real_plan_task(index);
        }
    }

 private:
    // callback assumes pull mutex is alerady unlocked
    void callback(size_t index) {
        std::unique_lock<std::mutex> lk(states_mutex);
        tasks[index].completed = true;
        for (size_t i : tasks[index].notify) {
            ++tasks[i].satisfied_dependencies;
            if (tasks[i].planned
            && tasks[i].satisfied_dependencies == tasks[i].dependencies_count) {
                real_plan_task(i);
            }
        }
        lk.unlock();
        // unlock mutex before callback
        tasks[index].callback();
    }

    //assumes states_mutex is already locked
    void real_plan_task(size_t index) {
        std::function<void(size_t)> cl = [this](size_t index) -> void {
            callback(index);
        };
        tasks[index].task.help_info = index;
        tasks[index].real_planned = true;
        pull.add_task(cl, tasks[index].task);
    }

    ThreadPull pull;
    std::mutex states_mutex;
    std::vector<Task> tasks;
};