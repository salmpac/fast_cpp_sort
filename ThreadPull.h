#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>
#include <functional>

#include "DeferredFuture.h"

class ThreadPull {
 public:
    ThreadPull(int threads_num) {
        threads.reserve(threads_num);
        for (int i = 0; i < threads_num; ++i) {
            threads.emplace_back(&ThreadPull::run, this);
        }
    }

    ~ThreadPull() {
        quit_flag = true;
        for (int i = 0; i < threads.size(); ++i) {
            new_tasks.notify_all();
            threads[i].join();
        }
    }

    void add_task(std::function<void(size_t)> callback, DefferedFuture task) {
        std::lock_guard<std::mutex> lk(queue_mutex);

        tasks.emplace_back(task, std::move(callback));
        new_tasks.notify_one();
    }

 private:
    void run() {
        while (!quit_flag.load()) {
            std::unique_lock<std::mutex> uniq_lock(queue_mutex);
            new_tasks.wait(uniq_lock, [this]()->bool { return !tasks.empty() || quit_flag; });
            if (!tasks.empty()) {
                DefferedFuture task = std::move(tasks.front().first);
                std::function<void(size_t)> callback = std::move(tasks.front().second);
                tasks.pop_front();
                uniq_lock.unlock();

                // unlock mutex before run
                task.run();
                callback(task.help_info);
            }
        }
    }

    std::condition_variable new_tasks;
    std::mutex queue_mutex;
    std::atomic_bool quit_flag;
    // function to run and callback
    std::deque<std::pair<DefferedFuture, std::function<void(size_t)>>> tasks;
    std::vector<std::thread> threads;
};