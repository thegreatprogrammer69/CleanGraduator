#ifndef CLEANGRADUATOR_ATOMICSTRUCT_H
#define CLEANGRADUATOR_ATOMICSTRUCT_H

#include <atomic>
#include <memory>
#include <functional>

namespace utils::atomic {

    template<typename T>
    class AtomicStruct
    {
    public:
        AtomicStruct()
            : m_ptr(new T{})
        {}

        explicit AtomicStruct(const T& value)
            : m_ptr(new T(value))
        {}

        ~AtomicStruct()
        {
            delete m_ptr.load(std::memory_order_acquire);
        }

        // === Snapshot read ===
        T load() const
        {
            T* ptr = m_ptr.load(std::memory_order_acquire);
            return *ptr; // копия
        }

        // === Полная замена ===
        void store(const T& value)
        {
            T* newPtr = new T(value);
            T* oldPtr = m_ptr.exchange(newPtr, std::memory_order_acq_rel);
            delete oldPtr;
        }

        // === CAS update через функцию ===
        void update(const std::function<void(T&)>& mutator)
        {
            while (true)
            {
                T* oldPtr = m_ptr.load(std::memory_order_acquire);

                std::unique_ptr<T> newPtr = std::make_unique<T>(*oldPtr);
                mutator(*newPtr);

                if (m_ptr.compare_exchange_weak(
                        oldPtr,
                        newPtr.get(),
                        std::memory_order_acq_rel,
                        std::memory_order_acquire))
                {
                    newPtr.release();
                    delete oldPtr;
                    return;
                }
            }
        }

    private:
        std::atomic<T*> m_ptr;
    };

}

#endif //CLEANGRADUATOR_ATOMICSTRUCT_H