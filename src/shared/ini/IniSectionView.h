#ifndef CLEANGRADUATOR_INISECTIONVIEW_H
#define CLEANGRADUATOR_INISECTIONVIEW_H

namespace utils::ini {
    class IniSection;

    template<typename T>
    class IniSectionView {
    public:
        IniSectionView(IniSection& section);

        T load(const T& defaults = {});
        void save(const T& value);

    private:
        IniSection& section_;
    };
}

#endif //CLEANGRADUATOR_INISECTIONVIEW_H