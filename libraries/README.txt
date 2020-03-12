OSD3358-SM-RED Supporting Libraries (Redperipherallib)
Octavo Systems - 2017

Contains libraries to assist in the use of your OSD3358-SM-RED platform.
The included Makefile will compile a single shared object redperipherallib.so
and install it to /usr/lib. It will also move all header files to
/usr/include.

To use the .so you only need to #include <redperipherallib.h>
We also recommend using #include <useful_includes.h>

Some of the library components may require device tree changes. Please make 
sure proper pinmuxing is done before using the libraries.


