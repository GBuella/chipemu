
#include "machine_implementation.h"

#include <vector>
#include <cstring>
#include <memory>

namespace testbench
{

void machine_implementation::enable_trace(FILE *stream)
{
    trace_out = stream;
}

void machine_implementation::disable_trace()
{
    trace_out = nullptr;
}

machine_implementation::~machine_implementation()
{
}

machine::~machine()
{
}

namespace
{

struct machines_container
{
    std::vector<const char*> names;
    std::vector<machine*(*)()> constructors;
};

static machines_container *machines;

class destroyer
{
public:
    ~destroyer()
    {
        if (machines != nullptr) {
            delete machines;
        }
    }
};

static destroyer machines_destroyer;

static void factory_init()
{
    if (machines == nullptr) {
        machines = new machines_container;
    }
}

}

const char ** machine::availability::begin()
{
    factory_init();
    return &machines->names[0];
}

const char ** machine::availability::end()
{
    factory_init();
    return &machines->names[machines->names.size()];
}

size_t machine::availability::count()
{
    factory_init();
    return machines->names.size();
}

bool machine::availability::is(const char *name)
{
    factory_init();
    for (auto current_name : machines->names) {
        if (strcmp(current_name, name) == 0) {
            return true;
        }
    }
    return false;
}

machine* machine::create(const char *name)
{
    factory_init();
    for (unsigned index = 0; index < machines->names.size(); ++index) { 
        if (strcmp(machines->names[index], name) == 0) {
            return machines->constructors[index]();
        }
    }
    return nullptr;
}

machine_implementation::registrar::registrar(const char *name,
                                           machine*(*constructor)())
{
    factory_init();
    machines->names.push_back(name);
    machines->constructors.push_back(constructor);
}

machine::availability machine::available;

}
