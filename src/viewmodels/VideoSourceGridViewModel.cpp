#include "VideoSourceGridViewModel.h"
#include "VideoSourceViewModel.h"


using namespace mvvm;


VideoSourceGridViewModel::Slot::Slot(int row, int col, VideoSourceViewModel &vm)
    : row(row), col(col), vm(vm)
{
}

VideoSourceGridViewModel::Slot::~Slot() = default;

VideoSourceGridViewModel::VideoSourceGridViewModel(Slots video_slots, int rows, int cols, double aspectRatioWH)
    : slots_(std::move(video_slots)), rows_(rows), cols_(cols), aspectRatioWH_(aspectRatioWH)
{
}

VideoSourceGridViewModel::~VideoSourceGridViewModel() = default;

int VideoSourceGridViewModel::rows() const {
    return rows_;
}

int VideoSourceGridViewModel::cols() const {
    return cols_;
}

double VideoSourceGridViewModel::aspectRatioWH() const {
    return aspectRatioWH_;
}

const VideoSourceGridViewModel::Slots& VideoSourceGridViewModel::videoSlots() const {
    return slots_;
}
