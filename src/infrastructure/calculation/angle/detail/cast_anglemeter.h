#ifndef CAM8_CAST_ANGLEMETER_H
#define CAM8_CAST_ANGLEMETER_H

#include <vector>

// ------------------------------------
// Простейшие структуры координат
// ------------------------------------
struct pos_t {
    unsigned short x;
    unsigned short y;
};

// ------------------------------------
// Координата в виде float для точной геометрии
// ------------------------------------
struct posf_t {
    float x;
    float y;
};

// ------------------------------------
// Результаты поиска перепадов яркости по линии сканирования
// ------------------------------------
struct scan_t {
    pos_t posDifMin;
    pos_t posDifMax;
};

struct anglemeter_config {
    int bright_lim = 150;
    int max_pairs = 6;
    int scan_step = 2;
    std::vector<int> offsets = {0, -8, +8, -16, +16, -24, +24, -36, +36, -48, +48, -56, +56, -64, +64, -82, +82, -96, +96};
};

// ------------------------------------
// Основное состояние алгоритма измерения угла
// ------------------------------------
struct anglemeter_t {
    anglemeter_config config;

    int img_width{};
    int img_height{};

    std::vector<scan_t> x_scans{};
    std::vector<scan_t> y_scans{};

    std::vector<posf_t> points_1{};
    std::vector<posf_t> points_2{};

    float last_angle_deg{};
    float (*transform_angle)(float) = nullptr;
};


void anglemeterCreate(anglemeter_t** am_ptr, anglemeter_config config);
void anglemeterDestroy(anglemeter_t* am);
void anglemeterSetConfig(anglemeter_t* am, anglemeter_config config);
void anglemeterSetImageSize(anglemeter_t* am, int width, int height);
void anglemeterSetAngleTransformation(anglemeter_t* am, float (*func_ptr)(float));
void anglemeterRestoreState(anglemeter_t* am);

// ----------------------------------------------------------------------------------------
// Основная функция вычисления угла стрелки
// На основании геометрии двух линий стрелки на изображении вычисляет значение угла,
// а также дополнительные значения (angle1, angle2), представляющие углы
// каждого из двух обнаруженных контуров отдельно.
// ----------------------------------------------------------------------------------------
bool anglemeterGetArrowAngle(anglemeter_t* am, const unsigned char *img, float* angle);

#endif //CAM8_CAST_ANGLEMETER_H