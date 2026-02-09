#ifndef CLEANGRADUATOR_CAMERASTRING_H
#define CLEANGRADUATOR_CAMERASTRING_H
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype>

namespace application::dto {

    class CameraString {
    public:
        explicit CameraString(const std::string& input) {
            std::unordered_set<int> seen;

            for (unsigned char c : input) {
                if (!std::isdigit(c))
                    continue;

                int idx = c - '0';
                if (seen.insert(idx).second) {
                    indexes_.push_back(idx);
                }
            }
        }

        CameraString() = default;

        std::string toString() const {
            std::string str;
            for (const auto idx : indexes_) {
                str += std::to_string(idx);
            }
            return str;
        }

        const std::vector<int>& indexes() const noexcept {
            return indexes_;
        }

        bool empty() const noexcept {
            return indexes_.empty();
        }

    private:
        std::vector<int> indexes_{};
    };

} // namespace application::model

#endif //CLEANGRADUATOR_CAMERASTRING_H