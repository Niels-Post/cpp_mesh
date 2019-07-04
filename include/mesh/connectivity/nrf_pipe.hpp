/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_MESH_NRF_PIPE_HPP
#define IPASS_MESH_NRF_PIPE_HPP


#include <hwlib.hpp>
#include <nrf24l01plus/nrf24l01plus.hpp>
#include <nrf24l01plus/definitions.hpp>
#include <mesh/definitions.hpp>
#include <mesh/message.hpp>

namespace mesh {
    namespace connectivity {
        /**
         * \addtogroup connectivity_adapters
         * @{
         */

        /**
         * \brief ADT for a single NRF connection
         *
         * Handles the pipe-swapping needed for sending messages over a non-base address.
         * Contains the connection state of a pipe.
         * A pipe with number 0 is assumed to be a broadcast pipe.
         * Buffers settings before sending them to an NRF module, to prevent keeping it busy for too long.
         */
        class nrf_pipe {
            mesh::mesh_connection_state connection_state = mesh::DISCONNECTED;
            uint8_t pipe_number;
            node_id connected_node = 0;
            nrf24l01::address nrf_address;

        public:
            /**
             * Create an nrf_pipe.
             * @param pipeNumber Index of the pipe, this can be in range 0-5
             */
            nrf_pipe(uint8_t pipeNumber) : pipe_number(pipeNumber) {}

            /**
             * Flush the current settings to an nrf module
             * @param nrf Nrf module to flush to.
             */
            void flush(nrf24l01::nrf24l01plus &nrf);

            /**
             * \brief Transmit data using the NRF module
             *
             * Since this function takes care of swapping pipe addresses, it needs to have information about all pipes.
             * When the pipe number is 0 (broadcast pipe) the payload is written with NOACK, since having Auto acknowledgement on broadcast messages would not work with more than 2 nodes.
             * When broadcasting, this method will return true as soon as the data is sent.
             * When unicasting, this method only returns true when the data was acknowledged by the other module.
             * @param all_pipes Array of all pipes currently used
             * @param nrf NRF Module to send data through
             * @param n Size of data to send
             * @param data Data to send
             * @return True if the data was sent (and possibly acknowledged) successfully, false otherwise
             */
            bool send_message(std::array<nrf_pipe, 6> &all_pipes, nrf24l01::nrf24l01plus &nrf, uint8_t n,
                              uint8_t *data);

            /**
             * \brief Change the connection state on this pipe
             * @param cS The new connection state
             */
            void setConnectionState(mesh::mesh_connection_state cS);

            /**
             * \brief Set the id of the node this pipe is connected to
             *
             * Note that this doesn't transmit anything, and doesn't actually CHANGE the other node's id
             * @param nodeId The new node id
             */
            void setNodeId(uint8_t nodeId);

            /**
             * \brief Set the NRF address this pipe should transmit/receive on
             *
             * Note that when the pipe_number is between 2 and 5, only the lsByte of the address can be used.
             * The first 4 bytes will be equal to those of pipe 1 in this case
             * @param nrfAddress The new address
             */
            void setNrfAddress(const nrf24l01::address &nrfAddress);

            /**
             * \brief Get current connection state of this pipe
             * @return The state
             */
            mesh::mesh_connection_state getConnectionState() const;

            /**
             * \brief Get the id of the node this pipe is connected to
             * @return The node_id
             */
            uint8_t getNodeId() const;

            /**
             * \brief Get the NRF address this pipe is curerntly using
             *
             * @return  The full address
             */
            const nrf24l01::address &getNrfAddress() const;

            /**
             * \brief Print information about this pipe to an ostream
             *
             * Prints connection state, node id, pipe number and the lsBYte of the nrf address
             * @param os Stream to output to
             * @param pipe Pipe to output information about
             * @return The ostream, after writing
             */
            friend hwlib::ostream &operator<<(hwlib::ostream &os, const nrf_pipe &pipe);
        };
        /**
         * @}
         */
    }
}

#endif //IPASS_MESH_NRF_PIPE_HPP
