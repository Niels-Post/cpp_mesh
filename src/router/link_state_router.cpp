/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#include <mesh/router/link_state_router.hpp>

namespace mesh {
    namespace routers {
        void link_state::graph_update_other(const node_id &other, const message &message) {
            std::array<node_id, 5> edges = {};
            std::array<uint8_t, 5> costs = {};


            for (size_t i = 0; i < message.dataSize / 2; i++) {
                edges[i] = message.data[i * 2];
                costs[i] = message.data[i * 2 + 1];
            }


            ls_calc.insert_replace({other, edges, costs});
            is_updated = false;
        }

        void link_state::fill_update_message(message &message) {
            auto &me = ls_calc.get_node(0);
            message.dataSize = uint8_t(me.edge_count * 2);
            for (size_t i = 0; i < me.edge_count; i++) {
                message.data[i * 2] = me.edges[i];
                message.data[i * 2 + 1] = me.edge_costs[i];
            }
            is_updated = false;
        }

        link_state::link_state(connectivity_adapter &connectivity) : router(connectivity),
                                                                     ls_calc(connectivity.id) {
            update_neighbours();
        }

        void link_state::update_neighbours() {
//            connectivity.status();
            size_t count = connectivity.get_neighbour_count();
            uint8_t neighbours[count];
            connectivity.get_neighbours(neighbours);
            auto &me = ls_calc.get_node(0);
            me.edge_count = uint8_t(count);
            for (size_t i = 0; i < count; i++) {
                me.edges[i] = neighbours[i];
                me.edge_costs[i] = 1; //Todo actual cost
            }
        }

        void link_state::send_update() {
            update_neighbours();
            message update_message(
                    LINK_STATE_ROUTING::UPDATE,
                    0,
                    connectivity.id,
                    0
            );
            fill_update_message(update_message);
            connectivity.send_all(update_message);

        }

        void link_state::initial_update() {
            update_neighbours();
            message message = {
                    LINK_STATE_ROUTING::UPDATE_REQUEST,
                    0,
                    connectivity.id,
                    0
            };
            fill_update_message(message);
            connectivity.send_all(message);
        }

        void link_state::on_routing_message(message &message) {
            switch (message.type) {
                case LINK_STATE_ROUTING::UPDATE_REQUEST: {
                    graph_update_other(message.sender, message);
                    send_update();
                    break;
                }
                case LINK_STATE_ROUTING::UPDATE: {
                    graph_update_other(message.sender, message);
                    break;
                }
                default:
                    break;
            }

            connectivity.send_all(message);

        }

        node_id link_state::get_next_hop(const node_id &receiver) {
            if (!is_updated) {
                is_updated = true;
                ls_calc.setup();
                ls_calc.loop();
                ls_calc.cleanup();
            }
            return ls_calc.get_next_hop(receiver);
        }

        calculator<node_id, uint8_t, 5, 10> &link_state::get_calculator() {
            return ls_calc;
        }


    }
}