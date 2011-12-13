System Settings Server/Client
=============================

Both Server and Client applications are using cmake as default build tool.
This file will guide you to compile these applications.


1. SERVER
=========

The Server application is the application which make available the menu
structure over dbus.

1.1 Compiling the server
------------------------

cd server
mkdir build
cd build
cmake ../


2. CLIENT
=========

The client side application read the information exported by the server and
create a QML menu structure based on this information

2.1 Compiling the client
------------------------

cd system-ui
mkdir build
cd build
cmake ../


3. TESTING
==========

To test the current implementation you should run the server and client
application simultaneous. To do that follow the steps below:

In the server build directory(<project-dir>/server/build):
    ./src/ExportMenu ../xml/test.xml &
    ./system-ui/examples/run-example.sh ../system-ui/examples/DBusMenuTest.qml


