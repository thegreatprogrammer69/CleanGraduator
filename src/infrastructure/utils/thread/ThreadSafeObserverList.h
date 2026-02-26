#ifndef CLEANGRADUATOR_THREADSAFEOBSERVERLIST_H
#define CLEANGRADUATOR_THREADSAFEOBSERVERLIST_H
#include <mutex>
#include <vector>

template<typename T>
class ThreadSafeObserverList {
public:
    void add(T& observer) {
        std::lock_guard lock(mutex_);
        observers_.push_back(&observer);
    }

    void remove(T& observer) {
        std::lock_guard lock(mutex_);
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), &observer),
            observers_.end()
        );
    }

    template<typename Func>
    void notify(Func&& func) {
        std::vector<T*> copy;
        {
            std::lock_guard lock(mutex_);
            copy = observers_;
        }

        for (auto* o : copy)
            func(*o);
    }

private:
    std::vector<T*> observers_;
    std::mutex mutex_;
};

#endif //CLEANGRADUATOR_THREADSAFEOBSERVERLIST_H