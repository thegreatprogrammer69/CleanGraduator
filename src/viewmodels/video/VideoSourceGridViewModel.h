#ifndef CLEANGRADUATOR_VIDEOSOURCEGRIDVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEGRIDVIEWMODEL_H
#include <map>
#include <memory>
#include <vector>

namespace mvvm {
    class VideoSourceViewModel;

    class VideoSourceGridViewModel {
    public:
        struct Slot {
            Slot(int row, int col, VideoSourceViewModel& vm);
            ~Slot();
            int row;
            int col;
            VideoSourceViewModel& vm;
        };
        using Slots = std::vector<Slot>;

        explicit VideoSourceGridViewModel(Slots video_slots, int rows, int cols, double aspectRatioWH);
        ~VideoSourceGridViewModel();

        int rows()const;
        int cols() const;
        double aspectRatioWH() const;
        const Slots& videoSlots() const;

    private:
        Slots slots_;
        int rows_;
        int cols_;
        double aspectRatioWH_;
    };
}


#endif //CLEANGRADUATOR_VIDEOSOURCEGRIDVIEWMODEL_H