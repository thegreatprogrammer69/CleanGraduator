#include "ThreadWorker.h"

namespace utils::thread {

    ThreadWorker::ThreadWorker(Task task)
        : m_task(std::move(task))
    {
    }

    ThreadWorker::~ThreadWorker() {
        stop();
    }

    void ThreadWorker::start() {
        if (m_running.load())
            return;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopRequested = false;
            m_paused = false;
        }

        m_running.store(true);
        m_thread = std::thread(&ThreadWorker::threadLoop, this);
    }

    void ThreadWorker::stop() {
        if (!m_running.load())
            return;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopRequested = true;
            m_paused = false;
        }

        m_cv.notify_all();

        if (m_thread.joinable())
            m_thread.join();

        m_running.store(false);
    }

    void ThreadWorker::pause() {
        if (!m_running.load())
            return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_paused = true;
    }

    void ThreadWorker::resume() {
        if (!m_running.load())
            return;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_paused = false;
        }

        m_cv.notify_all();
    }

    bool ThreadWorker::isRunning() const noexcept {
        return m_running.load();
    }

    void ThreadWorker::threadLoop() {
        std::unique_lock<std::mutex> lock(m_mutex);

        while (!m_stopRequested)
        {
            // Ждём если на паузе
            m_cv.wait(lock, [this] {
                return !m_paused || m_stopRequested;
            });

            if (m_stopRequested)
                break;

            // Выполняем задачу вне критической секции
            lock.unlock();

            if (m_task)
                m_task();

            lock.lock();
        }
    }

}