/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#include <mesh/addon/status_lcd.hpp>
#include <mesh/router/link_state_router.hpp>

mesh::addons::status_lcd::status_lcd(mesh::mesh_network &network, lcd::i2c_backpack &lcd) : network(network), lcd(lcd),
                                                                                            current_mode(GENERAL) {}

void mesh::addons::status_lcd::update() {
    lcd.set_row(0);
    lcd << "ID: " << hwlib::hex << network.get_connection().id;
    switch (current_mode) {
        case GENERAL:

            break;
        case LINK_STATE_DISTANCES: {
            lcd << "  distances";
            lcd.set_row(1);
            auto &router = static_cast<mesh::routers::link_state &>(network.get_router());
            router.update_neighbours();
            router.get_calculator().cleanup(true);
            for (uint8_t i = 1; i < router.get_calculator().get_node_count(); i++) {
                auto node = router.get_calculator().get_node(i);
                lcd << hwlib::hex << node.id << "(" << node.distance << ")";
            }
            break;
        }
        case LINK_STATE_NEIGHBOURS:
            break;
        case NETWORK_BUFFERSIZE:
            break;
        case BLACKLIST:
            break;
    }
    lcd.flush();
}

void mesh::addons::status_lcd::setCurrentMode(mesh::addons::display_mode currentMode) {
    current_mode = currentMode;
}
