#ifndef CLEANGRADUATOR_CAMERASTRING_H
#define CLEANGRADUATOR_CAMERASTRING_H
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype>

namespace application::dto {

    class VideoSourceGridString {
    public:
        static VideoSourceGridString fromIndexes(std::vector<int> indexes) {
            std::sort(indexes.begin(), indexes.end());

            std::string result;
            for (int i : indexes) {
                result += std::to_string(i);
            }
            return VideoSourceGridString(result, std::move(indexes));
        }

        static VideoSourceGridString fromString(const std::string& input) {
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

            return VideoSourceGridString(
                std::move(normalized),
                std::move(indexes)
            );
        }

        const std::vector<int>& indexes() const { return indexes_; }
        const std::string& asString() const { return value_; }

        bool operator = (const VideoSourceGridString& o) {
            return value_ == o.value_;
        }

    private:
        VideoSourceGridString(std::string value, std::vector<int> indexes)
            : value_(std::move(value)), indexes_(std::move(indexes)) {}

        std::string value_;
        std::vector<int> indexes_;
    };


} // namespace application::model

#endif //CLEANGRADUATOR_CAMERASTRING_H