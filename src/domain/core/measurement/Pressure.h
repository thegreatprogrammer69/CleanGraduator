#ifndef GRADUATOR_PRESSURE_H
#define GRADUATOR_PRESSURE_H
#include <stdexcept>
#include <cmath>

#include "PressureUnit.h"


namespace domain::common {

class Pressure {
public:
    // Храним всегда в Паскалях
    Pressure() = default;
    explicit Pressure(double value, PressureUnit unit) {
        set(value, unit);
    }

    // Фабрики
    static Pressure fromPa(double v)      { return Pressure(v, PressureUnit::Pa); }
    static Pressure fromKPa(double v)     { return Pressure(v, PressureUnit::kPa); }
    static Pressure fromMPa(double v)     { return Pressure(v, PressureUnit::MPa); }
    static Pressure fromBar(double v)     { return Pressure(v, PressureUnit::bar); }
    static Pressure fromAtm(double v)     { return Pressure(v, PressureUnit::atm); }
    static Pressure fromMmHg(double v)    { return Pressure(v, PressureUnit::mmHg); }
    static Pressure fromMmH2O(double v)   { return Pressure(v, PressureUnit::mmH2O); }
    static Pressure fromKgfCm2(double v)  { return Pressure(v, PressureUnit::kgf_cm2); }
    static Pressure fromKgfM2(double v)   { return Pressure(v, PressureUnit::kgf_m2); }

    // Геттеры
    double pa()  const { return m_pa; }
    double kpa() const { return to(PressureUnit::kPa); }
    double mpa() const { return to(PressureUnit::MPa); }
    double bar() const { return to(PressureUnit::bar); }
    double atm() const { return to(PressureUnit::atm); }

    double value() const {
        return to(m_unit);
    }

    double to(PressureUnit unit) const {
        return m_pa / factorToPa(unit);
    }

    // Сеттер
    void set(double value, PressureUnit unit) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("Pressure: value is not finite");
        }
        m_pa = value * factorToPa(unit);
        m_unit = unit;
    }

    PressureUnit unit() const { return m_unit; }
    void setUnit(PressureUnit u) { m_unit = u; }

    // Арифметика
    friend Pressure operator+(Pressure a, Pressure b) {
        return Pressure::fromPa(a.m_pa + b.m_pa);
    }

    friend Pressure operator-(Pressure a, Pressure b) {
        return Pressure::fromPa(a.m_pa - b.m_pa);
    }

    friend Pressure operator*(Pressure a, double k) {
        return Pressure::fromPa(a.m_pa * k);
    }

    friend Pressure operator/(Pressure a, double k) {
        return Pressure::fromPa(a.m_pa / k);
    }

    Pressure& operator+=(Pressure other) {
        m_pa += other.m_pa;
        return *this;
    }

    Pressure& operator-=(Pressure other) {
        m_pa -= other.m_pa;
        return *this;
    }

    // Сравнение
    friend bool operator==(Pressure a, Pressure b) {
        return a.m_pa == b.m_pa;
    }

    friend bool operator<(Pressure a, Pressure b) {
        return a.m_pa < b.m_pa;
    }

    bool approximatelyEquals(
        Pressure other,
        double relEps = 1e-9,
        double absEps = 0.0
    ) const {
        double diff = std::fabs(m_pa - other.m_pa);
        if (diff <= absEps) return true;
        double scale = std::max(std::fabs(m_pa), std::fabs(other.m_pa));
        return diff <= scale * relEps;
    }

    // Коэффициенты перевода в Паскали
    static double factorToPa(PressureUnit u) {
        switch (u) {
            case PressureUnit::Pa:       return 1.0;
            case PressureUnit::kPa:      return 1'000.0;
            case PressureUnit::MPa:      return 1'000'000.0;
            case PressureUnit::bar:      return 100'000.0;
            case PressureUnit::atm:      return 101'325.0;
            case PressureUnit::mmHg:     return 133.322387415;
            case PressureUnit::mmH2O:    return 9.80665;
            case PressureUnit::kgf_cm2:  return 98'066.5;
            case PressureUnit::kgf_m2:   return 9.80665;
        }
        throw std::invalid_argument("Unknown PressureUnit");
    }

private:
    double m_pa {0.0};                     // базовая единица — Паскали
    PressureUnit m_unit {PressureUnit::kPa}; // единица отображения
};

}

#endif // GRADUATOR_PRESSURE_H