
#ifndef TESTBENCH_MEMORY_H
#define TESTBENCH_MEMORY_H

#include <memory>
#include <vector>

namespace testbench
{

class address_range
{
public:
    virtual bool is_writable() const = 0;
    virtual bool is_visible() const = 0;
    virtual bool contains(unsigned address) const = 0;
    virtual unsigned char read(unsigned address) const = 0;
    virtual void write(unsigned address, unsigned char value) = 0;
    virtual ~address_range();
};

class memory 
{
    std::vector<std::shared_ptr<address_range> > ranges;

public:

    void add_range(std::shared_ptr<address_range>);
    void add_RAM(unsigned start_address, unsigned size);
    void add_ROM(unsigned start_address,
                 const unsigned char *data,
                 unsigned size);

    unsigned char read(unsigned address) const noexcept;
    void write(unsigned address, unsigned char value) noexcept;

};

}

#endif
