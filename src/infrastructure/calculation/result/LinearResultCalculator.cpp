#include "LinearResultCalculator.h"
#include "domain/core/measurement/PressurePoints.h"
#include "domain/core/measurement/MeasurementSeries.h"

using namespace infra::calculation;
using namespace domain::common;

namespace {
    struct calculateForPointResult {
        double angle;

    };
    double calculateForPoint(const MeasurementSeries& series, PressurePoint pressure_point, double over) {
        const auto& pressures = series.pressureSamples();
        const auto& angles = series.angleSamples();

        if (pressures.empty() || angles.empty()) return -1;

        using pressure_it = decltype(pressures.begin());
        using angle_it    = decltype(angles.begin());

        pressure_it pressure_from = pressures.end();
        pressure_it pressure_to   = pressures.end();
        angle_it angle_from       = angles.end();
        angle_it angle_to         = angles.end();

        // 1) Находим диапазон давлений [pressure_point - over, pressure_point + over]
        for (pressure_it it = pressures.begin(); it != pressures.end(); ++it) {
            if (it->pressure > pressure_point - over) {
                pressure_from = it;
                break;
            }
        }

        if (pressure_from == pressures.end()) return -1;

        for (pressure_it it = pressure_from; it != pressures.end(); ++it) {
            if (it->pressure > pressure_point + over) {
                pressure_to = it;
                break;
            }
        }

        if (pressure_to == pressures.end()) return -1;

        // У вас было: нужно минимум 3 шага по давлению в диапазоне
        if (pressure_to - pressure_from < 3) return -1;

        const auto time_from = pressure_from->timestamp;
        const auto time_to   = pressure_to->timestamp;

        // 2) Находим диапазон углов по времени [time_from, time_to]
        for (angle_it it = angles.begin(); it != angles.end(); ++it) {
            if (it->timestamp > time_from) {
                angle_from = it;
                break;
            }
        }

        if (angle_from == angles.end()) return -1;

        for (angle_it it = angle_from; it != angles.end(); ++it) {
            if (it->timestamp > time_to) {
                angle_to = it;
                break;
            }
        }

        if (angle_to == angles.end()) return -1;

        // У вас было: нужно минимум 6 шагов по углу в диапазоне
        if (angle_to - angle_from < 6) return -1;

        // ------------------------------------------------------------
        // A) Интерполяция "время-давление": найти t*, когда P(t*) = pressure_point
        // ------------------------------------------------------------
        // Ищем два соседних pressure-сэмпла, между которыми находится pressure_point
        // (подразумеваем, что pressure монотонен в этом окне; если нет — всё равно найдём первое пересечение)
        pressure_it p1 = pressures.end();
        pressure_it p2 = pressures.end();

        for (pressure_it it = pressure_from; it != pressure_to; ++it) {
            auto it_next = std::next(it);
            if (it_next == pressures.end()) break;

            const double P_a = it->pressure;
            const double P_b = it_next->pressure;

            // pressure_point лежит между P_a и P_b (в любом направлении)
            const bool crosses =
                (P_a <= pressure_point && pressure_point <= P_b) ||
                (P_b <= pressure_point && pressure_point <= P_a);

            if (crosses) {
                p1 = it;
                p2 = it_next;
                break;
            }
        }

        if (p1 == pressures.end() || p2 == pressures.end()) return -1;

        const double P1 = p1->pressure;
        const double P2 = p2->pressure;

        // Защита от деления на ноль (два одинаковых давления подряд)
        const double dP = (P2 - P1);
        if (dP == 0.0) return -1;

        // timestamp может быть любым числом/типом. Если это double/float/int — ок.
        // Если это std::chrono::time_point, нужно будет отдельно конвертировать в duration count.
        const double t1 = static_cast<double>(p1->timestamp);
        const double t2 = static_cast<double>(p2->timestamp);

        const double alpha = (pressure_point - P1) / dP; // 0..1 (или около того)
        const double t_star = t1 + alpha * (t2 - t1);

        // ------------------------------------------------------------
        // B) Интерполяция "время-угол": найти angle(t_star)
        // ------------------------------------------------------------
        // Ищем два соседних angle-сэмпла вокруг t_star
        angle_it a1 = angles.end();
        angle_it a2 = angles.end();

        // angle_from — первый с timestamp > time_from; значит предыдущий может быть валиден,
        // но чтобы не выходить за границы, ищем в [begin..end) от angle_from-1, если можно.
        angle_it search_begin = angle_from;
        if (search_begin != angles.begin()) {
            --search_begin;
        }

        for (angle_it it = search_begin; it != angle_to; ++it) {
            auto it_next = std::next(it);
            if (it_next == angles.end()) break;

            const double T_a = static_cast<double>(it->timestamp);
            const double T_b = static_cast<double>(it_next->timestamp);

            // t_star лежит между T_a и T_b (предполагаем T_a <= T_b; если нет — тоже обработаем)
            const bool crosses =
                (T_a <= t_star && t_star <= T_b) ||
                (T_b <= t_star && t_star <= T_a);

            if (crosses) {
                a1 = it;
                a2 = it_next;
                break;
            }
        }

        if (a1 == angles.end() || a2 == angles.end()) return -1;

        const double T1 = static_cast<double>(a1->timestamp);
        const double T2 = static_cast<double>(a2->timestamp);
        const double dT = (T2 - T1);
        if (dT == 0.0) return -1;

        const double A1 = a1->angle;
        const double A2 = a2->angle;

        const double beta = (t_star - T1) / dT;
        const double angle_star = A1 + beta * (A2 - A1);

        return angle_star;
    }

}

Result LinearResultCalculator::calculate(const MeasurementSeries& series, const PressurePoints& points) const {
    std::vector<double> angles;
    const double over = std::abs(points[0] - points[1]) * 0.02;
    for (const auto& p : points) {
        calculateForPoint(series, p, over);
    }
    return Result();
}
