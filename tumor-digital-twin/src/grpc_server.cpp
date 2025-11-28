#include "grpc_server.h"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace tumordtwin {

// ============================================================================
// SimulationServiceImpl Implementation
// ============================================================================

SimulationServiceImpl::SimulationServiceImpl() {
    // Initialize service
}

grpc::Status SimulationServiceImpl::StartSimulation(
    grpc::ServerContext* context,
    const SimulationRequest* request,
    SimulationResponse* response) {
    
    // Validate request
    std::string error_msg;
    if (!validateSimulationRequest(*request, error_msg)) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_msg);
    }

    // Generate unique simulation ID
    std::string sim_id = generateSimulationId();
    
    // Set response fields
    response->set_simulation_id(sim_id);
    response->set_status(SimulationStatus::QUEUED);
    response->set_message("Simulation queued successfully");
    
    // Set estimated completion time (placeholder - 1 hour from now)
    auto now = std::chrono::system_clock::now();
    auto future = now + std::chrono::hours(1);
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        future.time_since_epoch()).count();
    response->set_estimated_completion_time(timestamp);

    return grpc::Status::OK;
}

grpc::Status SimulationServiceImpl::GetSimulationStatus(
    grpc::ServerContext* context,
    const StatusRequest* request,
    StatusResponse* response) {
    
    // Validate simulation ID
    if (request->simulation_id().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, 
                          "Simulation ID cannot be empty");
    }

    // Stub implementation - return placeholder status
    response->set_simulation_id(request->simulation_id());
    response->set_status(SimulationStatus::QUEUED);
    response->set_current_step(0);
    response->set_total_steps(100);
    response->set_progress_percentage(0.0);
    response->set_estimated_time_remaining(3600);
    response->set_message("Simulation is queued");

    return grpc::Status::OK;
}

grpc::Status SimulationServiceImpl::GetSimulationResults(
    grpc::ServerContext* context,
    const ResultsRequest* request,
    grpc::ServerWriter<ResultsChunk>* writer) {
    
    // Validate simulation ID
    if (request->simulation_id().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "Simulation ID cannot be empty");
    }

    // Stub implementation - send a single empty chunk
    ResultsChunk chunk;
    chunk.set_simulation_id(request->simulation_id());
    chunk.set_chunk_number(0);
    chunk.set_total_chunks(1);
    chunk.set_is_final(true);
    chunk.set_data("");  // Empty data for now

    writer->Write(chunk);

    return grpc::Status::OK;
}

grpc::Status SimulationServiceImpl::StopSimulation(
    grpc::ServerContext* context,
    const StopRequest* request,
    StopResponse* response) {
    
    // Validate simulation ID
    if (request->simulation_id().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "Simulation ID cannot be empty");
    }

    // Stub implementation
    response->set_simulation_id(request->simulation_id());
    response->set_success(true);
    response->set_message("Simulation stop requested");
    
    if (request->save_checkpoint()) {
        response->set_checkpoint_path("/tmp/checkpoint_" + request->simulation_id());
    }

    return grpc::Status::OK;
}

grpc::Status SimulationServiceImpl::ListSimulations(
    grpc::ServerContext* context,
    const ListRequest* request,
    SimulationList* response) {
    
    // Stub implementation - return empty list
    response->set_total_count(0);

    return grpc::Status::OK;
}

grpc::Status SimulationServiceImpl::LoadSimulation(
    grpc::ServerContext* context,
    const LoadSimulationRequest* request,
    LoadSimulationResponse* response) {
    
    // Validate simulation ID
    if (request->simulation_id().empty()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "Simulation ID cannot be empty");
    }

    // Stub implementation
    response->set_simulation_id(request->simulation_id());
    response->set_success(false);
    response->set_message("Load simulation not yet implemented");

    return grpc::Status::OK;
}

grpc::Status SimulationServiceImpl::HealthCheck(
    grpc::ServerContext* context,
    const HealthCheckRequest* request,
    HealthCheckResponse* response) {
    
    // Health check always returns SERVING if the server is running
    if (is_serving_) {
        response->set_status(HealthCheckResponse::SERVING);
        response->set_message("Service is healthy");
    } else {
        response->set_status(HealthCheckResponse::NOT_SERVING);
        response->set_message("Service is shutting down");
    }

    return grpc::Status::OK;
}

// ============================================================================
// Validation Methods
// ============================================================================

bool SimulationServiceImpl::validateSimulationRequest(
    const SimulationRequest& request, 
    std::string& error_msg) {
    
    // Validate patient ID
    if (request.patient_id().empty()) {
        error_msg = "Patient ID is required";
        return false;
    }

    // Validate patient data
    if (!request.has_data()) {
        error_msg = "Patient data is required";
        return false;
    }
    
    if (!validatePatientData(request.data(), error_msg)) {
        return false;
    }

    // Validate simulation parameters
    if (!request.has_params()) {
        error_msg = "Simulation parameters are required";
        return false;
    }
    
    if (!validateSimulationParameters(request.params(), error_msg)) {
        return false;
    }

    return true;
}

