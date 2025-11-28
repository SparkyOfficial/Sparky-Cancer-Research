#include <catch2/catch_test_macros.hpp>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>

#include "grpc_server.h"
#include "service.grpc.pb.h"

using namespace tumordtwin;

// Helper class to manage server lifecycle in tests
class TestServerFixture {
public:
    TestServerFixture() : server_address_("localhost:50052") {
        server_ = std::make_unique<GrpcServer>(server_address_);
    }

    ~TestServerFixture() {
        stopServer();
    }

    bool startServer() {
        if (!server_->start()) {
            return false;
        }
        
        // Give the server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return true;
    }

    void stopServer() {
        if (server_) {
            server_->shutdown();
        }
    }

    std::string getServerAddress() const {
        return server_address_;
    }

    std::unique_ptr<SimulationService::Stub> createStub() {
        auto channel = grpc::CreateChannel(
            server_address_,
            grpc::InsecureChannelCredentials()
        );
        return SimulationService::NewStub(channel);
    }

private:
    std::string server_address_;
    std::unique_ptr<GrpcServer> server_;
};

// Helper function to create valid simulation parameters
SimulationParameters createValidParameters() {
    SimulationParameters params;
    params.set_grid_size_x(100);
    params.set_grid_size_y(100);
    params.set_grid_size_z(100);
    params.set_spatial_resolution(10.0);
    params.set_num_steps(100);
    params.set_time_step(0.1);
    params.set_mutation_rate(0.001);
    params.set_division_rate(0.1);
    params.set_death_rate(0.05);
    params.set_migration_rate(0.01);
    params.set_oxygen_diffusion_coeff(1.0);
    params.set_glucose_diffusion_coeff(0.8);
    params.set_checkpoint_interval(10);
    params.set_num_threads(4);
    params.set_num_mpi_ranks(1);
    params.set_use_gpu(false);
    return params;
}

// Helper function to create valid patient data
PatientData createValidPatientData() {
    PatientData data;
    data.set_patient_id("test_patient_001");
    
    // Add minimal DICOM data
    auto* dicom = data.mutable_dicom();
    dicom->set_patient_id("test_patient_001");
    dicom->set_dicom_archive("dummy_dicom_data");
    dicom->set_modality("CT");
    
    return data;
}

// ============================================================================
// Test Cases
// ============================================================================

TEST_CASE("gRPC server starts successfully", "[grpc][server][initialization]") {
    TestServerFixture fixture;
    
    SECTION("Server can be started") {
        bool started = fixture.startServer();
        REQUIRE(started == true);
    }
    
    SECTION("Server can be started and stopped") {
        REQUIRE(fixture.startServer() == true);
        fixture.stopServer();
        // If we get here without hanging, the test passes
        REQUIRE(true);
    }
}

TEST_CASE("Health check endpoint responds", "[grpc][server][health]") {
    TestServerFixture fixture;
    REQUIRE(fixture.startServer());
    
    auto stub = fixture.createStub();
    
    grpc::ClientContext context;
    HealthCheckRequest request;
    HealthCheckResponse response;
    
    grpc::Status status = stub->HealthCheck(&context, request, &response);
    
    REQUIRE(status.ok());
    REQUIRE(response.status() == HealthCheckResponse::SERVING);
    REQUIRE(!response.message().empty());
}

TEST_CASE("StartSimulation with valid request", "[grpc][server][start]") {
    TestServerFixture fixture;
    REQUIRE(fixture.startServer());
    
    auto stub = fixture.createStub();
    
    grpc::ClientContext context;
    SimulationRequest request;
    request.set_patient_id("test_patient_001");
    request.set_simulation_name("Test Simulation");
    *request.mutable_data() = createValidPatientData();
    *request.mutable_params() = createValidParameters();
    
    SimulationResponse response;
    grpc::Status status = stub->StartSimulation(&context, request, &response);
    
    REQUIRE(status.ok());
    REQUIRE(!response.simulation_id().empty());
    REQUIRE(response.status() == SimulationStatus::QUEUED);
    REQUIRE(!response.message().empty());
}

