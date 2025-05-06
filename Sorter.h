#include "DynamicTasksGraph.h"
#include <cassert>
#include <functional>

const int sorter_block_size = 1000000;

template<typename Tp>
class Sorter {
 public:
    // threads_number_ is number of threads excluding main thread
    // i mean actually there is one more
    static void sort(std::vector<Tp>& to_sort, int threads_number_) {
        assert(threads_number_ >= 1);
        Sorter(to_sort, threads_number_);
    }

 private:
    Sorter(std::vector<Tp>& to_sort, int threads_number_)
    : graph(threads_number_) {    
        threads_number = threads_number_;
        m_sort(to_sort);    
    }

    ~Sorter() {
    }

    void sort_part(Tp* help, size_t L, size_t R) {
        std::sort(help + L, help + R);
    }

    void finalize_merge(Tp* srt, std::vector<Tp>& help, size_t L1, size_t R1, size_t L2, size_t R2) {
        copy(help.begin() + L1, help.begin() + R2, srt + L1);
    }

    void merge_two(Tp* srt, std::vector<Tp>& help, size_t L1, size_t R1, size_t L2, size_t R2, size_t L) {
        size_t nxt = L;
        while (L1 < R1 || L2 < R2) {
            if (L1 == R1) {
                help[nxt++] = srt[L2++];
            } else if (L2 == R2) {
                help[nxt++] = srt[L1++];
            } else if (srt[L1] <= srt[L2]) {
                help[nxt++] = srt[L1++];
            } else {
                help[nxt++] = srt[L2++];
            }
        }
    }

    void smart_merge(Tp* srt, std::vector<Tp>& help, size_t L1, size_t R1, size_t L2, size_t R2, size_t L, size_t R, size_t fin_merge) {
        if (R2 - L2 + R1 - L1 <= sorter_block_size || L2 == R2 || L1 == R1) {
            merge_two(srt, help, L1, R1, L2, R2, L);
            return;
        }
        if (R1 - L1 < R2 - L2) {
            std::swap(L1, L2);
            std::swap(R1, R2);
        }
        size_t md = (R1 + L1) / 2;
        size_t bL = L2;
        size_t rzd = R2;
        while (bL + 1 != rzd) {
            size_t mid = (bL + rzd) / 2;
            if (srt[mid] >= srt[md]) {
                rzd = mid;
            } else {
                bL = mid;
            }
        }
        // [L1 md) [L2, rzd) -> [L, L + md - L1 + rzd - L2)
        // [md, R1) [rzd, R2) -> [L + md - L1 + rzd - L2)
        std::function<void()> do_nothing = []() -> void {};
        size_t merge1 = graph.create_task(do_nothing, &Sorter<Tp>::smart_merge, this,
            srt, std::ref(help), L1, md, L2, rzd, L, L + md - L1 + rzd - L2, fin_merge);
        size_t merge2 = graph.create_task(do_nothing, &Sorter<Tp>::smart_merge, this,
            srt, std::ref(help), md, R1, rzd, R2, L + md - L1 + rzd - L2, R, fin_merge);

        graph.add_dependency(fin_merge, merge1);
        graph.add_dependency(fin_merge, merge2);
        graph.plan_task(merge1);
        graph.plan_task(merge2);
    }

    int64_t merge_h(Tp* srt, std::vector<Tp>& help, size_t L, size_t R, size_t cnt, size_t n) {
        if (L + 1 == R) {
            return -1;
        }
        size_t mid = (L + R) / 2;

        std::function<void()> do_nothing = []() -> void {};

        int64_t task_left = merge_h(srt, help, L, mid, cnt, n);
        int64_t task_right = merge_h(srt, help, mid, R, cnt, n);

        size_t fin_merge = graph.create_task(do_nothing, &Sorter<Tp>::finalize_merge,  this, 
            srt, std::ref(help), L * cnt, mid * cnt, mid * cnt, std::min(n, R * cnt));

        size_t start_smart_merge = graph.create_task(do_nothing, &Sorter<Tp>::smart_merge, this,
            srt, std::ref(help), L * cnt, mid * cnt, mid * cnt, std::min(n, R * cnt), L * cnt, std::min(n, R * cnt), fin_merge);

        graph.add_dependency(fin_merge, start_smart_merge);
        if (task_left != -1) {
            graph.add_dependency(start_smart_merge, task_left);
        }
        if (task_right != -1) {
            graph.add_dependency(start_smart_merge, task_right);
        }
        graph.plan_task(start_smart_merge);
        graph.plan_task(fin_merge);

        return fin_merge;
    }

    void m_sort(std::vector<Tp>& x) {
        size_t n = x.size();
        Tp* help = new Tp[n];
        copy(x.begin(), x.end(), help);
        size_t cnt = (n + threads_number - 1) / threads_number;
        std::vector<std::thread> threads;
        for (size_t i = 0; i < threads_number; ++i) {
            threads.emplace_back(&Sorter::sort_part, this, help, i * cnt, std::min(n, i * cnt + cnt));
        }
        for (size_t i = 0; i < threads_number; ++i) {
            threads[i].join();
        }
        ready = false;
        int64_t last_task = merge_h(help, x, 0, threads_number, cnt, n);

        if (last_task != -1) {
            std::function<void()> cl = [this]() -> void {
                ready.store(true);
            };
            std::function<void()> dn = []()-> void {};
            size_t tsk = graph.create_task(cl, dn);
            graph.add_dependency(tsk, last_task);
            graph.plan_task(tsk);
            while (!ready.load()) {
            }
        }

        x.clear();
        x.insert(x.end(), std::make_move_iterator(help), std::make_move_iterator(help + n));
        delete[] help;
    }
    
    int threads_number;
    DynamicTasksGraph graph;
    std::atomic_bool ready;
};