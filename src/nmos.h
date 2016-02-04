
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */

#ifndef CHIPEMU_CHIP_BASE_H
#define CHIPEMU_CHIP_BASE_H

#include "chipemu.h"

#include <vector>
#include <set>

namespace chipemu
{
namespace implementation
{

template<typename node_id>
struct transdef
{
    const node_id gate;
    const node_id c1;
    const node_id c2;

    bool operator==(const transdef&) const;

    transdef(node_id a, node_id b, node_id c):
        gate(a),
        c1(b),
        c2(c)
    { }
};

template<typename node_id>
struct chip_description
{
    const unsigned node_count;
    const bool *pullups;
    const unsigned transistor_count;
    const transdef<node_id> *transistors;
    const node_id node_power;
    const node_id node_ground;
};

template<typename node_id>
class nmos : public virtual chipemu::chip
{
private:

    void recalc_node(node_id);
    void recalc_nodes();

    std::vector<node_id> changed_queue;
    typename std::vector<node_id>::iterator changed_eating, changed_feeding;

    std::vector<node_id> current_group;
    typename std::vector<node_id>::iterator group_tail;

    enum class group_contains {
        nothing,
        high,
        pullup,
        pulldown,
        power,
        ground
    };

    group_contains group_current_value;

protected:

    struct transistor
    {
        bool on;
        node_id c1;
        node_id c2;

        transistor(const transdef<node_id>* ctor_def);
    };

    struct node
    {
        bool pullup;
        bool pulldown;
        bool value;
        bool is_member_of_current_group;
        std::vector<node_id> gates;
        std::vector<node_id> c1c2s;
        std::set<node_id> dependants;
        std::set<node_id> left_dependants;

        node(bool is_pullup);

    };

private:

    void changed_queue_init();
    void group_init();
    void group_update_value(node_id);
    void group_add_siblings(const struct transistor&, node_id);
    void group_add(node_id);

    void create_nodes(const chip_description<node_id>&);
    void setup_transistors(const chip_description<node_id>&);
    void setup_dependants(const chip_description<node_id>&);

    node_id changed_pop();
    void changed_push(node_id id);
    void changed_push(const std::set<node_id> &ids);
    bool changed_is_empty() const;
    void changed_clear();

    void group_setup(node_id);
    bool group_get_value() const;
    bool is_group_empty() const;
    node_id group_pop();

    std::vector<node> nodes;
    std::vector<transistor> transistors;

protected:

    const node_id power;
    const node_id ground;

    nmos(const chip_description<node_id>& desc);

    unsigned read_nodes(const node_id*, unsigned count) const noexcept;
    void write_nodes(const node_id*, unsigned count, unsigned value) noexcept;

public:

    bool get_node(unsigned) const noexcept;
    void set_node(unsigned, bool) noexcept;
    virtual unsigned node_count() const noexcept final;
    virtual unsigned transistor_count() const noexcept final;
    virtual void stabilize_network() noexcept override;
    virtual void recalc() noexcept override;

    virtual ~nmos();
};

}
}

#endif
