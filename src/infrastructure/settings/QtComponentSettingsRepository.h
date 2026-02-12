#ifndef CLEANGRADUATOR_QTCOMPONENTSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_QTCOMPONENTSETTINGSREPOSITORY_H

#include <QString>
#include "application/ports/outbound/IComponentSettingsRepository.h"

class QtComponentSettingsRepository final : public application::ports::IComponentSettingsRepository
{
public:
    explicit QtComponentSettingsRepository(QString filePath);

    void save(const application::models::ComponentSettings& settings) override;

    application::models::ComponentSettings load() const override;

private:
    QString path_;
};

#endif //CLEANGRADUATOR_QTCOMPONENTSETTINGSREPOSITORY_H