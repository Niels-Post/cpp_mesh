C++ Mesh networking
============================

Library for mesh network communication in C++. Code standard: C++17.

This library is part of my IPASS end project, for the first year of my HBO-ICT study at Hogeschool Utrecht in 2019.


Features 
---
- Different routing algorithms can easily be swapped in
- Different connection methods can easily be used

Included
---
- Link state routing
- NRF24L01+ connectivity

Dependencies
-----
When using link_state routing, 
[cpp_link_state_algorithm](https://github.com/Niels-Post/cpp_link_state_algorithm) is required to perform the algorithm.

When using the library with a microcontroller and the nrf connectivity adapter, [cpp_nrf24l01](https://github.com/Niels-Post/cpp_nrf24l01) is needed for communication with the NRF module.

Since this library was originally used for an embedded project, the included makefile is written for [BMPTK](http://github.com/wovo/bmptk).
However, the library can also be compiled without BMPTK, for which instructions are below.


Installation/usage
-----
- Download the library `git clone https://github.com/Niels-Post/mesh`
- Include *Makefile.inc* from your project
- When not using HWLIB, make sure to set NO_HWLIB before including *Makefile.inc*
- Include `mesh/mesh_network.hpp`

Building with BMPTK
----
- Just Include *Makefile.inc* :)


Building without BMPTK
----
The included makefile only sets some variables, but doesn't do any compiling itself. When building without BMPTK, some variables should be interpreted properly after including.
- HEADERS: all .hpp header files
- SOURCES: all library .cpp files
- SEARCH: the include path for header files of this library 


License Information
---
   
*Copyright Niels Post 2019.     
Distributed under the Boost Software License, Version 1.0.  
(See accompanying file LICENSE_1_0.txt or copy at  
[https://www.boost.org/LICENSE_1_0.txt](https://www.boost.org/LICENSE_1_0.txt))*
  
 