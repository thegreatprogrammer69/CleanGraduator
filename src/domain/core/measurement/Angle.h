#ifndef CLEANGRADUATOR_ANGLE_H
#define CLEANGRADUATOR_ANGLE_H

#include <stdexcept>
#include <cmath>
#include "AngleUnit.h"


namespace domain::common {

class Angle {
public:
    // Храним всегда в радианах
    constexpr Angle() = default;
    explicit constexpr Angle(double value, AngleUnit unit) { set(value, unit); }

    // Фабрики
    static constexpr Angle fromRadians(double v) { return Angle(v, AngleUnit::rad); }
    static constexpr Angle fromDegrees(double v) { return Angle(v, AngleUnit::deg); }

    // Быстрые геттеры
    constexpr double radians() const { return m_rad; }
    constexpr double degrees() const { return to(AngleUnit::deg); }

    // Значение в "единице отображения"
    constexpr double value() const { return to(m_unit); }

    // Конвертация
    constexpr double to(AngleUnit unit) const {
        return m_rad / factorToRad(unit);
    }

    // Сеттер
    constexpr void set(double value, AngleUnit unit) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("Angle: value is not finite");
        }
        m_rad = value * factorToRad(unit);
        m_unit = unit;
    }

    // Единица отображения (не влияет на хранение)
    constexpr AngleUnit unit() const { return m_unit; }
    constexpr void setUnit(AngleUnit u) { m_unit = u; }

    // Арифметика
    friend constexpr Angle operator+(Angle a, Angle b) { return Angle::fromRadians(a.m_rad + b.m_rad); }
    friend constexpr Angle operator-(Angle a, Angle b) { return Angle::fromRadians(a.m_rad - b.m_rad); }
    friend constexpr Angle operator*(Angle a, double k){ return Angle::fromRadians(a.m_rad * k); }
    friend constexpr Angle operator*(double k, Angle a){ return Angle::fromRadians(a.m_rad * k); }
    friend constexpr Angle operator/(Angle a, double k){ return Angle::fromRadians(a.m_rad / k); }

    Angle& operator+=(Angle other) { m_rad += other.m_rad; return *this; }
    Angle& operator-=(Angle other) { m_rad -= other.m_rad; return *this; }
    Angle& operator*=(double k)    { m_rad *= k; return *this; }
    Angle& operator/=(double k)    { m_rad /= k; return *this; }

    // Сравнения
    friend constexpr bool operator==(Angle a, Angle b) { return a.m_rad == b.m_rad; }
    friend constexpr bool operator!=(Angle a, Angle b) { return !(a == b); }
    friend constexpr bool operator<(Angle a, Angle b)  { return a.m_rad < b.m_rad; }
    friend constexpr bool operator<=(Angle a, Angle b) { return a.m_rad <= b.m_rad; }
    friend constexpr bool operator>(Angle a, Angle b)  { return a.m_rad > b.m_rad; }
    friend constexpr bool operator>=(Angle a, Angle b) { return a.m_rad >= b.m_rad; }

    // Приблизительное сравнение (для double)
    bool approximatelyEquals(Angle other, double relEps = 1e-12, double absEps = 0.0) const {
        const double diff = std::fabs(m_rad - other.m_rad);
        if (diff <= absEps) return true;
        const double scale = std::max(std::fabs(m_rad), std::fabs(other.m_rad));
        return diff <= scale * relEps;
    }

    // Коэффициенты: 1 unit = factor радиан
    static constexpr double factorToRad(AngleUnit u) {
        switch (u) {
            case AngleUnit::rad: return 1.0;
            case AngleUnit::deg: return pi() / 180.0;
        }
        throw std::invalid_argument("Unknown AngleUnit");
    }

    static constexpr double pi() {
        return 3.141592653589793238462643383279502884;
    }

private:
    double m_rad {0.0};                 // базовая единица — радианы
    AngleUnit m_unit {AngleUnit::deg};  // единица отображения
};

}

#endif //CLEANGRADUATOR_ANGLE_H