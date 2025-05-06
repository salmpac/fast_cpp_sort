#include <future>

class DefferedFuture {
 public:
    // arguments like in std::thread
    template <typename Func, typename ...Args>
    DefferedFuture(const Func& task_funcion, Args&&... args) {
        task = std::make_shared<std::future<void> >(std::async(std::launch::deferred, task_funcion, args...));
    }

    DefferedFuture() {
    }

    void run() {
        task->get();
    }
    
    ~DefferedFuture() {
    }

    // task index
    size_t help_info;
 private:
    std::shared_ptr<std::future<void> > task;
};