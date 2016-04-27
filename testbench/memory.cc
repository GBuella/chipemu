
#include "memory.h"

namespace testbench
{

address_range::~address_range()
{}

namespace
{

class general_RAM : address_range
{
private:
    const unsigned start;
    std::vector<unsigned char> data;

public:

    general_RAM(unsigned start_address, unsigned size):
        start(start_address),
        data(size)
    { }

    virtual bool is_writable() const override final
    {
        return true;
    }

    virtual bool is_visible() const override final
    {
        return true;
    }

    virtual bool contains(unsigned address) const final
    {
        return address >= start and address < start + data.size();
    }

    virtual unsigned char read(unsigned address) const final
    {
        return data[address - start];
    }

    virtual void write(unsigned address, unsigned char value) final
    {
        data[address - start] = value;
    }

};

class general_ROM : address_range
{
private:
    const unsigned char *const data;
    const unsigned start;
    const unsigned size;

public:

    general_ROM(unsigned ctor_start_address,
                const unsigned char *ctor_data,
                unsigned ctor_size):
        data(ctor_data),
        start(ctor_start_address),
        size(ctor_size)
    { }

    virtual bool is_writable() const override final
    {
        return false;
    }

    virtual bool is_visible() const override final
    {
        return true;
    }

    virtual bool contains(unsigned address) const final
    {
        return address >= start and address < start + size;
    }

    virtual unsigned char read(unsigned address) const final
    {
        return data[address - start];
    }

    virtual void write(unsigned address, unsigned char value) final
    {
    }

};

}

void memory::add_range(std::shared_ptr<address_range> range)
{
    ranges.push_back(range);
}

void memory::add_RAM(unsigned start_address, unsigned size)
{
    std::shared_ptr<address_range> RAM;
    
    RAM.reset((address_range*)(new general_RAM(start_address, size)));
    add_range(std::move(RAM));
}

void memory::add_ROM(unsigned start_address,
                     const unsigned char *data,
                     unsigned size)
{
    std::shared_ptr<address_range> ROM;
    
    ROM.reset((address_range*)(new general_ROM(start_address, data, size)));
    add_range(std::move(ROM));
}

unsigned char memory::read(unsigned address) const noexcept
{
    for (auto range : ranges) {
        if (range->is_visible() and range->contains(address)) {
            return range->read(address);
        }
    }
    return 0;
}

void memory::write(unsigned address, unsigned char value) noexcept
{
    for (auto range : ranges) {
        if (range->is_visible() and range->contains(address)) {
            if (range->is_writable()) {
                range->write(address, value);
            }
            return;
        }
    }
}

}
