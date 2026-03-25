#ifndef CLEANGRADUATOR_COLOR_H
#define CLEANGRADUATOR_COLOR_H


#include <cstdint>
#include <string>

class Color {
private:
    uint8_t r_;
    uint8_t g_;
    uint8_t b_;

public:
    // Конструкторы
    Color();
    Color(uint8_t r, uint8_t g, uint8_t b);

    // Фабричный метод
    static Color FromHex(const std::string& hex);

    // Геттеры
    uint8_t R() const;
    uint8_t G() const;
    uint8_t B() const;

    // Сеттеры
    void SetR(uint8_t r);
    void SetG(uint8_t g);
    void SetB(uint8_t b);
    void Set(uint8_t r, uint8_t g, uint8_t b);

    // Преобразование
    std::string ToHex() const;

    // Операторы сравнения
    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;
};


#endif //CLEANGRADUATOR_COLOR_H