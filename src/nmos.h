
#ifndef CHIPEMU_CHIP_BASE_H
#define CHIPEMU_CHIP_BASE_H

#include "chipemu.h"

#include <cstdint>
#include <cstddef>
#include <vector>

namespace chipemu
{
namespace implementation
{

struct transdef
{
    const uint16_t gate;
    const uint16_t c1;
    const uint16_t c2;

    bool operator==(const transdef&) const;

    transdef(uint16_t a, uint16_t b, uint16_t c):
        gate(a),
        c1(b),
        c2(c)
    { }
};

struct chip_description
{
    const unsigned node_count;
    const bool *pullups;
    const unsigned transistor_count;
    const transdef *transistors;
    const uint16_t node_power;
    const uint16_t node_ground;
};

class nmos : public virtual chipemu::chip
{
private:

    void recalc_node(uint16_t);
    void recalc_nodes();

    std::vector<uint16_t> changed_queue;
    typename std::vector<uint16_t>::iterator changed_eating, changed_feeding;

    std::vector<uint16_t> current_group;
    typename std::vector<uint16_t>::iterator group_tail;

    enum class group_contains {
        nothing,
        high,
        pullup,
        pulldown,
        power,
        ground
    };

    group_contains group_current_value;

    void changed_queue_init();
    void group_init();
    void group_update_value(uint16_t);
    void group_add(uint16_t);

    uint16_t changed_pop();
    void changed_push(uint16_t id);
    bool changed_is_empty() const;
    void changed_clear();

    void group_setup(uint16_t);
    bool group_get_value() const;
    bool is_group_empty() const;
    uint16_t group_pop();

    uint16_t *node_addr(uint16_t id);
    const uint16_t *node_addr(uint16_t id) const;

    std::vector<uint16_t> nodes;
    std::vector<uint16_t> node_offsets;
    std::vector<uint16_t> change_order;

    void add_ordered_change(uint16_t);
    size_t change_count;
    void commit_ordered_changes();
    uint16_t desc_nodes_count;
    uint16_t desc_transistor_count;

protected:

    const uint16_t power;
    const uint16_t ground;

    nmos(const chip_description& desc);

    unsigned read_nodes(const uint16_t*, unsigned count) const noexcept;
    void write_nodes(const uint16_t*, unsigned count, unsigned value) noexcept;

public:

    bool get_node(unsigned) const noexcept;
    void set_node(unsigned, bool) noexcept;
    virtual unsigned node_count() const noexcept final;
    virtual unsigned transistor_count() const noexcept final;
    virtual void stabilize_network() noexcept override;
    virtual void recalc() noexcept override;

};

}
}

#endif
