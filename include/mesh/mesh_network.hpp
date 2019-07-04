/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_MESH_NETWORK_HPP
#define IPASS_MESH_NETWORK_HPP


#include <mesh/connectivity_adapter.hpp>
#include <mesh/router.hpp>


/**
 * \defgroup mesh_networking Mesh Networking Library
 * \brief Implementation of a mesh network, using routing
 *
 * Connectivity_adapter and network_router can be set to different implementations, allowing for custom routing algorithms, or connection methods.
 * When using the router base class, the network will just have direct connections.
 */


/**
 * \defgroup addons Mesh Addons
 * \ingroup mesh_networking
 * \brief Mesh network addons
 *
 * Example addons for debugging purposes, or to provide extended functionality to a node.
 */

namespace mesh {

    /**
     * \addtogroup mesh_networking
     * @{
     */

    /**
     * \brief Main class of a mesh_network.
     *
     * handles discovery messages, keepalives and routing through the given router.
     * Because of the abstraction of connectivity_adapter, this class can work with any connection method.
     */
    class mesh_network {
        connectivity_adapter &connection;
        router &network_router;

        std::array<node_id, 10> blacklist = {0};
        size_t blacklist_size = 0;

        uint32_t update_count = 0;
        uint32_t keepalive_interval = 1000;


        /**
         * \brief Checks if a node is blacklisted from direct connections.
         *
         * @param id Node id of the node to check for
         * @return True if the node is blacklisted
         */
        bool is_blacklisted(const node_id &id) {
            for (const node_id &checkid : blacklist) {
                if (id == checkid) {
                    return true;
                }
                if (checkid == 0) {
                    break;
                }
            }
            return false;
        }


    public:
        /**
         * \brief Construct a mesh_network
         *
         * Note that for a network to work properly, all nodes should implement the same router and connection method.
         * @param connection Connectivity_adapter to use for this network
         * @param networkrouter Routing protocol to use for this network
         */
        mesh_network(connectivity_adapter &connection, router &networkrouter) :
                connection(
                        connection),
                network_router(networkrouter) {}

        /**
         * \brief Add the given nodes to the direct connection blacklist
         *
         * The blacklist can be used to force certain nodes to communicate through routing.
         * This can be used to reduce load on the network, when many nodes are close in proximity.
         * This can also be used to show that routing works in your code ;).
         * @tparam n Amount of nodes to add to the blacklist
         * @param list Array of node_id's to add
         */
        template<size_t n>
        void add_blacklist(std::array<node_id, n> list) {
            for (const node_id &node: list) {
                blacklist[blacklist_size++] = node;
            }
        }

        /**
         * \brief Broadcast a discovery message to the network
         *
         * Normally, the update function calls this method periodically, so it doesn't need to be called
         */
        void discover() {
            message message = {DISCOVERY::PRESENT, 0, connection.id, 0,
                               0};
            connection.send(message);
        }

        /**
         * \brief Check for new received messages.
         *
         * This method automatically handles any discovery/routing messages.
         * Anything that remains will be put in the uncaught array, so the caller can handle it.
         *
         * @param uncaught Array reference to put any uncaught messages in
         * @return The count of uncaught messages
         */
        uint8_t check_new_messages(std::array<message, 10> &uncaught) {
            uint8_t index = 0;
            while (connection.has_message()) {
                message msg = connection.next_message();
                if (!connection.is_new_message(msg)) { //message already handled
                    continue;
                }
                if (msg.receiver == connection.id ||
                    msg.receiver == 0) { //Message is for us, or broadcast, take it
                    if (!handleMessage(msg)) {
                        uncaught[index++] = msg;
                    }
                } else { //Not for us, todo relay message (through routing)
                    uint8_t next_hop = 0;
                    if (connection.connection_state(msg.receiver) != ACCEPTED) {
                        next_hop = network_router.get_next_hop(msg.receiver);
                    } else {
                    }
                    if (!connection.send(msg, next_hop)) {
                        network_router.update_neighbours();
                    }
                }

            }
            return index;
        }

