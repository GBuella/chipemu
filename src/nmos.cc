
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <memory>
#include <tuple>
#include <limits>
#include <set>

#include <cinttypes>

#include "nmos.h"

using std::vector;
using std::pair;
using std::make_pair;
using std::tie;

namespace chipemu
{
namespace implementation
{
namespace
{

/* Internal node representation
 *
 * nmos::nodes         stores the nodes
 * nmos::node_offsets  stores the offset of each node in nmos::nodes
 *
 * nodes.data() + nodes_offsets[id] yields the starting address of
 *  the node with the specific id, in a uint16_t pointer
 *
 *  the internal structure of node at   uint16_t *node  :
 *  node[0] == flags
 *  node[1] == number of transistor gates this node controls
 *  node[2] == number of sibling nodes this node is connected to,
 *             via gates controlled by other nodes
 *
 *  starting at node[3] up to node[3 + node[1]*2 - 1] are the
 *   transistors controlled by the node described
 *   each transistor consists of two uint16_t values, which containt
 *   *) a flag in the least significant bit, representing the current
 *      state of the transistor ( 1 - on, 0 - off )
 *   *) the id of a node connected to a leg of the transistor,
 *      left shifted by one ( to leave the LSB free for the flag )
 *
 *  starting at node[3 + node[1]*2]
 *   up to node[2 + node[1]*2 + node[2] - 1] are offsets of
 *   transistor legs connecting this node to other nodes
 */

enum node_flags : uint16_t {
    node_is_pullup       = 0b00001,
    node_is_pulldown     = 0b00010,
    node_is_high         = 0b00100,
    node_in_changelist   = 0b01000,
    node_in_group        = 0b10000,
};

static constexpr uint16_t header_size = 3;

static uint16_t
node_gate_count(uint16_t *node)
{
    return node[1];
}

static uint16_t
node_sibling_count(uint16_t *node)
{
    return node[2];
}

static uint16_t*
node_gates(uint16_t *node)
{
    return node + header_size;
}

static uint16_t*
node_sibling_connectors(uint16_t *node)
{
    return node + header_size + (2 * node_gate_count(node));
}

static uint16_t
lookup_leg(uint16_t *nodes, uint16_t id,
           uint16_t gate_node_off, const struct transdef *tdef)
{
    uint16_t count = node_gate_count(nodes + gate_node_off);
    uint16_t *leg = node_gates(nodes + gate_node_off);
    for (uint16_t i = 0; i < count; ++i, leg += 2) {
        if (leg[0] >> 1 == tdef->c1 and leg[1] >> 1 == tdef->c2) {
            if (leg[0] >> 1 == id) {
                return leg + 1 - nodes;
            }
            else {
                return leg -  nodes;
            }
        }
    }
    throw std::logic_error("building nodes");
}


/* utility type, used while constructing the
 * internal nodes
 */
struct construct_node
{
    bool is_pullup;
    vector<transdef> gates;
    vector<const transdef*> sibling_connectors;

