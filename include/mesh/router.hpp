/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_MESH_ROUTER_HPP
#define IPASS_MESH_ROUTER_HPP


#include <mesh/connectivity_adapter.hpp>

namespace mesh {
    /**
     * \defgroup routers Mesh Router Implementations
     * \ingroup mesh_networking
     * \brief Provided router implementations for mesh_networks
     */

    /**
     * \addtogroup mesh_networking
     * @{
     */
    /**
     * \brief Class for routing implementations
     *
     * This class needs to be extended to implement routing algorithms.
     * When using the base class as router, it will act as a dummy,  allowing for routing-less networks.
     */
    class router {
    protected:
        /// \brief Connectivity Adapter to use for this router
        connectivity_adapter &connectivity;
    public:
        /**
         * Create a router using the given connectivity adapter
         * @param connectivity  Connectivity adapter
         */
        router(connectivity_adapter &connectivity) : connectivity(connectivity) {}

        /**
         * \brief Cache neighbour information from connectivity adapter.
         *
         * This method doesn't need to transmit anything.
         * When caching neighbours isn't needed, this function can just be left.
         */
        virtual void update_neighbours() {};

        /**
         * \brief Send update message into network, containing the current neighbour information
         *
         * Any specific routing-information can also be sent here
         */
        virtual void send_update() {};

        /**
         * \brief Send the first update message, after connecting
         *
         * Other nodes should respond to these messages with their own neighbour information.
         */
        virtual void initial_update() {};

        /**
         * \brief Handle a routing message that was received from another node
         *
         * @param message Message to handle
         */
        virtual void on_routing_message(message &message) {};

        /**
         * \brief Run the routing algorithm, and calculate the next hop for the given node_id
         * @param receiver Node_id to find the next hop for
         * @return The id of the next_hop, or 0 if none was found
         */
        virtual node_id get_next_hop(const node_id &receiver) {
            return 0;
        };
    };

    /**
     * @}
     */
}

#endif //IPASS_MESH_ROUTER_HPP
