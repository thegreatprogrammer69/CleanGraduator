#include "ThreadWorker.h"

namespace utils::thread {
    ThreadWorker::ThreadWorker(Task task): m_task(std::move(task)) {}

    ThreadWorker::~ThreadWorker() {
        stop();
    }

    void ThreadWorker::start() {
        if (m_running.load())
            return;

        m_stopRequested.store(false);
        m_paused.store(false);
        m_running.store(true);

        m_thread = std::thread(&ThreadWorker::threadLoop, this);
    }

    void ThreadWorker::stop() {
        if (!m_running.load())
            return;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopRequested.store(true);
            m_paused.store(false);
        }

        m_cv.notify_all();

        if (m_thread.joinable())
            m_thread.join();

        m_running.store(false);
    }

    void ThreadWorker::pause() {
        if (!m_running.load())
            return;

        m_paused.store(true);
    }

    void ThreadWorker::resume() {
        if (!m_running.load())
            return;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_paused.store(false);
        }

        m_cv.notify_all();
    }

    bool ThreadWorker::isRunning() const noexcept {
        return m_running.load();
    }

    void ThreadWorker::threadLoop() {
        while (true)
        {
            // Проверка на остановку
            if (m_stopRequested.load())
                break;

            // Обработка паузы
            if (m_paused.load())
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [this]
                {
                    return !m_paused.load() || m_stopRequested.load();
                });

                if (m_stopRequested.load())
                    break;
            }

            // Выполнение пользовательской задачи
            if (m_task)
                m_task();
        }
    }
}