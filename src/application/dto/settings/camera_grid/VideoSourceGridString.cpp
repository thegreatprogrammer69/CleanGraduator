#include "VideoSourceGridString.h"

application::dto::VideoSourceGridString::VideoSourceGridString(const std::string &input) {
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

application::dto::VideoSourceGridString::VideoSourceGridString() = default;

std::string application::dto::VideoSourceGridString::toString() const {
    std::string str;
    for (const auto idx : indexes_) {
        str += std::to_string(idx);
    }
    return str;
}

const std::vector<int> & application::dto::VideoSourceGridString::indexes() const noexcept {
    return indexes_;
}

bool application::dto::VideoSourceGridString::empty() const noexcept {
    return indexes_.empty();
}
