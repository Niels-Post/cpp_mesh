/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#include <mesh/connectivity/nrf_pipe.hpp>

using nrf24l01::nrf24l01plus;
using std::array;
using nrf24l01::NRF_REGISTER;

namespace mesh {
    namespace connectivity {
        void nrf_pipe::flush(nrf24l01plus &nrf) {
            uint8_t old_mode = nrf.get_mode();
            nrf.mode(nrf.MODE_NONE);
            switch (connection_state) {
                case mesh::DISCONNECTED:
                    nrf.rx_enabled(pipe_number, false);
                    break;
                case mesh::WAITING:
                case mesh::RESPONDED:
                case mesh::ACCEPTED:
                    nrf.rx_set_address(pipe_number, nrf_address);
                    nrf.rx_enabled(pipe_number, true);
                    if (pipe_number == 0) {
                        nrf.tx_set_address(nrf_address);
                    }
                    break;
            }
            nrf.mode(old_mode);
        }

        bool nrf_pipe::send_message(array<nrf_pipe, 6> &all_pipes, nrf24l01plus &nrf,
                                    uint8_t n,
                                    uint8_t *data) {
            uint8_t old_mode = nrf.get_mode();
            nrf.mode(nrf.MODE_PTX);

            uint8_t old_pipe = pipe_number;
            if (pipe_number != 0) {
                nrf.rx_enabled(pipe_number, false);
                pipe_number = 0;
                flush(nrf);
            }
            nrf.tx_flush();
            nrf.write_register(NRF_REGISTER::NRF_STATUS, 0x60); //clear previous sent bit
            nrf.write_register(NRF_REGISTER::NRF_STATUS, 0x10); // Clear Max RT
            nrf.tx_write_payload(data, n, old_pipe == 0);
            bool success = true;
            do {
                nrf.no_operation();
                if ((nrf.last_status & nrf24l01::NRF_STATUS::MAX_RT) > 0) {
                    success = false;
                    nrf.write_register(NRF_REGISTER::NRF_STATUS, 0x10); // Clear Max RT
                    break;
                }

            } while ((nrf.last_status & nrf24l01::NRF_STATUS::TX_DS) == 0);


            if (old_pipe != 0) {
                pipe_number = old_pipe;
                flush(nrf);
                all_pipes[0].flush(nrf);
            }
            nrf.mode(old_mode);
            return success;
        }

        void nrf_pipe::setConnectionState(mesh::mesh_connection_state cS) {
            nrf_pipe::connection_state = cS;
        }

        void nrf_pipe::setNodeId(node_id nodeId) {
            connected_node = nodeId;
        }

        void nrf_pipe::setNrfAddress(const nrf24l01::address &nrfAddress) {
            nrf_address = nrfAddress;
        }

        mesh::mesh_connection_state nrf_pipe::getConnectionState() const {
            return connection_state;
        }

        uint8_t nrf_pipe::getNodeId() const {
            return connected_node;
        }

        const nrf24l01::address &nrf_pipe::getNrfAddress() const {
            return nrf_address;
        }


        hwlib::ostream &operator<<(hwlib::ostream &os, const nrf_pipe &pipe) {
            os << "connection_state: " << pipe.connection_state << " pipe_number: " << pipe.pipe_number
               << " connected_node: " << hwlib::hex << pipe.connected_node << " address: "
               << pipe.nrf_address.address_bytes[4];
            return os;
        }
    }
}