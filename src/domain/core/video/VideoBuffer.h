#ifndef CLEANGRADUATOR_VIDEOBUFFER_H
#define CLEANGRADUATOR_VIDEOBUFFER_H
#include <cstdint>
#include <cstdlib>
#include <memory>


namespace domain::common {

struct VideoBuffer {
    uint8_t* data = nullptr;
    size_t   size = 0;

    VideoBuffer() = default;

    explicit VideoBuffer(size_t sz)
        : data(static_cast<uint8_t*>(std::malloc(sz)))
        , size(sz)
    {}

    ~VideoBuffer() {
        std::free(data);
    }

    VideoBuffer(VideoBuffer&& other) noexcept
        : data(other.data)
        , size(other.size)
    {
        other.data = nullptr;
        other.size = 0;
    }

    VideoBuffer& operator=(VideoBuffer&& other) noexcept {
        if (this != &other) {
            std::free(data);
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }

    VideoBuffer(const VideoBuffer&) = delete;
    VideoBuffer& operator=(const VideoBuffer&) = delete;
};

}

#endif //CLEANGRADUATOR_VIDEOBUFFER_H