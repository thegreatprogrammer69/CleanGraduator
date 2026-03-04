#ifndef CLEANGRADUATOR_THREADWORKER_H
#define CLEANGRADUATOR_THREADWORKER_H

#include <thread>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <atomic>

namespace utils::thread {

    class ThreadWorker
    {
    public:
        using Task = std::function<void()>;

        explicit ThreadWorker(Task task);
        ~ThreadWorker();

        void start();
        void stop();
        void pause();
        void resume();

        bool isRunning() const noexcept;

    private:
        void threadLoop();

    private:
        Task m_task;
        std::thread m_thread;

        std::atomic<bool> m_running{false};

        // Защищены m_mutex
        bool m_stopRequested{false};
        bool m_paused{false};

        std::mutex m_mutex;
        std::condition_variable m_cv;
    };

}

#endif