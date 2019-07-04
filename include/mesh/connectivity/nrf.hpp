/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_MESH_NRF_CONNECTIVITY_HPP
#define IPASS_MESH_NRF_CONNECTIVITY_HPP

#include <nrf24l01plus/nrf24l01plus.hpp>
#include <mesh/connectivity_adapter.hpp>
#include <mesh/connectivity/nrf_pipe.hpp>

namespace mesh {
    namespace connectivity {
        /**
         * \addtogroup connectivity_adapters
         * @{
         */

        /**
         * \brief NRF Connectivity adapter
         *
         * New NRF addresses are created by checking for unused pipes starting at the node_id incrementing by 2.
         * For this reason, nodes in an NRF mesh network should always be at least 12 node_id's apart
         */
        class nrf : public mesh::connectivity_adapter {
        private:
            std::array<message, 100> message_buffer = {};
            size_t buffer_start = 0;
            size_t buffer_end = 0;


            std::array<nrf_pipe, 6> connections;
            nrf24l01::nrf24l01plus &nrf24;
            const nrf24l01::address discovery_address = {0x70, 0x70, 0x70, 0x70, 0x70};
            const nrf24l01::address base_address = {0x72, 0x72, 0x72, 0x72, 0x70};

            /**
             * \brief get index of pipe connected through a given NRF address last-byte.
             *
             * This method retrieves index, regardless of connection state.
             * @param nrfaddress address to find
             * @return The index of the pipe, or 6 if none was found
             */
            uint8_t getPipeByNRFAddress(const uint8_t &nrfaddress);

            /**
             * \brief Get index of a pipe connected to a given node_id
             *
             * This method retrieves index, regardless of connection state.
             * @param node_id Node_id to find
             * @return The index of the pipe, or 6 if none was found
             */
            uint8_t getPipeByNodeId(const uint8_t &node_id);

            /**
             * \brief Retrieve index of the first pipe with connection state mesh::DISCONNECTED
             * @return The index of the free pipe, or 6 if none was found
             */
            uint8_t getFirstFreePipe();

            /**
             * \brief Buffer received messages to prevent FIFO overflow in the NRF module
             */
            void buffer_messages();


        public:
            /**
             * \brief Create an NRF connectivity adapter
             *
             * @param address Address of this node
             * @param nrf NRF module to use
             */
            nrf(const node_id &address, nrf24l01::nrf24l01plus &nrf);


        private:
            /**
             * \brief Open a listening connection to wait for DISCOVERY::RESPOND messages
             *
             * If a listening connection is already open, nothing happens.
             * If none is open, the first free pipe will be converted to a listening pipe.
             */
            void start_waiting();

        protected:
            /**
             * \brief send implementation for NRF
             *
             * @param id Node_id to send to
             * @param data Pointer to data to be sent
             * @param size Size of the data to be sent
             * @return True if transmission was successful (and autoACK'ed)
             */
            bool send_implementation(node_id &id, uint8_t *data, size_t size) override;

        public:

            /**
             * \brief Checks if a message is available
             *
             * This method fills the buffer with new messages from the NRF module, then checks if the buffer has messages available.
             * It does this to prevent NRF module FIFO buffer overflow
             * @return True if a message is available in buffer
             */
            bool has_message() override;

            /**
             * \brief Retrieves the first message from buffer
             *
             * This method also fills the buffer with new messages first
             * @return The first message available (in FIFO order)
             */
            mesh::message next_message() override;

            /**
             * \brief Get connection state for node_id
             *
             * Returns DISCONNECTED when the node_id is not found
             * @param id Id to check for
             * @return The connection state
             */
            mesh::mesh_connection_state connection_state(const uint8_t &id) override;

            /**
             * \brief Handle a discovery_present message
             *
             * Opens a connection to the NRF address found in the message's connectionData, then returns true.
             * When the maximum amount of connections is reached, returns false.
             * @param origin Message to handle
             * @return True if connection was made, false otherwise
             */
            bool discovery_present_received(mesh::message &origin) override;

            /**
             * \brief Breaks connection to a given node_id
             *
             * Sets the found pipe's state to DISCONNECTED, and it's node_id to 0, to make sure it isn't accidentally found later
             * @param id ID for which to remove the connection
             */
            void remove_direct_connection(const uint8_t &id) override;

            /**
             * \brief Handle a discovery_respond message
             *
             * Checks if the connection on the given NRF address (message connectionData) is not already an established connection to another node.
             * If it isn't, it sets the node_id for this pipe, otherwise return false.
             * @param origin message to handle
             * @return True if the pipe was still free, and can be used
             */
            bool discovery_respond_received(mesh::message &origin) override;

            /**
             * \brief Handle a discover_accept message
             *
             * Sets the connection state of the pipe connected to the sender to ACCEPTED
             * @param origin Message to handle
             */
            void discovery_accept_received(mesh::message &origin) override;

            /**
             * \brief Get count of currently connected neighbours
             *
             * Only returns the count of ACCEPTED connections
             * @return The count
             */
            size_t get_neighbour_count() override;

            /**
             * \brief Load list of directly connected node_id's
             *
             * Only loads node_id's of ACCEPTED connections
             * @param data Pointer to the location to store the id's in
             */
            void get_neighbours(uint8_t *data) override;

            /**
             * \brief Add NRF-specific data to messages that are to be sent
             *
             * For NRF this only needs to add NRF addresses to PRESENT and RESPOND messages
             * @param message Message to add data to
             * @param next_hop Next hop of the message, not really necessary for NRF
             */
            void add_connection_data(message &message, node_id &next_hop) override;

            /**
             * \brief Print NRF connection status message
             *
             * Prints TX and RX-base addresses.
             * Prints all connections, using their operator<<
             */
            void status() override;
        };

        /**
         * @}
         */

    }
}
#endif //IPASS_MESH_NRF_CONNECTIVITY_HPP
