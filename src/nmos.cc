
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */

#include <cassert>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <functional>
#include <set>
#include <memory>

#include <cstdio>

#include "nmos.h"

using std::set;
using std::vector;

namespace chipemu
{
namespace implementation
{

template<typename node_id>
inline bool
transdef<node_id>::operator==(const transdef& other) const
{
    return (gate == other.gate) &&
        ((c1 == other.c1 && c2 == other.c2)
         || (c1 == other.c2 && c2 == other.c1));
}

template<typename node_id>
void
nmos<node_id>::create_nodes(const chip_description<node_id>& desc)
{
    nodes.emplace_back(false);
    for (unsigned index = 0; index < desc.node_count; ++index) {
        nodes.emplace_back(desc.pullups[index]);
    }
}

template<typename node_id>
nmos<node_id>::transistor::transistor(const transdef<node_id> *ctor_def):
    on(false),
    c1(ctor_def->c1),
    c2(ctor_def->c2)
{ }

template<typename node_id>
nmos<node_id>::node::node(bool is_pullup):
    pullup(is_pullup),
    pulldown(false),
    value(false)
{ }


/* The current queue of changed nodes */
template<typename node_id>
void
nmos<node_id>::changed_queue_init()
{
    changed_queue.resize(node_count() * 32);
    changed_eating = changed_feeding = changed_queue.begin();
}

template<typename node_id>
inline node_id
nmos<node_id>::changed_pop()
{
    assert(changed_eating != changed_feeding);

    node_id result = *changed_eating;
    ++changed_eating;
    if (changed_eating == changed_queue.end()) {
        changed_eating = changed_queue.begin();
    }
    return result;
}

template<typename node_id>
inline void
nmos<node_id>::changed_push(node_id id)
{
    *changed_feeding = id;
    ++changed_feeding;
    if (changed_feeding == changed_queue.end()) {
        changed_feeding = changed_queue.begin();
    }
}

template<typename node_id>
inline void
nmos<node_id>::changed_push(const set<node_id> &ids)
{
    for (auto id : ids) {
        changed_push(id);
    }
}

template<typename node_id>
inline bool
nmos<node_id>::changed_is_empty() const
{
    return changed_eating == changed_feeding;
}

template<typename node_id>
inline void
nmos<node_id>::changed_clear()
{
    changed_feeding = changed_eating;
}



/* The currently inspected group of nodes */
template<typename node_id>
inline void
nmos<node_id>::group_update_value(node_id id)
{
    switch (group_current_value) {
        case group_contains::nothing:
            if (nodes[id].value) {
                group_current_value = group_contains::high;
            }
        case group_contains::pullup:
            if (nodes[id].pullup) {
                group_current_value = group_contains::pullup;
            }
        case group_contains::pulldown:
            if (nodes[id].pulldown) {
                group_current_value = group_contains::pulldown;
            }
        case group_contains::power:
        case group_contains::ground:
        case group_contains::high:
            break;
    }
}

template<typename node_id>
inline void
nmos<node_id>::group_add_siblings(const struct transistor& transistor, node_id id)
{
    if (transistor.on) {
        if (transistor.c1 == id) {
            group_add(transistor.c2);
        }
        else {
            group_add(transistor.c1);
        }
    }
}

template<typename node_id>
inline void
nmos<node_id>::group_add(node_id id)
{
    if (nodes[id].is_member_of_current_group) {
        return;
    }
    else if (id == ground) {
        group_current_value = group_contains::ground;
    }
    else if (id == power) {
        if (group_current_value != group_contains::ground) {
            group_current_value = group_contains::power;
        }
    }
    else {
        nodes[id].is_member_of_current_group = true;
        *(group_tail++) = id;
        group_update_value(id);
        for (auto c : nodes[id].c1c2s) {
            group_add_siblings(transistors[c], id);
        }
    }
}


template<typename node_id>
inline void
nmos<node_id>::group_init()
{
    current_group.resize(node_count() * 32);   // how many should it be ??
}

template<typename node_id>
inline void
nmos<node_id>::group_setup(node_id id)
{
    group_tail = current_group.begin();
    group_current_value = group_contains::nothing;
    group_add(id);
}

template<typename node_id>
inline bool
nmos<node_id>::group_get_value() const
{
    switch (group_current_value) {
        case group_contains::power:
        case group_contains::pullup:
        case group_contains::high:
            return true;
        case group_contains::nothing:
        case group_contains::pulldown:
        case group_contains::ground:
            return false;
    }
    __builtin_unreachable();
}

template<typename node_id>
inline bool
nmos<node_id>::is_group_empty() const
{
    return group_tail == current_group.cbegin();
}

template<typename node_id>
inline node_id
nmos<node_id>::group_pop()
{
    nodes[*--group_tail].is_member_of_current_group = false;
    return *group_tail;
}


template<typename node_id>
inline void
nmos<node_id>::setup_transistors(const chip_description<node_id>& desc)
{
    const transdef<node_id> *tdef = desc.transistors;

    for (;tdef != desc.transistors + desc.transistor_count; ++tdef) {
        if (tdef->gate >= nodes.size()
                or tdef->c1 >= nodes.size()
                or tdef->c2 >= nodes.size())
        {
            throw std::out_of_range("node id");
        }
        if (std::none_of(desc.transistors, tdef,
                    [tdef](const transdef<node_id>& t) { return t == *tdef; }))
        {
            node_id id = node_id(transistors.size());

            transistors.push_back(tdef);
            nodes[tdef->gate].gates.push_back(id);
            nodes[tdef->c1].c1c2s.push_back(id);
            nodes[tdef->c2].c1c2s.push_back(id);
        }
    }
}

template<typename node_id>
inline void
nmos<node_id>::setup_dependants(const chip_description<node_id>& desc)
{
    for (node_id id = 0; id < nodes.size(); ++id) {
        struct node& node = nodes[id];

        for (node_id gate : node.gates) {
            node_id c1 = transistors[gate].c1;
            node_id c2 = transistors[gate].c2;

            if (desc.node_power != c1 and desc.node_ground != c1) {
                node.dependants.insert(c1);
                node.left_dependants.insert(c1);
            }
            else {
                node.left_dependants.insert(c2);
            }
            if (desc.node_power != c2 and desc.node_ground != c2) {
                node.dependants.insert(c2);
            }
        }
    }
}

template<typename node_id>
bool
nmos<node_id>::get_node(unsigned id) const noexcept
{
    if (id < node_count()) {
        return nodes[id].value;
    }
    else {
        return false;
    }
}

template<typename node_id>
void
nmos<node_id>::set_node(unsigned id, bool value) noexcept
{
    if (id < node_count()) {
        if (nodes[id].pullup != value) {
            nodes[id].pullup = value;
            nodes[id].pulldown = not value;
            changed_push(id);
        }
    }
}

template<typename node_id>
unsigned
nmos<node_id>::read_nodes(const node_id* ids, unsigned count) const noexcept
{
    unsigned value = 0;

    for (unsigned index = 0; index < count; ++index) {
        value = (value << 1) + (get_node(ids[index]) ? 1 : 0);
    }
    return value;
}

template<typename node_id>
void
nmos<node_id>::write_nodes(const node_id* ids,
                       unsigned count,
                       unsigned value) noexcept
{
    const node_id *id = ids + count;

    while (id-- != ids) {
        set_node(*id, value & 1);
        value >>= 1;
    }
}

template<typename node_id>
nmos<node_id>::nmos(const chip_description<node_id>& desc):
    power(desc.node_power),
    ground(desc.node_ground)
{
    create_nodes(desc);
    setup_transistors(desc);
    setup_dependants(desc);
    changed_queue_init();
    group_init();
}

template<typename node_id>
unsigned
nmos<node_id>::node_count() const noexcept
{
    return nodes.size();
}

template<typename node_id>
unsigned
nmos<node_id>::transistor_count() const noexcept
{
    return transistors.size();
}

template<typename node_id>
inline void
nmos<node_id>::recalc_node(node_id id)
{
    bool value;

    group_setup(id);
    value = group_get_value();
    while (not is_group_empty()) {
        struct node& node = nodes[group_pop()];

        if (node.value != value) {
            node.value = value;
            for (node_id gate : node.gates) {
                transistors[gate].on = value;
            }
            changed_push(value ? node.left_dependants : node.dependants);
        }
    }
}

template<typename node_id>
inline void
nmos<node_id>::recalc_nodes()
{
    while (not changed_is_empty()) {
        recalc_node(changed_pop());
    }
}

template<typename node_id>
void
nmos<node_id>::stabilize_network() noexcept
{
    for (node_id id = 0; id < node_count(); ++id) {
        changed_push(id);
    }
    recalc_nodes();
}

template<typename node_id>
void
nmos<node_id>::recalc() noexcept
{
    recalc_nodes();
}

template<typename node_id>
nmos<node_id>::~nmos<node_id>()
{
}

template class nmos<uint16_t>;

}
}
