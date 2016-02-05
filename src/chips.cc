
#include "chips.h"

#include <unordered_map>
#include <memory>
#include <string>

typedef std::unordered_map<std::string, chipemu::chip*(*)()> ctor_map_type;

static std::unique_ptr<ctor_map_type> constructor_map;

chipemu::implementation::registrar::registrar(const char *name,
                                              chip*(*consturctor)())
{
    if (constructor_map == nullptr) {
        constructor_map = std::make_unique<ctor_map_type>();
    }
    constructor_map->emplace(std::string(name), consturctor);
}

chipemu::chip*
chipemu::create(const char* name)
{
    if (constructor_map != nullptr) {
        auto consturctor = constructor_map->find(std::string(name));
        if (consturctor != constructor_map->end()) {
            return consturctor->second();
        }
    }
    return nullptr;
}

chipemu::chip::~chip()
{
}

