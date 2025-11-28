#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <atomic>

#include "service.grpc.pb.h"

namespace tumordtwin {

/**
 * @brief Implementation of the SimulationService gRPC service
 * 
 * This class provides the server-side implementation of all RPC methods
 * defined in the SimulationService. It handles simulation lifecycle,
 * status queries, and result retrieval.
 */
class SimulationServiceImpl final : public SimulationService::Service {
public:
    SimulationServiceImpl();
    ~SimulationServiceImpl() override = default;

    // RPC method implementations
    grpc::Status StartSimulation(
        grpc::ServerContext* context,
        const SimulationRequest* request,
        SimulationResponse* response) override;

    grpc::Status GetSimulationStatus(
        grpc::ServerContext* context,
        const StatusRequest* request,
        StatusResponse* response) override;

    grpc::Status GetSimulationResults(
        grpc::ServerContext* context,
        const ResultsRequest* request,
        grpc::ServerWriter<ResultsChunk>* writer) override;

    grpc::Status StopSimulation(
        grpc::ServerContext* context,
        const StopRequest* request,
        StopResponse* response) override;

    grpc::Status ListSimulations(
        grpc::ServerContext* context,
        const ListRequest* request,
        SimulationList* response) override;

    grpc::Status LoadSimulation(
        grpc::ServerContext* context,
        const LoadSimulationRequest* request,
        LoadSimulationResponse* response) override;

    grpc::Status HealthCheck(
        grpc::ServerContext* context,
        const HealthCheckRequest* request,
        HealthCheckResponse* response) override;

private:
    // Helper methods for request validation
    bool validateSimulationRequest(const SimulationRequest& request, std::string& error_msg);
    bool validateSimulationParameters(const SimulationParameters& params, std::string& error_msg);
    bool validatePatientData(const PatientData& data, std::string& error_msg);
    
    // Generate unique simulation ID
    std::string generateSimulationId();
    
    // Server state
    std::atomic<bool> is_serving_{true};
};

/**
 * @brief gRPC server wrapper class
 * 
 * Manages the lifecycle of the gRPC server, including initialization,
 * starting, and graceful shutdown.
 */
class GrpcServer {
public:
    /**
     * @brief Construct a new GrpcServer
     * @param server_address Address to bind the server to (e.g., "0.0.0.0:50051")
     */
    explicit GrpcServer(const std::string& server_address);
    
    ~GrpcServer();

    /**
     * @brief Start the gRPC server
     * @return true if server started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Wait for the server to shutdown
     * 
     * This is a blocking call that waits until the server is shut down.
     */
    void wait();

    /**
     * @brief Shutdown the server gracefully
     */
    void shutdown();

    /**
     * @brief Check if the server is running
     * @return true if server is running, false otherwise
     */
    bool isRunning() const { return is_running_; }

private:
    std::string server_address_;
    std::unique_ptr<SimulationServiceImpl> service_;
    std::unique_ptr<grpc::Server> server_;
    std::atomic<bool> is_running_{false};
};

} // namespace tumordtwin
