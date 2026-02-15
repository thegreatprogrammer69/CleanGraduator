#ifndef CLEANGRADUATOR_QTSETTINGSSTORAGE_H
#define CLEANGRADUATOR_QTSETTINGSSTORAGE_H


#include <QSettings>

#include "../../application/ports/outbound/settings/ISettingsStorage.h"

namespace infra::storage {

    class QtSettingsStorage final : public application::ports::ISettingsStorage {
    public:
        QtSettingsStorage(QString organization, QString application);
        ~QtSettingsStorage() override;

    private:
        QSettings settings_;
    };

}


#endif //CLEANGRADUATOR_QTSETTINGSSTORAGE_H