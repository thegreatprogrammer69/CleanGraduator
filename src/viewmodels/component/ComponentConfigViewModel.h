#ifndef CLEANGRADUATOR_COMPONENTCONFIGVIEWMODEL_H
#define CLEANGRADUATOR_COMPONENTCONFIGVIEWMODEL_H

#include <memory>
#include <unordered_map>
#include <string>
#include <any>

#include "application/ports/outbound/IRuntimeConfigurator.h"
#include "application/usecases/settings/ConfigureComponent.h"
#include "viewmodels/Observable.h"

namespace mvvm {

    class ComponentConfigViewModel {
    public:
        using Schema = application::models::ConfigSchema;

        explicit ComponentConfigViewModel(
            application::usecase::ConfigureComponent& usecase
        );

        Observable<Schema> schema;

        void load();

        void setInt(const std::string& name, int value);
        void setBool(const std::string& name, bool value);
        void setText(const std::string& name, const std::string& value);
        void invoke(const std::string& action);

    private:
        application::usecase::ConfigureComponent& usecase_;
    };

} // namespace mvvm


#endif //CLEANGRADUATOR_COMPONENTCONFIGVIEWMODEL_H