bool SimulationServiceImpl::validateSimulationParameters(
    const SimulationParameters& params,
    std::string& error_msg) {
    
    // Validate grid dimensions
    if (params.grid_size_x() <= 0 || params.grid_size_y() <= 0 || params.grid_size_z() <= 0) {
        error_msg = "Grid dimensions must be positive";
        return false;
    }

    // Check for reasonable grid size (prevent memory exhaustion)
    const int64_t max_grid_cells = 1000LL * 1000LL * 1000LL;  // 1 billion cells
    int64_t total_cells = static_cast<int64_t>(params.grid_size_x()) * 
                         static_cast<int64_t>(params.grid_size_y()) * 
                         static_cast<int64_t>(params.grid_size_z());
    
    if (total_cells > max_grid_cells) {
        error_msg = "Grid size too large (exceeds 1 billion cells)";
        return false;
    }

    // Validate spatial resolution
    if (params.spatial_resolution() <= 0.0) {
        error_msg = "Spatial resolution must be positive";
        return false;
    }

    // Validate time parameters
    if (params.num_steps() <= 0) {
        error_msg = "Number of steps must be positive";
        return false;
    }

    if (params.time_step() <= 0.0) {
        error_msg = "Time step must be positive";
        return false;
    }

    // Validate biological parameters
    if (params.mutation_rate() < 0.0 || params.mutation_rate() > 1.0) {
        error_msg = "Mutation rate must be between 0 and 1";
        return false;
    }

    if (params.division_rate() < 0.0) {
        error_msg = "Division rate must be non-negative";
        return false;
    }

    if (params.death_rate() < 0.0) {
        error_msg = "Death rate must be non-negative";
        return false;
    }

    if (params.migration_rate() < 0.0) {
        error_msg = "Migration rate must be non-negative";
        return false;
    }

    // Validate diffusion coefficients
    if (params.oxygen_diffusion_coeff() < 0.0) {
        error_msg = "Oxygen diffusion coefficient must be non-negative";
        return false;
    }

    if (params.glucose_diffusion_coeff() < 0.0) {
        error_msg = "Glucose diffusion coefficient must be non-negative";
        return false;
    }

    // Validate checkpoint interval
    if (params.checkpoint_interval() < 0) {
        error_msg = "Checkpoint interval must be non-negative";
        return false;
    }

    // Validate parallelization parameters
    if (params.num_threads() < 0) {
        error_msg = "Number of threads must be non-negative";
        return false;
    }

    if (params.num_mpi_ranks() < 0) {
        error_msg = "Number of MPI ranks must be non-negative";
        return false;
    }

    return true;
}

bool SimulationServiceImpl::validatePatientData(
    const PatientData& data,
    std::string& error_msg) {
    
    // Check if at least one data source is provided
    bool has_dicom = data.has_dicom() && !data.dicom().dicom_archive().empty();
    bool has_vcf = data.has_vcf() && data.vcf().mutations_size() > 0;
    bool has_genomic = data.has_genomic_sequences() && 
                      (!data.genomic_sequences().bam_data().empty() || 
                       !data.genomic_sequences().fastq_data().empty());

    if (!has_dicom && !has_vcf && !has_genomic) {
        error_msg = "At least one data source (DICOM, VCF, or genomic sequences) is required";
        return false;
    }

    // Validate DICOM data if present
    if (has_dicom) {
        if (data.dicom().patient_id().empty()) {
            error_msg = "DICOM patient ID is required";
            return false;
        }
    }

    // Validate VCF data if present
    if (has_vcf) {
        if (data.vcf().sample_id().empty()) {
            error_msg = "VCF sample ID is required";
            return false;
        }
    }

    // Validate genomic sequence data if present
    if (has_genomic) {
        if (data.genomic_sequences().sample_id().empty()) {
            error_msg = "Genomic sequence sample ID is required";
            return false;
        }
    }

    return true;
}

std::string SimulationServiceImpl::generateSimulationId() {
    // Generate a UUID-like simulation ID
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << (dis(gen) & 0xFFFFFFFF) << "-";
    ss << std::setw(4) << (dis(gen) & 0xFFFF) << "-";
    ss << std::setw(4) << ((dis(gen) & 0x0FFF) | 0x4000) << "-";
    ss << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000) << "-";
    ss << std::setw(12) << (dis(gen) & 0xFFFFFFFFFFFF);

    return ss.str();
}

// ============================================================================
// GrpcServer Implementation
// ============================================================================

GrpcServer::GrpcServer(const std::string& server_address)
    : server_address_(server_address),
      service_(std::make_unique<SimulationServiceImpl>()) {
}

GrpcServer::~GrpcServer() {
    if (is_running_) {
        shutdown();
    }
}

bool GrpcServer::start() {
    if (is_running_) {
        return false;  // Already running
    }

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;
    
    // Listen on the given address without authentication
    builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());
    
    // Register the service
    builder.RegisterService(service_.get());

    // Build and start the server
    server_ = builder.BuildAndStart();
    
    if (!server_) {
        return false;
    }

    is_running_ = true;
    return true;
}

void GrpcServer::wait() {
    if (server_) {
        server_->Wait();
    }
}

void GrpcServer::shutdown() {
    if (server_ && is_running_) {
        server_->Shutdown();
        is_running_ = false;
    }
}

} // namespace tumordtwin
