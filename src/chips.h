
#ifndef CHIPEMU_CHIPS_H
#define CHIPEMU_CHIPS_H

#include "chipemu.h"

namespace chipemu
{
namespace implementation
{

class registrar
{
public:
	registrar(const char*, chipemu::chip*(*create_func)());
};

}
}

#endif
