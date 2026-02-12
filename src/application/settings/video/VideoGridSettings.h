#ifndef CLEANGRADUATOR_VIDEOGRIDSETTINGS_H
#define CLEANGRADUATOR_VIDEOGRIDSETTINGS_H
#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

namespace application::models {
    class VideoGridSettings {
    public:
        VideoGridSettings() = default;

        explicit VideoGridSettings(std::vector<int> indexes) {
            std::sort(indexes.begin(), indexes.end());

            std::string result;
            for (int i : indexes) {
                result += std::to_string(i);
            }

            as_string_ = result;
        }

        explicit VideoGridSettings(const std::string& input) {
            std::unordered_set<int> seen;
            std::vector<int> indexes;
            std::string normalized;

            for (unsigned char c : input) {
                if (!std::isdigit(c))
                    continue;

                int idx = c - '0';

                if (seen.insert(idx).second) {
                    indexes.push_back(idx);
                }
            }

            std::sort(indexes.begin(), indexes.end());

            for (int i : indexes) {
                normalized += std::to_string(i);
            }

            active_ = indexes;
            as_string_ = normalized;
        }

        const std::vector<int>& activeIndexes() const { return active_; }

    private:
        std::vector<int> active_;
        std::string as_string_;
    };
}

#endif //CLEANGRADUATOR_VIDEOGRIDSETTINGS_H