    construct_node(bool is_pullup): is_pullup(is_pullup) {}
};

} // anonym namespace

inline bool
transdef::operator==(const transdef& other) const
{
    return (gate == other.gate) &&
        ((c1 == other.c1 && c2 == other.c2)
         || (c1 == other.c2 && c2 == other.c1));
}

static vector<construct_node>
create_construct_nodes(const chip_description& desc)
{
    vector<construct_node> nodes;

    nodes.emplace_back(false);
    for (unsigned index = 0; index < desc.node_count; ++index) {
        nodes.emplace_back(desc.pullups[index]);
    }

    return nodes;
}


/* The current queue of changed nodes */
void
nmos::changed_queue_init()
{
    changed_queue.resize(node_count() * 32);
    changed_eating = changed_feeding = changed_queue.begin();
}

inline uint16_t
nmos::changed_pop()
{
    assert(changed_eating != changed_feeding);

    uint16_t id = *changed_eating;
    ++changed_eating;
    if (changed_eating == changed_queue.end()) {
        changed_eating = changed_queue.begin();
    }
    return id;
}

inline void
nmos::changed_push(uint16_t id)
{
    uint16_t *node = node_addr(id);

    if (*node & node_in_changelist) return;

    *changed_feeding = id;
    ++changed_feeding;
    if (changed_feeding == changed_queue.end()) {
        changed_feeding = changed_queue.begin();
    }
    *(node_addr(id)) |= node_in_changelist;
}

inline bool
nmos::changed_is_empty() const
{
    return changed_eating == changed_feeding;
}

inline void
nmos::changed_clear()
{
    changed_feeding = changed_eating;
}



/* The currently inspected group of nodes */
inline void
nmos::group_update_value(uint16_t flags)
{
    switch (group_current_value) {
        case group_contains::nothing:
            if (flags & node_is_high) {
                group_current_value = group_contains::high;
            }
        case group_contains::pullup:
            if (flags & node_is_pullup) {
                group_current_value = group_contains::pullup;
            }
        case group_contains::pulldown:
            if (flags & node_is_pulldown) {
                group_current_value = group_contains::pulldown;
            }
        case group_contains::power:
        case group_contains::ground:
        case group_contains::high:
            break;
    }
}

inline void
nmos::group_add(uint16_t id)
{
    uint16_t *node = node_addr(id);

    if (id == ground) {
        group_current_value = group_contains::ground;
    }
    else if (id == power) {
        if (group_current_value != group_contains::ground) {
            group_current_value = group_contains::power;
        }
    }
    else if (not (node[0] & node_in_group)) {
        node[0] |= node_in_group;
        node[0] &= ~node_in_changelist;
        *(group_tail++) = id;
        group_update_value(*node);
        uint16_t sibling_count = node_sibling_count(node);
        uint16_t *sibs = node_sibling_connectors(node);
        for (uint16_t i = 0; i < sibling_count; ++i) {
            uint16_t leg = nodes[sibs[i]];
            if (leg & 1) {
                group_add(leg >> 1);
            }
        }
    }
}


inline void
nmos::group_init()
{
    current_group.resize(node_count() * 32);   // how many should it be ??
}

inline void
nmos::group_setup(uint16_t id)
{
    group_tail = current_group.begin();
    group_current_value = group_contains::nothing;
    group_add(id);
}

inline bool
nmos::group_get_value() const
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

inline bool
nmos::is_group_empty() const
{
    return group_tail == current_group.cbegin();
}

inline uint16_t
nmos::group_pop()
{
    uint16_t id = *--group_tail;

    nodes[node_offsets[id]] &= ~node_in_group;
    return id;
}

static void
setup_transistors(const chip_description& desc,
				  vector<construct_node>& nodes)
{
    const transdef *tdef = desc.transistors;

    for (;tdef != desc.transistors + desc.transistor_count; ++tdef) {
        if (tdef->gate >= nodes.size()
                or tdef->c1 >= nodes.size()
                or tdef->c2 >= nodes.size())
        {
            throw std::out_of_range("node id");
        }
        if (std::none_of(desc.transistors, tdef,
                    [tdef](const transdef& t) { return t == *tdef; }))
        {
            nodes[tdef->gate].gates.push_back(*tdef);
            nodes[tdef->c1].sibling_connectors.push_back(tdef);
            nodes[tdef->c2].sibling_connectors.push_back(tdef);
        }
    }
}

static pair<vector<uint16_t>, vector<uint16_t>>
create_nodes(const vector<construct_node>& cnodes)
{
    vector<uint16_t> nodes;
    vector<uint16_t> node_offsets;

    node_offsets.push_back(0);
    for (uint16_t i = 1; i < cnodes.size(); ++i) {
        assert(std::numeric_limits<uint16_t>::max() > nodes.size());

        node_offsets.push_back(uint16_t(nodes.size()));
        nodes.push_back(cnodes[i].is_pullup ? node_is_pullup : 0);
        nodes.push_back(uint16_t(cnodes[i].gates.size()));
        nodes.push_back(uint16_t(cnodes[i].sibling_connectors.size()));
        nodes.resize(nodes.size() + cnodes[i].gates.size() * 2);
        nodes.resize(nodes.size() + cnodes[i].sibling_connectors.size());
    }
    for (uint16_t nodei = 1; nodei < cnodes.size(); ++nodei) {
        const construct_node *cnode = cnodes.data() + nodei;
        uint16_t *node = nodes.data() + node_offsets[nodei];
        uint16_t *leg = node_gates(node);
        for (uint16_t i = 0; i < cnode->gates.size(); ++i) {
            *leg++ = cnode->gates[i].c1 << 1;
            *leg++ = cnode->gates[i].c2 << 1;
        }
    }
    for (uint16_t nodei = 1; nodei < cnodes.size(); ++nodei) {
        const construct_node *cnode = cnodes.data() + nodei;
        uint16_t *node = nodes.data() + node_offsets[nodei];
        auto connectors = node_sibling_connectors(node);
        for (uint16_t i = 0; i < cnode->sibling_connectors.size(); ++i) {
            auto tdef = cnode->sibling_connectors[i];
            uint16_t gn_off = node_offsets[tdef->gate];
            connectors[i] = lookup_leg(nodes.data(), nodei, gn_off, tdef);
        }
    }

    return make_pair(node_offsets, nodes);
}

inline uint16_t*
nmos::node_addr(uint16_t id)
{
    return nodes.data() + node_offsets[id];
}

inline const uint16_t*
nmos::node_addr(uint16_t id) const
{
    return nodes.data() + node_offsets[id];
}

bool
nmos::get_node(unsigned id) const noexcept
{
    if (id > 0 and id <= node_count()) {
        return (*(node_addr(id))) & node_is_high;
    }
    else {
        return false;
    }
}

void
nmos::set_node(unsigned id, bool high) noexcept
{
    if (id > 0 and id <= node_count()) {
        uint16_t *node = node_addr(id);

        if ((*node & node_is_pullup) and not high) {
            *node &= ~node_is_pullup;
            *node |= node_is_pulldown;
        }
        else if (high and not (*node & node_is_pullup)) {
            *node |= node_is_pullup;
            *node &= ~node_is_pulldown;
        }
        else return;

        changed_push(id);
    }
}

unsigned
nmos::read_nodes(const uint16_t* ids, unsigned count) const noexcept
{
    unsigned value = 0;

    for (unsigned index = 0; index < count; ++index) {
        value = (value << 1) + (get_node(ids[index]) ? 1 : 0);
    }
    return value;
}

void
nmos::write_nodes(const uint16_t* ids,
                       unsigned count,
                       unsigned value) noexcept
{
    const uint16_t *id = ids + count;

    while (id-- != ids) {
        set_node(*id, value & 1);
        value >>= 1;
    }
}

nmos::nmos(const chip_description& desc):
    power(desc.node_power),
    ground(desc.node_ground)
{
    auto cnodes = create_construct_nodes(desc);
    setup_transistors(desc, cnodes);
    tie(node_offsets, nodes) = create_nodes(cnodes);
    desc_nodes_count = desc.node_count;
    desc_transistor_count = desc.transistor_count;
    changed_queue_init();
    change_order.resize(node_count());
    group_init();
    node_addr(power)[0] |= node_is_high;
}

unsigned
nmos::node_count() const noexcept
{
    return desc_nodes_count;
}

unsigned
nmos::transistor_count() const noexcept
{
    return desc_transistor_count;
}

inline void
nmos::add_ordered_change(uint16_t id)
{
    change_order[change_count++] = id;
    std::push_heap(change_order.begin(),
            change_order.begin() + change_count,
            std::greater<uint16_t>());
}

inline void
nmos::commit_ordered_changes()
{
    while (change_count > 0) {
        std::pop_heap(change_order.begin(),
            change_order.begin() + change_count,
            std::greater<uint16_t>());
        changed_push(change_order[--change_count]);
    }
}

inline void
nmos::recalc_node(uint16_t id)
{
    uint16_t *node = node_addr(id);

    if (not (node[0] & node_in_changelist)) return;
    node[0] &= ~node_in_changelist;
    group_setup(id);
    uint16_t high_value = group_get_value() ? node_is_high : 0;
    while (not is_group_empty()) {
        uint16_t gid = group_pop();
        uint16_t *node = node_addr(gid);

        if ((*node & node_is_high) != high_value) {
            *node ^= node_is_high;
            uint16_t gate_count = node_gate_count(node);
            uint16_t *leg = node_gates(node);
            change_count = 0;
            for (uint16_t i = 0; i < gate_count; ++i) {
                leg[0] ^= 1;
                leg[1] ^= 1;
                uint16_t c1 = leg[0] >> 1;
                uint16_t c2 = leg[1] >> 1;

                if (high_value) {
                    uint16_t flags_c1 = node_addr(c1)[0];
                    uint16_t flags_c2 = node_addr(c2)[0];

                    if ((flags_c1 & node_is_high) == (flags_c2 & node_is_high)) {
                        leg += 2;
                        continue;
                    }
                    if (c1 != power and c1 != ground) {
                        add_ordered_change(c1);
                    }
                    else {
                        add_ordered_change(c2);
                    }
                }
                else {
                    add_ordered_change(c1);
                    add_ordered_change(c2);
                }
                leg += 2;
            }
            commit_ordered_changes();
        }
    }
}

inline void
nmos::recalc_nodes()
{
    while (not changed_is_empty()) {
        recalc_node(changed_pop());
    }
}

void
nmos::stabilize_network() noexcept
{
    for (uint16_t i = 1; i <= node_count(); ++i) {
        changed_push(i);
    }
    recalc_nodes();
}

void
nmos::recalc() noexcept
{
    recalc_nodes();
}

}
}
