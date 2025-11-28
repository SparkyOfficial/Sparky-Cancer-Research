#include "grpc_server.h"
#include <iostream>
#include <csignal>
#include <memory>

// Global server instance for signal handling
std::unique_ptr<tumordtwin::GrpcServer> g_server;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    if (g_server) {
        g_server->shutdown();
    }
}

int main(int argc, char** argv) {
    // Default server address
    std::string server_address = "0.0.0.0:50051";
    
    // Parse command line arguments
    if (argc > 1) {
        server_address = argv[1];
    }

    std::cout << "Tumor Digital Twin Backend Server" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "Starting gRPC server on " << server_address << std::endl;

    // Set up signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Create and start the server
    g_server = std::make_unique<tumordtwin::GrpcServer>(server_address);
    
    if (!g_server->start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    std::cout << "Server started successfully" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    // Wait for the server to shutdown
    g_server->wait();

    std::cout << "Server stopped" << std::endl;
    return 0;
}
