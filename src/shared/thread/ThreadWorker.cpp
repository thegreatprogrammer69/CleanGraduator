#include "ThreadWorker.h"

#include <iostream>
#include <stdexcept>

namespace shared::thread {

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

        if (!m_task) {
            throw std::logic_error("ThreadWorker task is not set");
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopRequested = false;
            m_paused = false;
        }

        m_running.store(true);
        try {
            m_thread = std::thread(&ThreadWorker::threadLoop, this);
        } catch (...) {
            m_running.store(false);
            throw;
        }
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

            try {
                m_task();
            } catch (const std::exception& e) {
                std::cerr << "ThreadWorker task failed: " << e.what() << std::endl;
                lock.lock();
                m_stopRequested = true;
                m_running.store(false);
                m_cv.notify_all();
                break;
            } catch (...) {
                std::cerr << "ThreadWorker task failed: unknown exception" << std::endl;
                lock.lock();
                m_stopRequested = true;
                m_running.store(false);
                m_cv.notify_all();
                break;
            }

            lock.lock();
        }

        m_running.store(false);
    }

}
