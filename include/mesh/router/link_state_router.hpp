/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_LINK_STATE_ROUTER_HPP
#define IPASS_LINK_STATE_ROUTER_HPP

#include <mesh/router.hpp>
#include <link_state/calculator.hpp>

using link_state::calculator;
using link_state::node;
namespace mesh { //Todo: Pagination for shitloads of nodes
    namespace routers {
        /**
         * \addtogroup routers
         * @{
         */

        /**
         * \brief Router implementation using the link_state algorithm
         *
         * To conserve processing time, this router only calculates when a message needs to be routed. This may increase delay in messages.
         * Routing information in messages is sent in data, and formatted as follows:
         * 1st byte: neighbour1.node_id
         * 2nd byte: neighbour1.connection_cost
         * 3rd byte: neighbour2.node_id
         * etc...
         */
        class link_state : public router {
            bool is_updated = false;
            calculator<node_id, uint8_t, 5, 10> ls_calc;

            /**
             * \brief Save updated routing information to the link_state calculator
             *
             * Adds a new node if no node with the sender id was found
             * @param other Node to update information for
             * @param message Message to use for updating, see class documentation for the format
             */
            void graph_update_other(const node_id &other, const message &message);

            /**
             * \brief Write the currently known neighbour nodes to a message.
             *
             * This method discards any data that was already in the message
             * @param message Message to add information to
             */
            void fill_update_message(message &message);

        public:
            /**
             * \brief Create a link state router
             *
             * @param connectivity Connectivity adapter to send messages through
             */
            explicit link_state(connectivity_adapter &connectivity);

            /**
             * \brief Update node graph with current neighbours from connectivity adapter
             *
             * Retrieves the current neighbours using get_neighbours, and updates the graph with their information. Costs need to be calculated here
             */
            void update_neighbours() override;

            /**
             * \brief Send currently known neighbour information to all nodes in the network
             *
             * Uses unicast_all, assuming that every node will pass the routing information on to it's neighbours
             */
            void send_update() override;

            /**
             * \brief Send initial update, requesting updates from all other nodes
             */
            void initial_update() override;

            /**
             * \brief Handle a routing message
             *
             * This method updates the node graph when an update message is received, even when the message is not directed at this node
             * @param message Message to handle
             */
            void on_routing_message(message &message) override;

            /**
             * \brief Calculate next hop for a given destination
             *
             * Checks if the algorithm has been run since the last node graph change.
             * If it hasn't, it is calculated, otherwise the next_hop is found through the graph.
             * @param receiver Final destination to get next hop for
             * @return The next hop, or 0 if none was found
             */
            node_id get_next_hop(const node_id &receiver) override;

            /**
             * \brief Get the LinkState Calculator for this router
             *
             * @return The Calculator
             */
            calculator<node_id, uint8_t, 5, 10> &get_calculator();

        };

        /**
         * @}
         */
    }
}

#endif //IPASS_LINK_STATE_ROUTER_HPP
