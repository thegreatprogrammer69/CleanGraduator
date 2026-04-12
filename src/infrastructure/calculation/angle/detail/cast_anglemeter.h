#pragma once

#include <cstdint>
#include <vector>

struct pos_t {
    uint16_t x = 0;
    uint16_t y = 0;
};

struct posf_t {
    float x = 0.0f;
    float y = 0.0f;
};

struct anglemeter_config {
    // Исходные параметры
    int bright_lim = 90;
    int max_pairs = 8;
    int scan_step = 2;
    std::vector<int> offsets{ 0, -12, 12, -24, 24 };

    // Доп. параметры для более гибкой настройки
    int min_object_width = 10;
    int max_object_width = 50;

    int track_margin = 8;
    int background_probe = 16;
    int background_min = 160;
    int min_background_delta = 35;

    int stable_fail_limit = 6;
    int min_track_points = 20;
    float regression_mse_limit = 4.0f;

    float ransac_eps = 1.5f;
    int ransac_iterations = 256;
    float ransac_min_inliers_ratio = 0.72f;

    float angle_match_tolerance_deg = 8.0f;
};

struct anglemeter_t;

void anglemeterCreate(anglemeter_t** am_ptr, anglemeter_config config);
void anglemeterDestroy(anglemeter_t* am);
void anglemeterSetConfig(anglemeter_t* am, anglemeter_config config);
void anglemeterSetImageSize(anglemeter_t* am, int width, int height);
void anglemeterSetAngleTransformation(anglemeter_t* am, float (*func_ptr)(float));
bool anglemeterGetArrowAngle(anglemeter_t* am, const unsigned char* img, float* angle);