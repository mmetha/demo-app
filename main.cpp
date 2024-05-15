#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/asio/property.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <variant>

std::string testFunction(const int32_t& callCount) 
{
    return "success: " + std::to_string(callCount);
}

int server() {
    // setup connection to dbus
    boost::asio::io_context io;
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);

    // make object server
    conn->request_name("dbus.test.server");
    auto server = sdbusplus::asio::object_server(conn);

    // add interface 
    std::shared_ptr<sdbusplus::asio::dbus_interface> iface =
        server.add_interface("/test/object", "test.interface");

    // add properties
    iface->register_property("intVal", 33,
        sdbusplus::asio::PropertyPermission::readWrite);

    // add properties with specialized callbacks
    iface->register_property("valWithRange", 66,
        // custom set
        [](const int& req, int& propertyValue) {
            if (req >= 50 && req <= 100)
            {
                propertyValue = req;
                return true;
            }

            return false;
        });

    // add properties (vector)
    std::vector<std::string> myStringVec = { "some", "test", "data" };
    iface->register_property("myStringVec", myStringVec,
        sdbusplus::asio::PropertyPermission::readWrite);

    // add method
    iface->register_method("testMethod", testFunction);

    // initialize interface
    iface->initialize();

    io.run();

    return 0;
}

int client() {
    // setup connection to dbus
    boost::asio::io_context io;
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);

    // check if IntVal exist
    sdbusplus::asio::getProperty<int>(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "intVal", [](boost::system::error_code ec, int value) 
        {
            if (ec) {
                std::cout << "Failed to get intVal:" << ec << std::endl;
                return;
            }
            std::cout << "Original intVal:" << value << std::endl;
            return;
        }
    );

    // set new IntVal and retrive the value
    sdbusplus::asio::setProperty(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "intVal", 41, [](boost::system::error_code ec) {
            if (ec)
            {
                std::cout << "Failed to set intVal:" << ec << std::endl;
                return;
            }
        }
    );
    sdbusplus::asio::getProperty<int>(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "intVal", [](boost::system::error_code ec, int value) 
        {
            if (ec) {
                std::cout << "Failed to get intVal:" << ec << std::endl;
                return;
            }
            std::cout << "New intVal:" << value << std::endl;
        }
    );

    // check if valWithRange exist
    sdbusplus::asio::getProperty<int>(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "valWithRange", [](boost::system::error_code ec, int value) 
        {
            if (ec) {
                std::cout << "Failed to get valWithRange:" << ec << std::endl;
                return;
            }
            std::cout << "Original valWithRange:" << value << std::endl;
            return;
        }
    );

    // set new valWithRange and retrive the value
    sdbusplus::asio::setProperty(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "valWithRange", 120, [](boost::system::error_code ec) {
            if (ec)
            {
                std::cout << "Failed to set valWithRange:" << ec << std::endl;
                return;
            }
        }
    );
    sdbusplus::asio::getProperty<int>(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "valWithRange", [](boost::system::error_code ec, int value) 
        {
            if (ec) {
                std::cout << "Failed to get valWithRange:" << ec << std::endl;
                return;
            }
            std::cout << "New valWithRange:" << value << std::endl;
        }
    );

    // check if myStringVec exist
    sdbusplus::asio::getProperty<std::vector<std::string>>(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "myStringVec", [](boost::system::error_code ec, auto value) 
        {
            if (ec) {
                std::cout << "Failed to get myStringVec:" << ec << std::endl;
                return;
            }
            std::cout << "Original myStringVec:";
            for (auto& element : value) {
                std::cout << element << ' ';
            }
            std::cout << std::endl;
            return;
        }
    );

    std::vector<std::string> myStringVec = { "new", "data", "for", "test" };

    // set new myStringVec and retrive the value
    sdbusplus::asio::setProperty(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "myStringVec", myStringVec, [](boost::system::error_code ec) {
            if (ec)
            {
                std::cout << "Failed to set myStringVec:" << ec << std::endl;
                return;
            }
        }
    );
    sdbusplus::asio::getProperty<std::vector<std::string>>(
        *conn, "dbus.test.server", "/test/object", "test.interface",
        "myStringVec", [](boost::system::error_code ec, auto value) 
        {
            if (ec) {
                std::cout << "Failed to get myStringVec:" << ec << std::endl;
                return;
            }
            std::cout << "New myStringVec:";
            for (auto& element : value) {
                std::cout << element << ' ';
            }
            std::cout << std::endl;
            return;
        }
    );

    //method call
    conn->async_method_call(
        [](boost::system::error_code ec, std::string testValue) {
            if (ec)
            {
                std::cout << "fnErr: " << ec << std::endl;
                return;
            }
            std::cout << "fnReturn: " << testValue << std::endl;
        },
        "dbus.test.server", "/test/object", 
        "test.interface", "testMethod", int32_t(41)
    );

    io.run();

    return 0;
}

int main(int argc, const char* argv[])
{
    if (argc == 2) {
        if (std::string(argv[1]) == "--client") {
            std::cout << "Running Client" << std::endl;
            return client();
        } else if (std::string(argv[1]) == "--server") {
            std::cout << "Running Server" << std::endl;
            return server();
        }
    }

    std::cout << "usage: " << argv[0] << " [--server | --client]" << std::endl;
    return -1;
}
