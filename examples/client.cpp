#include "timer.h"
#include <iostream>
#include <thread>
#include <zmq.hpp>
#include <map>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>

double processing(unsigned int size)
{
    std::this_thread::sleep_for(std::chrono::microseconds(size*size*size));
    return size*size*size;
}

int main(int argc, char **argv)
{
    Timer timer("../logs/client-times.json", TimerOperation::Chrome);
    constexpr int numIters = 100;

    // Initialize random generators and distributions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> size_gen(10, 100);
    std::unordered_map<std::string, std::string> emptyArgs;

    // Initialize a ZMQ context
    zmq::context_t context(1);

    // Create a ZMQ socket for request-reply
    zmq::socket_t socket(context, zmq::socket_type::req);

    // Check for command-line arguments to determine network
    std::string server_address = "tcp://localhost:12345"; // Default to localhost
    if (argc ==2 && std::strcmp(argv[1], "--docker") == 0)
    {
        server_address = "tcp://server:12345"; // Use Docker network
    }
    else{
        server_address = "tcp://localhost:12345";
    }

    while (true)
    {
        try
        {
            socket.connect(server_address);
            break;
        }
        catch (const zmq::error_t &e)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    timer.stop("ZMQ Setup", "Setup", emptyArgs);

    for (int i = 0; i < numIters; ++i)
    {

        unsigned int size = size_gen(gen);
        std::unordered_map<std::string, std::string> args = {{"iteration", std::to_string(i)}, {"size", std::to_string(size)}};
        timer.start("Client Processing", std::to_string(i), args);
        double result = processing(size);
        timer.stop("Client Processing", std::to_string(i), args);

        // Send the result
        std::string message = "RESULT: " + std::to_string(result);
        zmq::message_t request(message.size());
        memcpy(request.data(), message.c_str(), message.size());
        timer.start("Client Send", std::to_string(i), args);
        socket.send(request, zmq::send_flags::none);

        // Receive a response from the server (optional based on server's behavior)
        zmq::message_t reply;
        auto _ = socket.recv(reply, zmq::recv_flags::none);
        timer.stop("Server Send", std::to_string(i), args);
        std::string replyStr(static_cast<char *>(reply.data()), reply.size());

        std::cout << "Server replied with: " << replyStr << std::endl;
    }

    timer.dumpLogs();

    return 0;
}
