/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#include <mesh/connectivity/nrf.hpp>
#include <cout_debug.hpp>

using nrf24l01::NRF_REGISTER;
using nrf24l01::NRF_FEATURE;
using nrf24l01::NRF_STATUS;
using nrf24l01::nrf24l01plus;
namespace mesh {

    namespace connectivity {

        nrf::nrf(const node_id &address, nrf24l01plus &nrf)
                : connectivity_adapter(
                address),
                  connections{
                          nrf_pipe(0),
                          nrf_pipe(1),
                          nrf_pipe(2),
                          nrf_pipe(3),
                          nrf_pipe(4),
                          nrf_pipe(5)
                  },
                  nrf24(nrf) {


            nrf.write_register(NRF_REGISTER::FEATURE, NRF_FEATURE::EN_DPL | NRF_FEATURE::EN_DYN_ACK);

            // Global nrf options
            nrf.rx_auto_acknowledgement(true);
            nrf.rx_set_dynamic_payload_length(true);

            nrf.write_register(NRF_REGISTER::SETUP_RETR, 0xFA);

            nrf.write_register(NRF_REGISTER::RF_SETUP, 8);

            // Broadcast pipe
            connections[0].setNodeId(0);
            connections[0].setNrfAddress(discovery_address);
            connections[0].setConnectionState(mesh::ACCEPTED);
            connections[0].flush(nrf);

            start_waiting();

            nrf.mode(nrf.MODE_PRX);
        }


        void nrf::add_connection_data(message &message, node_id &next_hop) {
            switch (message.type) {
                case DISCOVERY::RESPOND:
                    message.connectionData[0] = connections[getPipeByNodeId(next_hop)].getNrfAddress().address_bytes[4];
                    break;
                case DISCOVERY::PRESENT:
                    message.connectionData[0] = connections[getPipeByNodeId(id)].getNrfAddress().address_bytes[4];
                    break;
                default:
                    break;
            }
        }

        bool nrf::send_implementation(node_id &id, uint8_t *data, size_t size) {
            size_t listen_pipe = getPipeByNodeId(id);
            if (listen_pipe == 6) {
                LOG("No Pipe", "");
                return false;
            }
            return connections[listen_pipe].send_message(connections, nrf24, size, data);
        }

        mesh::message nrf::next_message() {
            buffer_messages();
            if (buffer_end != buffer_start) {
                message &msg = message_buffer[buffer_start++];
                if (buffer_start == 100) {
                    buffer_start = 0;
                }
                return msg;
            }
            return {};
        }

        void nrf::buffer_messages() {
            while ((nrf24.fifo_status() & uint8_t(1)) == 0) {
                uint8_t payload_width = nrf24.rx_payload_width();
                uint8_t data[payload_width];
                nrf24.rx_read_payload(data, payload_width);


                nrf24.write_register(NRF_REGISTER::NRF_STATUS, NRF_STATUS::RX_DR);
                message_buffer[buffer_end++].parse(payload_width, data);

                if (buffer_end == 100) {
                    buffer_end = 0;
                }
                if (buffer_end == (buffer_start - 1)) {
                    // The input buffer is full panic
                }
            }

        }


        bool nrf::has_message() {
            buffer_messages();
            return buffer_start != buffer_end;
        }


        bool nrf::discovery_present_received(mesh::message &origin) {
            uint8_t freePipe = getFirstFreePipe();
            if (freePipe == 6) {
                return false;
            }

            nrf_pipe &freeConnection = connections[freePipe];
            freeConnection.setNodeId(origin.sender);
            freeConnection.setNrfAddress({base_address, origin.connectionData[0]});
            freeConnection.setConnectionState(mesh::RESPONDED);
            freeConnection.flush(nrf24);

            return true;
        }

        void nrf::remove_direct_connection(const uint8_t &id) {
            uint8_t pipe = getPipeByNodeId(id);
            if (pipe == 6 || pipe == 0) {
                return;
            }

            nrf_pipe &conn = connections[pipe];
            LOG("REMOVING", pipe << " - " << id);
            conn.setConnectionState(mesh::DISCONNECTED);
            conn.setNodeId(0);
            forget_message_history_for(id);

            conn.flush(nrf24);

            start_waiting();
        }


