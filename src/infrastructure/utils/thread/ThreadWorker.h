#ifndef CLEANGRADUATOR_THREADWORKER_H
#define CLEANGRADUATOR_THREADWORKER_H

#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace utils::thread {
    class ThreadWorker
    {
    public:
        using Task = std::function<void()>;

        explicit ThreadWorker(Task task);

        ~ThreadWorker();

        // Запуск потока
        void start();

        // Корректная остановка
        void stop();

        // Пауза
        void pause();

        // Продолжить
        void resume();

        bool isRunning() const noexcept;

    private:
        void threadLoop();

    private:
        Task m_task;
        std::thread m_thread;

        std::atomic<bool> m_running{false};
        std::atomic<bool> m_stopRequested{false};
        std::atomic<bool> m_paused{false};

        std::condition_variable m_cv;
        std::mutex m_mutex;
    };
}

#endif //CLEANGRADUATOR_THREADWORKER_H