        /**
         * \brief Handles discovery and keepalives
         *
         * Sends a keepalive, and a discovery message every "keepalive_interval" updates
         * TODO: seperate this
         */
        void update() {
            if (update_count++ > keepalive_interval) {
                update_count = 0;
                message keepalive = {
                        DISCOVERY::NO_OPERATION,
                        0,
                        connection.id,
                        0
                };

                unicast_all_close_if_fail(keepalive);

            }
            if (update_count == keepalive_interval / 2) {
                discover();
            }
        }

        /**
         * \brief Unicast a message to a node, and close it's connection immediately if the transmit fails
         *
         * @param msg Message to send
         * @param next_hop First hop to send it through
         */
        void unicast_close_if_fail(message &msg, const node_id &next_hop = 0) {
            if (!connection.send(msg, next_hop)) {
                connection.remove_direct_connection(next_hop != 0 ? next_hop : msg.receiver);
                network_router.send_update();
            }
        }

        /**
         * \brief Unicast a message to all connected neighbours. Close every connection that fails to transmit.
         *
         * Is used for keepalives.
         * @param msg Message to send
         */
        void unicast_all_close_if_fail(message &msg) {
            size_t count = connection.get_neighbour_count();
            node_id failed[count];

            for (size_t i = 0; i < count; i++) {
                failed[i] = 0;
            }

            if (!connection.send_all(msg, failed)) {
                for (size_t i = 0; i < connection.get_neighbour_count(); i++) {
                    if (failed[i] == 0) {
                        continue;
                    }
                    connection.remove_direct_connection(failed[i]);
                }
                network_router.send_update();
            }
        }

        /**
         * \brief Transmit a message to a receiver, using the network_router
         *
         * Note that this function ignores failure
         * @param msg message to send, receiver should be set on this message
         */
        void sendMessage(message &msg) {
            uint8_t nextAddress = network_router.get_next_hop(msg.receiver);
            if (nextAddress == 0) {
                return;
            }
            msg.sender = connection.id;
            if (!connection.send(msg, nextAddress)) {
//                router.update_neighbours();
            }
        }

        /**
         * \brief Handle an incoming message
         *
         * Checks for blacklisted sender, handles routing messages and discovery messages.
         * @param msg message to handle
         * @return True if the message was handled, false if it still needs to be handled by the caller
         */
        bool handleMessage(message &msg) {
            if ((msg.type & 0x10) > 0) { //This is a routing message
                network_router.on_routing_message(msg);
            }

            if ((msg.type & 0x20) > 0) {
                return false;
            }

            if (is_blacklisted(msg.sender)) {
                return true;
            }


            switch (msg.type) {
                case DISCOVERY::PRESENT:
                    if (connection.connection_state(msg.sender) == DISCONNECTED) {
                        if (connection.discovery_present_received(msg)) {
                            message connectMessage = {DISCOVERY::RESPOND, 0,
                                                      connection.id,
                                                      msg.sender, 0};
                            unicast_close_if_fail(connectMessage);
                        }
                    }
                    break;
                case DISCOVERY::RESPOND: {
                    if (connection.discovery_respond_received(msg)) {

                        message finishMessage = {DISCOVERY::ACCEPT, 0, connection.id,
                                                 msg.sender, 0};
                        if (connection.send(finishMessage)) {
                            network_router.update_neighbours();
                        }

                    } else {
                        message finishMessage = {DISCOVERY::DENY, 0, connection.id,
                                                 msg.sender, 0};
                        connection.send(finishMessage);
                    }
                    break;
                }
                case DISCOVERY::ACCEPT:
                    connection.discovery_accept_received(msg);
                    network_router.initial_update();
                    break;
                case DISCOVERY::DENY:
                    if (msg.receiver == connection.id) {
                        connection.remove_direct_connection(msg.sender);
                    }
                    break;
                case DISCOVERY::NO_OPERATION:
                    break;
                default:
                    return false;
                    break;
            }
            return true;
        }

        /**
         * \brief Get the connectivity_adapter for this network
         *
         * Mainly used by the LCD status display
         * @return The Connectivity adapter
         */
        connectivity_adapter &get_connection() const {
            return connection;
        }

        /**
         * \brief Get the network_router for this network
         *
         * Mainly used by the LCD status display
         * @return The network router
         */
        router &get_router() const {
            return network_router;
        }
    };


    /**
     * @}
     */
}
#endif //IPASS_MESH_NETWORK_HPP
