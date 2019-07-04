/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_STATUS_LCD_HPP
#define IPASS_STATUS_LCD_HPP


#include <mesh/mesh_network.hpp>
#include <mesh/router.hpp>
#include <lcd/i2c_backpack.hpp>

namespace mesh {
    namespace addons {
        /**
         * \addtogroup addons
         * @{
         */

        /**
         * \brief LCD Status display mode
         *
         * Determines what data should be displayed on the LCD at this moment
         */
        enum display_mode {
            /// Some general information about connection staus
                    GENERAL,
            /// Known distances to all nodes in the network (only for link-state routing)
                    LINK_STATE_DISTANCES,
            /// Direct connections as known by lin_state routing
                    LINK_STATE_NEIGHBOURS,
            /// Current size of the message buffer, note that since update is only called periodically, it might not be up to date
                    NETWORK_BUFFERSIZE,
            /// Information about the network's direct connection blacklist
                    BLACKLIST,
        };

        /**
         * \brief Shows network information on an i2c LCD, using the i2c_backpack module
         */
        class status_lcd {

            mesh::mesh_network &network;
            lcd::i2c_backpack &lcd;
            display_mode current_mode;
        public:
            /**
             * Create an LCD_Status
             * @param network Network to show information for
             * @param lcd LCD display to show information on
             */
            status_lcd(mesh_network &network, lcd::i2c_backpack &lcd);

            /**
             * \brief Update the information currently shown on the LCD
             */
            void update();


            /**
             * \brief Change display mode
             *
             * Note that the LCD is not updated until the next update() call
             * @param currentMode Mode to switch to
             */
            void setCurrentMode(display_mode currentMode);
        };

        /**
         * @}
         */

    }

}


#endif //IPASS_STATUS_LCD_HPP
