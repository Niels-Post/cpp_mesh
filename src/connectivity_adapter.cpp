/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#include <mesh/connectivity_adapter.hpp>
#include <cout_debug.hpp>

void mesh::connectivity_adapter::add_message_id(mesh::message &msg) {
    if (msg.sender == id && msg.message_id == 0) {
        msg.message_id = current_message_id++;
    }
}

mesh::connectivity_adapter::connectivity_adapter(const mesh::node_id &my_id) : id(my_id) {}

void mesh::connectivity_adapter::forget_message_history_for(const mesh::node_id &id) {
    for (size_t i = 0; i < previous_messages_count; i++) {
        if ((previous_messages[i] >> 8) == id) {
            for (uint8_t j = i; j < previous_messages_count; j++) {
                previous_messages[i] = previous_messages[i + 1];
            }
            previous_messages_count--;
        }
    }
}

bool mesh::connectivity_adapter::is_new_message(const mesh::message &msg) {
    uint16_t check_value = ((msg.sender << 8) | msg.message_id);
    for (size_t i = 0; i < previous_messages_count; i++) {
        if (previous_messages[i] == check_value) {
            return false;
        }
    }

    if (previous_messages_count == 20) {
        for (uint8_t i = 0; i < 19; i++) {
            previous_messages[i] = previous_messages[i + 1];
        }
        previous_messages_count--;
    }
    previous_messages[previous_messages_count++] = check_value;
    return true;
}

bool mesh::connectivity_adapter::send(mesh::message &message, mesh::node_id next_hop) {
    if (next_hop == 0) next_hop = message.receiver;


    mesh_connection_state state = connection_state(next_hop);
    if (state == DISCONNECTED) {
        LOG("Node disconnected", "");
        status();
        return false;
    } else if (message.receiver != 0 &&
               state != mesh::ACCEPTED
               && message.type != DISCOVERY::RESPOND
               && message.type != DISCOVERY::ACCEPT
               && message.type != DISCOVERY::DENY) {
        return false;
    }

    add_message_id(message);
    add_connection_data(message, next_hop);


    uint8_t message_bytes[message.size()];
    message.to_byte_array(message_bytes);

    for (uint8_t fail_count = 0; fail_count < 5; fail_count++) {

        if (send_implementation(next_hop, message_bytes, message.size())) {

            return true;
        }

        hwlib::wait_ms(1);
        has_message();

    }

    return false;
}

bool mesh::connectivity_adapter::send_all(message &msg, mesh::node_id *failed_addresses) {
    bool all_successful = true;
    node_id neighbours[get_neighbour_count()];
    get_neighbours(neighbours);

    add_message_id(msg);

    for (size_t i = 0; i < get_neighbour_count(); i++) {
        if (connection_state(neighbours[i]) == mesh::ACCEPTED && msg.sender != neighbours[i]) {
            message copy_msg = msg; //Make a copy with neighbour-specific changes
            if (!send(copy_msg, neighbours[i])) {
                if (failed_addresses != nullptr) {
                    *failed_addresses++ = neighbours[i];
                }
                all_successful = false;
            }

        }

    }
    return all_successful;
};
