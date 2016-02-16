
#ifndef CHIPEMU_H
#define CHIPEMU_H

namespace chipemu
{

extern unsigned lib_version_number;

class chip
{
public:

    virtual const char* name() const noexcept = 0;
    virtual unsigned node_count() const noexcept = 0;
    virtual unsigned transistor_count() const noexcept = 0;
    virtual void stabilize_network() noexcept = 0;
    virtual void recalc() noexcept = 0;

    virtual ~chip();

};

}

#endif
