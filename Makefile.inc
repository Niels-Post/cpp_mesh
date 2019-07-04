#
# Copyright Niels Post 2019.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt)
#

MESH_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

SEARCH += $(MESH_DIR)include/

SOURCES += $(MESH_DIR)src/connectivity_adapter.cpp
SOURCES += $(MESH_DIR)src/router/link_state_router.cpp



HEADERS += $(MESH_DIR)include/mesh/connectivity_adapter.hpp
HEADERS += $(MESH_DIR)include/mesh/definitions.hpp
HEADERS += $(MESH_DIR)include/mesh/mesh_network.hpp
HEADERS += $(MESH_DIR)include/mesh/message.hpp
HEADERS += $(MESH_DIR)include/mesh/router.hpp
HEADERS += $(MESH_DIR)include/mesh/router/link_state_router.hpp


# The following files depend on HWLib, since they use its pin_out implementation
ifdef ($(NO_HWLIB))
SOURCES += $(MESH_DIR)src/connectivity/nrf.cpp
SOURCES += $(MESH_DIR)src/connectivity/nrf_pipe.cpp
HEADERS += $(MESH_DIR)include/mesh/connectivity/nrf.hpp
HEADERS += $(MESH_DIR)include/mesh/connectivity/nrf_pipe.hpp
SOURCES += $(MESH_DIR)src/addon/status_lcd.cpp
HEADERS += $(MESH_DIR)include/mesh/addon/status_lcd.hpp
endif