#ifndef CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H
#define CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H
#include "application/ports/outbound/video/IVideoSourceRepository.h"
#include <string>
#include "infrastructure/video/VideoSourcePorts.h"

namespace infra::repo {
    class VideoSourceRepositoryFromFile final : public application::ports::IVideoSourceRepository {
    public:
        explicit VideoSourceRepositoryFromFile(const std::string& ini_path, camera::VideoSourcePorts video_source_ports);
        ~VideoSourceRepositoryFromFile() override;
        std::vector<domain::ports::IVideoSource*> all() override;

    private:
        void loadFromIniFile(const std::string& ini_path, camera::VideoSourcePorts video_source_ports);

    private:
        std::vector<std::unique_ptr<domain::ports::IVideoSource>> video_sources_;
        std::vector<domain::ports::IVideoSource*> video_sources_ptr_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H