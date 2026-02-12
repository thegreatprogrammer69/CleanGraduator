#include "QtComponentSettingsRepository.h"

#include <QSettings>

using namespace application::models;

QtComponentSettingsRepository::QtComponentSettingsRepository(QString filePath)
    : path_(std::move(filePath))
{
}

void QtComponentSettingsRepository::save(
    const ComponentSettings& settings)
{
    QSettings qs(path_, QSettings::IniFormat);

    for (const auto& [key, value] : settings.values)
    {
        QString qkey = QString::fromStdString(key);

        std::visit([&](auto&& v) {

            using T = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<T, int>)
            {
                qs.setValue(qkey, v);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                qs.setValue(qkey, v);
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                qs.setValue(qkey, QString::fromStdString(v));
            }

        }, value);
    }
}

ComponentSettings QtComponentSettingsRepository::load() const
{
    QSettings qs(path_, QSettings::IniFormat);

    ComponentSettings result;

    for (const auto& key : qs.allKeys())
    {
        QVariant v = qs.value(key);

        const std::string skey = key.toStdString();

        switch (v.type())
        {
            case QVariant::Int:
                result.values[skey] = v.toInt();
                break;

            case QVariant::Bool:
                result.values[skey] = v.toBool();
                break;

            default:
                result.values[skey] = v.toString().toStdString();
                break;
        }
    }

    return result;
}
