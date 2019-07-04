/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_MESH_CONNECTIVITY_ADAPTER_HPP
#define IPASS_MESH_CONNECTIVITY_ADAPTER_HPP

#include <mesh/message.hpp>
#include <mesh/definitions.hpp>

namespace mesh {
    /**
     * \defgroup connectivity_adapters Mesh Connectivity Implementations
     * \ingroup mesh_networking
     * \brief Set of currently implemented mesh connectivity adapters
     *
     * Each adapter enables mesh networking over a specific connection method
     */

    /**
     * \addtogroup mesh_networking
     * @{
     */


    /**
     * \brief Base abstract class for connectivity, extend this to implement mesh_networking for a custom connection method
     *
     * For a good implementation of this ADT, all virtual methods should be overidden.
     * This class also contains implemented methods for managing message history, and checking connection state before sending messages.
     */
    class connectivity_adapter {
        uint16_t previous_messages[20] = {0};
        uint8_t previous_messages_count = 0;
        uint8_t current_message_id = 0;
    protected:

        /**
         * \brief Add the next message id to a given message.
         *
         * Message ID's are automatically incremented, and the receiver can use this to check if a given message was already received before.
         * A message ID is not added if the message originates from another node, or if a message id was already added to it.
         * @param msg Message to add an ID to.
         */
        void add_message_id(message &msg);

        /**
         * \brief Adds any connection-method specific data.
         *
         * Each message had a 2-byte space for connection-method specific data (like RF channels).
         * This information should be added here. If connection specific data is not necessary, this function can be ignored
         * @param msg Message to add data to
         * @param next_hop The calculated next hop, in case this is needed
         */
        virtual void add_connection_data(message &msg, node_id &next_hop) {};

        /**
         * \brief Implementation of message sending. send_implementation should transmit the given bytes to the node with id "id"
         *
         * Note that when id is 0, the message should be assumed to be a broadcast message, and treated as such.
         * send_implementation does not need to check for a connection state to a specific node, since this has already been done by send/send_all
         * @param id Node id to send the message to
         * @param data Pointer to the data to send
         * @param size Size of the data to send
         * @return Wether the transmission was successful
         */
        virtual bool send_implementation(node_id &id, uint8_t *data, size_t size) = 0;

    public:
        /**
         * \brief Node ID of the node running this instance
         */
        node_id id;

        // All methods implemented in connectivity_adapter itself

        /**
         * \brief Constructor of a connectivity adapter
         *
         * Does nothing else than set address
         * @param my_id The node id of this node
         */
        explicit connectivity_adapter(const node_id &my_id);

        /**
         * \brief Remove all messages from history that were sent by a specific node.
         *
         * This should be used when a node disconnects, since it might have powered down, and it's message id's will start over
         * @param id ID of the node to forget message history for
         */
        void forget_message_history_for(const node_id &id);

        /**
         * \brief Check if a message was already received before
         *
         * Uses the sender id, combined with the message id.
         * Message id's overflowing should be no problem, since there are only 20 buffered messages
         * @param msg Message to check
         * @return True if the message hasn't been received before, false otherwise
         */
        bool is_new_message(const message &msg);

        /**
         * \brief Send a single message to a receiver through next_hop, using send_implementation.
         *
         * If next_hop is 0, the message is sent directly to the message's receiver.
         * If both next_hop and the message's receiver are 0, the message is assumed to be a broadcast, note that send_implementation needs to handle this properly
         * @param message Message to send
         * @param next_hop First hop to pass through on the way to the message's receiver
         * @return True if sending was successful, false otherwise
         */


        bool send(message &message, node_id next_hop = 0);

        /**
         * \brief Sends a single message to all directly connected nodes. Addresses the message to each receiver.
         *
         * Returns false if any of the transmissions had failed. For every failed transmission, the receiver's address is placed in "failed_addresses".
         * Make sure failed_addresses has at least as much bytes of space as the current neighbour count
         * @param msg Message to send
         * @param failed_addresses Pointer to a memory location to store failed connections in
         * @return True if all transmissions were successful, false if any of them failed
         */
        bool send_all(message &msg, node_id *failed_addresses = nullptr);


        // Connection-type specific methods

        /**
         * \brief Check if the connection method has a message available
         *
         * @return True if there is a message available, false otherwise
         */
        virtual bool has_message() = 0;

        /**
         * \brief Gets the first available message, this should be done in FIFO order.
         *
         * Mesh_network checks if there are messages available before retrieving them, so return value when there is no message available does not matter
         * Note that since some time elapses between message requests, buffering messages might be necessary, to prevent buffer overrun on a connection adapter's registers.
         * @return
         */
        virtual message next_message() = 0;

        /**
         * \brief Retrieve current connection state of a direct connection to the node with id "id".
         *
         * Note that an indirect connection (through routing)  should always return mesh::DISCONNECTED here.
         * @param id
         * @return
         */
        virtual mesh_connection_state connection_state(const node_id &id) = 0;

        /**
         * \brief Get the amount of neighbours that are currently in state mesh::ACCEPTED
         * @return The amount of accepted neighbours
         */
        virtual size_t get_neighbour_count() = 0;

        /**
         * \brief Retrieve node_id's of all neighbours that have state mesh::ACCEPTED
         *
         * @param data Pointer to memory location to store node_id's in
         */
        virtual void get_neighbours(uint8_t data[]) = 0;

        /**
         * \brief Handle DISCOVERY::PRESEND messages
         *
         * Called when a message with type DISCOVERY::PRESEND is received.
         * Should open a direct connection to the message's sender, and set the state of this connection to mesh::RESPONDED.
         * No messages need to be sent yet, mesh_network takes care of this.
         * If no more connections can be made, or the connection can't be made for another reason, this method should just return false. Mesh_network will drop the message in this case.
         * @param origin Message containing the DISCOVERY::PRESEND
         * @return True if the connection was made, false otherwise.
         */
        virtual bool discovery_present_received(mesh::message &origin) = 0;

        /**
         * \brief Handle DISCOVERY::RESPOND messages
         *
         * Called when a message with type DISCOVERY::RESPOND is received.
         * Should check if this direct connection is possible (unused channel etc.), and set the connection state to mesh::ACCEPTED.
         * If the connection is not possible, this method can just return false. Mesh_network will take care of sending deny messages.
         * @param origin message containing the DISCOVERY::RESPOND
         * @return True if the connection can be accepted, false otherwise
         */
        virtual bool discovery_respond_received(mesh::message &origin) = 0;

        /**
         * \brief Handle DISCOVERY::ACCEPT messages
         *
         * Called when a message with type DISCOVERY::ACCEPT is received.
         * Should set the connection state to mesh::ACCEPTED.
         * @param origin Message containing the DISCOVERY::ACCEPT
         */
        virtual void discovery_accept_received(mesh::message &origin) = 0;

        /**
         * \brief Disconnect from a given node_id
         *
         * No messages need to be sent, just disable the connection and set it's state to DISCONNECTED.
         * @param address Address of node_id to disconnect
         */
        virtual void remove_direct_connection(const uint8_t &address) = 0;

        /**
         * \brief Print a connection status message, mainly used for debugging
         */
        virtual void status() = 0;
    };

    /**
     * @}
     */

}
#endif //IPASS_MESH_CONNECTIVITY_ADAPTER_HPP
