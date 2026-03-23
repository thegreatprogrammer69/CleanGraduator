#ifndef CLEANGRADUATOR_OPERATIONRESULT_H
#define CLEANGRADUATOR_OPERATIONRESULT_H

#include <optional>
#include <utility>

namespace application::common {

template<typename E>
class OperationResult final
{
public:
    static OperationResult success()
    {
        return OperationResult();
    }

    static OperationResult failure(E error)
    {
        return OperationResult(std::move(error));
    }

    bool ok() const
    {
        return !error_.has_value();
    }

    explicit operator bool() const
    {
        return ok();
    }

    const E& error() const
    {
        return *error_;
    }

private:
    OperationResult() = default;

    explicit OperationResult(E error)
        : error_(std::move(error))
    {
    }

private:
    std::optional<E> error_;
};

} // namespace application::common

#endif // CLEANGRADUATOR_OPERATIONRESULT_H
