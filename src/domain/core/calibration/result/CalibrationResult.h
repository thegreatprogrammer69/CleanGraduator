#ifndef CLEANGRADUATOR_CALIBRATIONRESULT_H
#define CLEANGRADUATOR_CALIBRATIONRESULT_H

#include "CalibrationCell.h"
#include "CalibrationCellKey.h"

namespace domain::common {

class CalibrationResult
{
public:

    CalibrationResult(
        std::vector<MotorDirection> directions,
        std::vector<PointId> points,
        std::vector<SourceId> sources
    )
        : directions_(std::move(directions))
        , points_(std::move(points))
        , sources_(std::move(sources))
    {
        buildIndexMaps();

        cells_.resize(
            directions_.size() *
            points_.size() *
            sources_.size()
        );
    }

    CalibrationCell& cell(const CalibrationCellKey& key)
    {
        return cells_[index(
            directionIndex(key.direction),
            pointIndex(key.point_id),
            sourceIndex(key.source_id)
        )];
    }

    const CalibrationCell& cell(const CalibrationCellKey& key) const
    {
        return cells_[index(
            directionIndex(key.direction),
            pointIndex(key.point_id),
            sourceIndex(key.source_id)
        )];
    }

    const std::vector<MotorDirection>& directions() const noexcept
    {
        return directions_;
    }

    const std::vector<PointId>& points() const noexcept
    {
        return points_;
    }

    const std::vector<SourceId>& sources() const noexcept
    {
        return sources_;
    }

private:

    // ----- index helpers -----------------------------------------------------

    size_t directionIndex(MotorDirection d) const
    {
        return direction_index_.at(d);
    }

    size_t pointIndex(PointId p) const
    {
        return point_index_.at(p);
    }

    size_t sourceIndex(SourceId s) const
    {
        return source_index_.at(s);
    }

    size_t index(size_t d, size_t p, size_t s) const
    {
        return d * points_.size() * sources_.size()
             + p * sources_.size()
             + s;
    }

    void buildIndexMaps()
    {
        for (size_t i = 0; i < directions_.size(); ++i)
            direction_index_[directions_[i]] = i;

        for (size_t i = 0; i < points_.size(); ++i)
            point_index_[points_[i]] = i;

        for (size_t i = 0; i < sources_.size(); ++i)
            source_index_[sources_[i]] = i;
    }

private:

    std::vector<MotorDirection> directions_;
    std::vector<PointId> points_;
    std::vector<SourceId> sources_;

    std::unordered_map<MotorDirection, size_t> direction_index_;
    std::unordered_map<PointId, size_t> point_index_;
    std::unordered_map<SourceId, size_t> source_index_;

    std::vector<CalibrationCell> cells_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULT_H