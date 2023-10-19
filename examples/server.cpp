#include "timer.h"
#include <iostream>
#include <zmq.hpp>
#include <map>
#include <string>
#include <thread>
#include <chrono>

double server_processing(double client_result, int n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(n));
    return n;
}

int main(int argc, char** argv) {
    constexpr int numIters = 100;
    Timer timer("../logs/server-times.json", TimerOperation::Chrome);

    // Initialize a ZMQ context
    std::unordered_map<std::string, std::string> emptyArgs;
    timer.start("ZMQ Setup", "Setup", emptyArgs);
    zmq::context_t context(1);

    // Create a ZMQ socket for request-reply pattern
    zmq::socket_t socket(context, zmq::socket_type::rep);

    // Default bind address is for localhost
    std::string bind_address = "tcp://*:12345";

    // Bind to the specified address
    socket.bind(bind_address);

    for (int i = 0; i < numIters; ++i) {
        std::cout << "Awaiting client result..." << std::endl;
        std::unordered_map<std::string, std::string> args = {{"iteration", std::to_string(i)}};

        // Receive client's "RESULT"
        zmq::message_t request;
        auto _ = socket.recv(request, zmq::recv_flags::none);
        timer.stop("Client Send", std::to_string(i), args);
        std::string requestStr(static_cast<char*>(request.data()), request.size());

        if (requestStr.find("RESULT:") == 0) {
            // Extract the result value from the received string
            double client_result = std::stod(requestStr.substr(8));

            timer.start("Server Processing", std::to_string(i), args);
            // Process the client's result and get the server's result
            double server_result = server_processing(client_result, i);
            timer.stop("Server Processing", std::to_string(i), args);

            // Send the server's result back to the client
            std::string resultStr = "SERVER RESULT: " + std::to_string(server_result);
            zmq::message_t reply(resultStr.size());
            memcpy(reply.data(), resultStr.c_str(), resultStr.size());
            timer.start("Server Send", std::to_string(i), args);
            socket.send(reply, zmq::send_flags::none);

        }
    }
    timer.dumpLogs();

    return 0;
}
