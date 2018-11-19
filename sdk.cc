#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "plugin.grpc.pb.h"

using std::string;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using proto::Plugin;
using proto::Empty;
using proto::Job;
using proto::JobResult;

static const string SERVER_CERT_ENV = "GAIA_PLUGIN_CERT";
static const string SERVER_KEY_ENV = "GAIA_PLUGIN_KEY";
static const string ROOT_CA_CERT_ENV = "GAIA_PLUGIN_CA_CERT";
static const string LISTEN_ADDRESS = "localhost:0";

class GRPCPluginImpl final : public Plugin::Service {
    public:
        Status GetJobs(ServerContext* context, const Empty* request, ServerWriter<Job>* writer) {

            return Status::OK;
        }

        Status ExecuteJob(ServerContext* context, const Job* request, JobResult* response) {

            return Status::OK;
        }
};

void Serve() {
    GRPCPluginImpl service;
    ServerBuilder builder;
    int * selectedPort = new int(0);
    builder.AddListeningPort(LISTEN_ADDRESS, grpc::InsecureServerCredentials(), selectedPort);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on localhost:" << *selectedPort << std::endl;
    server->Wait();
};

int main(int argc, char** argv) {
    Serve();
};
