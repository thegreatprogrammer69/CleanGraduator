#include "QtSettingsStorage.h"

using namespace infra::storage;
using namespace application::ports;

namespace {
    constexpr const char* GROUP = "VideoGrid";
    constexpr const char* KEY_ACTIVE = "active";
}

QtSettingsStorage::QtSettingsStorage(QString organization, QString application)
    : settings_(organization, application)
{
}

QtSettingsStorage::~QtSettingsStorage() {
}

// VideoGridSettings QtSettingsStorage::loadVideoGridSettings()
// {
//     settings_.beginGroup(GROUP);
//
//     const QString value =
//         settings_.value(KEY_ACTIVE, "").toString();
//
//     settings_.endGroup();
//
//     return VideoGridSettings(value.toStdString());
// }
//
// void QtSettingsStorage::saveVideoGridSettings(
//     const VideoGridSettings& settings)
// {
//     settings_.beginGroup(GROUP);
//
//     settings_.setValue(
//         KEY_ACTIVE,
//         QString::fromStdString(settings.asString())
//     );
//
//     settings_.endGroup();
//
//     settings_.sync();
// }