#ifndef CLEANGRADUATOR_REGISTRY_H
#define CLEANGRADUATOR_REGISTRY_H
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include "IRegistry.h"

template<typename Interface, typename Ports>
class Registry {
public:
    using Factory = std::function<
        std::unique_ptr<Interface>(const Ports&)
    >;

    explicit Registry(const Ports& ports)
        : ports_(ports) {}

    void registerFactory(std::string name, Factory factory) {
        factories_[std::move(name)] = std::move(factory);
    }

    std::unique_ptr<Interface> create(std::string_view name) const {
        auto it = factories_.find(std::string(name));
        if (it == factories_.end())
            throw std::runtime_error("Unknown implementation: " + std::string(name));
        return it->second(ports_);
    }

    bool has(std::string_view name) const {
        return factories_.contains(std::string(name));
    }

private:
    const Ports& ports_;
    std::unordered_map<std::string, Factory> factories_;
};


#endif //CLEANGRADUATOR_REGISTRY_H