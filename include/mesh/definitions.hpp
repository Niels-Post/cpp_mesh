/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_MESH_MESSAGE_TYPES_HPP
#define IPASS_MESH_MESSAGE_TYPES_HPP

namespace mesh {

    /**
     * \defgroup definitions Mesh Networking Definitions
     * \ingroup mesh_networking
     * \brief Some constant definitions for mesh networking
     *
     *  - Connection States
     *  - Base message types
     *  Definitions are marked constexpr to prevent bloat by unused definitions
     */

    /**
     * \addtogroup definitions
     * @{
     */

    /**
     * \brief Connection state of a direct mesh connection.
     *
     * Note that this is not used for nodes connected through routing
     */
    enum mesh_connection_state {
        /// Node is not connected at all
                DISCONNECTED,
        /// This is a listen connection, with no node on the other side
                WAITING,
        /// We sent a response to this node, but it hasn't been acknowledged yet
                RESPONDED,
        /// This connection is accepted and ready to go
                ACCEPTED
    };

    /**
     * \brief Message types for basic discovery messages
     */
    struct DISCOVERY {
        /// Do nothing, can be used as a keepalive using send_all
        static constexpr const uint8_t NO_OPERATION = 0x00;
        /// Present to other nodes for connecting, this is sent as broadcast
        static constexpr const uint8_t PRESENT = 0x01;
        /// Respond to a present message, this is sent as unicast
        static constexpr const uint8_t RESPOND = 0x02;
        /// Accept a RESPOND message
        static constexpr const uint8_t ACCEPT = 0x03;
        /// Deny a respond message, receiver of this message should disconnect on receive of this message
        static constexpr const uint8_t DENY = 0x04;

    };

    /**
     * \brief Message types for link_state routing
     */
    struct LINK_STATE_ROUTING {
        /// Request an update from everyone on the network, this should also contain update information from the sender
        static constexpr const uint8_t UPDATE_REQUEST = 0x10;
        /// Send neighbour information
        static constexpr const uint8_t UPDATE = 0x11;
    };

    /**
     * \brief Message types for Mesh_Domotics
     */
    struct DOMOTICA {
        /// Send mesh_domotics data, should be filled with 4 bytes of data
        static constexpr const uint8_t DATA = 0x20;
    };

    /**
     * @}
     */

}

#endif //IPASS_MESH_MESSAGE_TYPES_HPP
