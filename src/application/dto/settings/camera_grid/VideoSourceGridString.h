#ifndef CLEANGRADUATOR_CAMERASTRING_H
#define CLEANGRADUATOR_CAMERASTRING_H
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype>

namespace application::dto {

    class VideoSourceGridString {
    public:
        explicit VideoSourceGridString(const std::string& input);
        VideoSourceGridString();

        std::string toString() const;
        const std::vector<int>& indexes() const noexcept;
        bool empty() const noexcept;

        bool operator==(const VideoSourceGridString& o) const {
            return this->indexes() == o.indexes();
        }

    private:
        std::vector<int> indexes_{};
    };

} // namespace application::model

#endif //CLEANGRADUATOR_CAMERASTRING_H