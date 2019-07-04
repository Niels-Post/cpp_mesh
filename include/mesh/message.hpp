/*
 *
 * Copyright Niels Post 2019.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef IPASS_MESH_MESSAGE_HPP
#define IPASS_MESH_MESSAGE_HPP

#include <stdlib.h>
#include <stdint.h>
#include <array>
#include <mesh/definitions.hpp>

namespace mesh {

    /// \brief Alias uint8_t to make function definitions easier to understand
    typedef uint8_t message_type;
    /// \brief Alias uint8_t to make function definitions easier to understand
    typedef uint8_t node_id;

    /**
     * \brief A message to be used in a mesh network
     */
    struct message {
        /**
         * Get the bytesize of this message eg. The amount of bytes needed to transmit it fully
         * @return The bytesize
         */
        size_t size() {
            return dataSize + 7;
        }

        /// Type of the message, see definitions.h
        message_type type;
        /// ID of the message, this is automatically incremented each message by connectivity_adapter, and used for checking if a message was received before
        uint8_t message_id;
        /// Node_id of the sender of the message
        node_id sender;
        /// Receiver of the message, messages with receiver 0 are assumed to be a broadcast
        node_id receiver;
        /// Size of the payload of this message
        uint8_t dataSize;
        /// Payload of this message
        std::array<uint8_t, 25> data = {0};
        /// Additional data, used for connectivity method specific interaction
        std::array<uint8_t, 2> connectionData = {0};

        /**
         * Make an empty message
         */
        message() : type(0), message_id(0), sender(0), receiver(0), dataSize(0) {}

        /**
         * Create a message
         * @param messageType Type of the message
         * @param messageId ID of the message
         * @param sender Sender of the message
         * @param receiver Receiver of the message
         * @param dataSize Size of the message payload
         * @param data Message payload
         * @param connectionData Connection specific data
         */
        message(message_type messageType, uint8_t messageId, node_id sender, node_id receiver, size_t dataSize = 0,
                const std::array<uint8_t, 25> &data = {0}, const std::array<uint8_t, 2> &connectionData = {0})
                : type(
                messageType), message_id(messageId), sender(sender), receiver(receiver), dataSize(dataSize),
                  data(data),
                  connectionData(
                          connectionData) {}

        /**
         * \brief Write the message data to a byte array
         *
         * Make sure the array has at least message.size() bytes of space.
         * This method always writes the connection specific data, if this is not needed, the last 2 bytes can be cut off later.
         * @param out Memory location to write to
         */
        void to_byte_array(uint8_t out[]) {
            out[0] = type;
            out[1] = message_id;
            out[2] = sender;
            out[3] = receiver;
            out[4] = dataSize;

            for (size_t i = 0; i < dataSize; i++) {
                out[i + 5] = data[i];
            }
            out[size() - 2] = connectionData[0];
            out[size() - 1] = connectionData[1];
        }

        /**
         * \brief Parse a byte array, and save the result in the current message.
         *
         * When the data is not large enough to contain a message, this method does nothing but return false.
         * @param n Size of the byte array.
         * @param in Memory location of the byte array
         * @return True if the byte array was a valid message, and it was processed, false otherwise.
         */
        bool parse(size_t n, const uint8_t in[]) {
            data.fill(0);
            connectionData.fill(0);


            if (n < 7) {
                return false;
            }

            type = in[0];
            message_id = in[1];
            sender = in[2];
            receiver = in[3];
            dataSize = in[4];


            for (size_t i = 0; i < (n - 7); i++) {
                data[i] = in[i + 5];
            }
            connectionData[0] = in[n - 2];
            connectionData[1] = in[n - 1];
            return true;
        }

#ifdef HWLIB_H

        /**
         * \brief Print this message to an ostream
         *
         * Can be used for debugging, but is hwlib dependent, and is therefore only compiled when hwlib is present.
         * @param os
         * @param message
         * @return
         */
        friend hwlib::ostream &operator<<(hwlib::ostream &os, const message &message) {
            os << "type:" << message.type << " message_id:" << message.message_id << " sender:" << hwlib::hex
               << message.sender
               << " receiver: " << message.receiver << " dataSize:" << hwlib::dec << message.dataSize
               << " connectionData:"
               << message.connectionData[0] << " - ";

            for (size_t i = 0; i < message.dataSize; i++) {
                os << message.data[i] << " ";
            }
            return os;
        }

#endif
    };
}

#endif //IPASS_MESH_MESSAGE_HPP
