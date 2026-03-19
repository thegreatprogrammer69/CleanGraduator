#ifndef CLEANGRADUATOR_SHARED_COMPENSATIONSTACK_H
#define CLEANGRADUATOR_SHARED_COMPENSATIONSTACK_H

#include <functional>
#include <utility>
#include <vector>

namespace shared
{

    class CompensationStack
    {
    public:
        CompensationStack() = default;

        CompensationStack(const CompensationStack&) = delete;
        CompensationStack& operator=(const CompensationStack&) = delete;

        CompensationStack(CompensationStack&&) = delete;
        CompensationStack& operator=(CompensationStack&&) = delete;

        ~CompensationStack()
        {
            if (committed_)
                return;

            rollback();
        }

        template<class F>
        void push(F&& action)
        {
            actions_.emplace_back(std::forward<F>(action));
        }

        void commit() noexcept
        {
            committed_ = true;
        }

        void rollback() noexcept
        {
            while (!actions_.empty())
            {
                auto action = std::move(actions_.back());
                actions_.pop_back();

                try
                {
                    action();
                }
                catch (...)
                {
                    // compensation path must not throw
                }
            }
        }

    private:
        std::vector<std::function<void()>> actions_;
        bool committed_{false};
    };

} // namespace shared

#endif // CLEANGRADUATOR_SHARED_COMPENSATIONSTACK_H