TEST_CASE("Invalid requests are rejected", "[grpc][server][validation]") {
    TestServerFixture fixture;
    REQUIRE(fixture.startServer());
    
    auto stub = fixture.createStub();
    
    SECTION("Empty patient ID is rejected") {
        grpc::ClientContext context;
        SimulationRequest request;
        request.set_patient_id("");  // Invalid: empty
        *request.mutable_data() = createValidPatientData();
        *request.mutable_params() = createValidParameters();
        
        SimulationResponse response;
        grpc::Status status = stub->StartSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
    
    SECTION("Missing patient data is rejected") {
        grpc::ClientContext context;
        SimulationRequest request;
        request.set_patient_id("test_patient_001");
        // No patient data set
        *request.mutable_params() = createValidParameters();
        
        SimulationResponse response;
        grpc::Status status = stub->StartSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
    
    SECTION("Missing simulation parameters is rejected") {
        grpc::ClientContext context;
        SimulationRequest request;
        request.set_patient_id("test_patient_001");
        *request.mutable_data() = createValidPatientData();
        // No parameters set
        
        SimulationResponse response;
        grpc::Status status = stub->StartSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
    
    SECTION("Invalid grid dimensions are rejected") {
        grpc::ClientContext context;
        SimulationRequest request;
        request.set_patient_id("test_patient_001");
        *request.mutable_data() = createValidPatientData();
        
        auto params = createValidParameters();
        params.set_grid_size_x(-10);  // Invalid: negative
        *request.mutable_params() = params;
        
        SimulationResponse response;
        grpc::Status status = stub->StartSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
    
    SECTION("Invalid time step is rejected") {
        grpc::ClientContext context;
        SimulationRequest request;
        request.set_patient_id("test_patient_001");
        *request.mutable_data() = createValidPatientData();
        
        auto params = createValidParameters();
        params.set_time_step(-0.1);  // Invalid: negative
        *request.mutable_params() = params;
        
        SimulationResponse response;
        grpc::Status status = stub->StartSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
    
    SECTION("Invalid mutation rate is rejected") {
        grpc::ClientContext context;
        SimulationRequest request;
        request.set_patient_id("test_patient_001");
        *request.mutable_data() = createValidPatientData();
        
        auto params = createValidParameters();
        params.set_mutation_rate(1.5);  // Invalid: > 1.0
        *request.mutable_params() = params;
        
        SimulationResponse response;
        grpc::Status status = stub->StartSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
    
    SECTION("Grid size too large is rejected") {
        grpc::ClientContext context;
        SimulationRequest request;
        request.set_patient_id("test_patient_001");
        *request.mutable_data() = createValidPatientData();
        
        auto params = createValidParameters();
        params.set_grid_size_x(10000);  // 10000^3 = 1 trillion cells
        params.set_grid_size_y(10000);
        params.set_grid_size_z(10000);
        *request.mutable_params() = params;
        
        SimulationResponse response;
        grpc::Status status = stub->StartSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
}

TEST_CASE("GetSimulationStatus works", "[grpc][server][status]") {
    TestServerFixture fixture;
    REQUIRE(fixture.startServer());
    
    auto stub = fixture.createStub();
    
    SECTION("Valid simulation ID returns status") {
        grpc::ClientContext context;
        StatusRequest request;
        request.set_simulation_id("test-sim-id-123");
        
        StatusResponse response;
        grpc::Status status = stub->GetSimulationStatus(&context, request, &response);
        
        REQUIRE(status.ok());
        REQUIRE(response.simulation_id() == "test-sim-id-123");
    }
    
    SECTION("Empty simulation ID is rejected") {
        grpc::ClientContext context;
        StatusRequest request;
        request.set_simulation_id("");
        
        StatusResponse response;
        grpc::Status status = stub->GetSimulationStatus(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
}

TEST_CASE("StopSimulation works", "[grpc][server][stop]") {
    TestServerFixture fixture;
    REQUIRE(fixture.startServer());
    
    auto stub = fixture.createStub();
    
    SECTION("Valid stop request succeeds") {
        grpc::ClientContext context;
        StopRequest request;
        request.set_simulation_id("test-sim-id-123");
        request.set_save_checkpoint(true);
        
        StopResponse response;
        grpc::Status status = stub->StopSimulation(&context, request, &response);
        
        REQUIRE(status.ok());
        REQUIRE(response.success() == true);
        REQUIRE(!response.checkpoint_path().empty());
    }
    
    SECTION("Empty simulation ID is rejected") {
        grpc::ClientContext context;
        StopRequest request;
        request.set_simulation_id("");
        
        StopResponse response;
        grpc::Status status = stub->StopSimulation(&context, request, &response);
        
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
}

TEST_CASE("ListSimulations works", "[grpc][server][list]") {
    TestServerFixture fixture;
    REQUIRE(fixture.startServer());
    
    auto stub = fixture.createStub();
    
    grpc::ClientContext context;
    ListRequest request;
    
    SimulationList response;
    grpc::Status status = stub->ListSimulations(&context, request, &response);
    
    REQUIRE(status.ok());
    REQUIRE(response.total_count() == 0);  // No simulations yet
}

TEST_CASE("GetSimulationResults works", "[grpc][server][results]") {
    TestServerFixture fixture;
    REQUIRE(fixture.startServer());
    
    auto stub = fixture.createStub();
    
    SECTION("Valid results request succeeds") {
        grpc::ClientContext context;
        ResultsRequest request;
        request.set_simulation_id("test-sim-id-123");
        request.set_include_agents(true);
        request.set_include_grid_data(true);
        
        ResultsChunk chunk;
        auto reader = stub->GetSimulationResults(&context, request);
        
        bool received_chunk = false;
        while (reader->Read(&chunk)) {
            received_chunk = true;
            REQUIRE(chunk.simulation_id() == "test-sim-id-123");
        }
        
        grpc::Status status = reader->Finish();
        REQUIRE(status.ok());
        REQUIRE(received_chunk == true);
    }
    
    SECTION("Empty simulation ID is rejected") {
        grpc::ClientContext context;
        ResultsRequest request;
        request.set_simulation_id("");
        
        auto reader = stub->GetSimulationResults(&context, request);
        ResultsChunk chunk;
        
        // Try to read (should fail)
        reader->Read(&chunk);
        
        grpc::Status status = reader->Finish();
        REQUIRE(!status.ok());
        REQUIRE(status.error_code() == grpc::StatusCode::INVALID_ARGUMENT);
    }
}
