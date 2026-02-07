#ifndef CLEANGRADUATOR_PRESSUREUNIT_H
#define CLEANGRADUATOR_PRESSUREUNIT_H


namespace domain::common {
    enum class PressureUnit {
        Pa,
        kPa,
        MPa,
        bar,
        atm,
        mmHg,
        mmH2O,
        kgf_cm2,
        kgf_m2
    };
}


#endif //CLEANGRADUATOR_PRESSUREUNIT_H