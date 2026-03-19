#ifndef CLEANGRADUATOR_SHARED_STARTRESULT_H
#define CLEANGRADUATOR_SHARED_STARTRESULT_H

#include <optional>
#include <string>
#include <utility>

namespace shared
{

    struct StartResult
    {
        std::optional<std::string> error;

        [[nodiscard]] bool ok() const
        {
            return !error.has_value();
        }

        [[nodiscard]] explicit operator bool() const
        {
            return ok();
        }

        static StartResult success()
        {
            return {};
        }

        static StartResult failure(std::string message)
        {
            StartResult result;
            result.error = std::move(message);
            return result;
        }
    };

} // namespace shared

#endif // CLEANGRADUATOR_SHARED_STARTRESULT_H