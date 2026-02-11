#ifndef CLEANGRADUATOR_DICONTAINER_H
#define CLEANGRADUATOR_DICONTAINER_H

#include <any>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

class DIContainer {
public:
    template <typename T>
    void add(std::shared_ptr<T> service) {
        services_[std::type_index(typeid(T))] = std::move(service);
    }

    template <typename T>
    std::shared_ptr<T> get() const {
        const auto it = services_.find(std::type_index(typeid(T)));
        if (it == services_.end()) {
            throw std::runtime_error("Service not found in DIContainer");
        }

        return std::any_cast<std::shared_ptr<T>>(it->second);
    }

private:
    std::unordered_map<std::type_index, std::any> services_;
};

#endif //CLEANGRADUATOR_DICONTAINER_H