        void nrf::start_waiting() {
            // Check if there is a listen pipe already active
            for (size_t i = 1; i < 6; i++) {
                if (connections[i].getConnectionState() == mesh::WAITING) {
                    return;
                }
            }

            for (nrf_pipe &empty_connection : connections) {
                if (empty_connection.getConnectionState() == mesh::DISCONNECTED) {
                    nrf24l01::address listenaddress = {base_address, id};
                    bool found = true;
                    while (found) {
                        listenaddress.address_bytes[4] += 2;
                        found = false;
                        for (nrf_pipe &check_connection : connections) {
                            if (check_connection.getNrfAddress() == listenaddress) {
                                found = true;
                                break;
                            }
                        }
                    }

                    empty_connection.setNrfAddress(listenaddress);
                    empty_connection.setNodeId(id);
                    empty_connection.setConnectionState(mesh::WAITING);
                    empty_connection.flush(nrf24);
                    return;
                }
            }


        }

        mesh::mesh_connection_state nrf::connection_state(const node_id &id) {
            size_t pipe = getPipeByNodeId(id);
            return pipe == 6 ? mesh::DISCONNECTED : connections[pipe].getConnectionState();
        }

        uint8_t nrf::getPipeByNRFAddress(const uint8_t &nrfaddress) {
            for (int i = 0; i < 6; i++) {
                if (connections[i].getNrfAddress().address_bytes[4] == nrfaddress) {
                    return i;
                }
            }
            return 6;
        }

        uint8_t nrf::getPipeByNodeId(const uint8_t &node_id) {
            for (int i = 0; i < 6; i++) {
                if (connections[i].getNodeId() == node_id) {
                    return i;
                }
            }
            return 6;
        }

        bool nrf::discovery_respond_received(mesh::message &origin) {
            uint8_t pipe_nr = getPipeByNRFAddress(origin.connectionData[0]);
            if (pipe_nr == 6 || connections[pipe_nr].getNodeId() != id) {
                return false;
            }

            if (getPipeByNodeId(origin.sender) != 6) {
                remove_direct_connection(origin.sender);
            }


            nrf_pipe &connection = connections[pipe_nr];

            connection.setConnectionState(mesh::ACCEPTED);
            connection.setNodeId(origin.sender);
            connection.flush(nrf24);


            start_waiting();

            return true;
        }

        void nrf::discovery_accept_received(mesh::message &origin) {

            connections[getPipeByNodeId(origin.sender)].setConnectionState(mesh::ACCEPTED);
        }

        uint8_t nrf::getFirstFreePipe() {
            for (int i = 0; i < 6; i++) {
                if (connections[i].getConnectionState() == mesh::DISCONNECTED) {
                    return i;
                }
            }
            return 6;
        }

        size_t nrf::get_neighbour_count() {
            size_t count = 0;
            for (size_t i = 1; i < connections.size(); i++) {
                if (connections[i].getConnectionState() == ACCEPTED) {
                    count++;
                }
            }
            return count;
        }

        void nrf::get_neighbours(uint8_t *data) {
            for (size_t i = 1; i < connections.size(); i++) {

                if (connections[i].getConnectionState() == ACCEPTED) {
                    *data++ = connections[i].getNodeId();
                }
            }
        }

        void nrf::status() {
            LOG("Connection status:", "");
            for (size_t i = 0; i < 6; i++) {
                LOG(i, connections[i]);
            }

            nrf24l01::address test = nrf24.rx_get_address(0);
            LOG("RX0", test);
            test = nrf24.tx_get_address();
            LOG("TX", test);
            LOG("status", hwlib::bin << nrf24.last_status);
            LOG("fifo", hwlib::bin << nrf24.fifo_status());
            uint8_t dat;
            nrf24.read_register(NRF_REGISTER::EN_RXADDR, &dat);
            LOG("EN_RX", hwlib::bin << dat);


        }
    }
}

