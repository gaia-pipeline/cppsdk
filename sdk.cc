#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "plugin.grpc.pb.h"
#include "sdk.h"

using std::string;
using std::unique_ptr;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using proto::Plugin;
using proto::Empty;
using proto::Job;
using proto::JobResult;
using proto::Argument;
using proto::ManualInteraction;

// General constants
static const string SERVER_CERT_ENV = "GAIA_PLUGIN_CERT";
static const string SERVER_KEY_ENV = "GAIA_PLUGIN_KEY";
static const string ROOT_CA_CERT_ENV = "GAIA_PLUGIN_CA_CERT";
static const string LISTEN_ADDRESS = "localhost";
static const int CORE_PROTOCOL_VERSION = 1;
static const int PROTOCOL_VERSION = 2;
static const string PROTOCOL_TYPE = "grpc";

// Error messages
static const string ERR_JOB_NOT_FOUND = "job not found in plugin";
static const string ERR_EXIT_PIPELINE = "pipeline exit requested by job";
static const string ERR_DUPLICATE_JOB = "duplicate job found (two jobs with the same title)";

class GRPCPluginImpl final : public Plugin::Service {
    public:
        Status GetJobs(ServerContext* context, const Empty* request, ServerWriter<Job>* writer) {
            // Iterate over all jobs and send every job to client (e.g. Gaia).
            list<job_wrapper>::iterator it = cached_jobs.begin();
            while (it != cached_jobs.end()) {
                writer->Write((*it).job);
                it++;
            }
            return Status::OK;
        }

        Status ExecuteJob(ServerContext* context, const Job* request, JobResult* response) {
            job_wrapper * job = GetJob((*request));
            if (job == nullptr) {
                return Status(grpc::StatusCode::CANCELLED, ERR_JOB_NOT_FOUND);
            }

            // Transform arguments.
            list<argument> args;
            for (int i = 0; i < (*request).args_size(); ++i) {
                argument arg = {};
                arg.key = (*request).args(i).key();
                arg.value = (*request).args(i).value();
                args.push_back(arg);
            }

            // Execute job function.
            try {
                (*job).handler(args);
            } catch (string e) {
                // Check if job wants to force exit pipeline.
                // We will exit the pipeline but not mark as 'failed'.
                if (e.compare(ERR_EXIT_PIPELINE) != 0) {
                   response->set_failed(true);
                }

                // Set log message and job id.
                response->set_exit_pipeline(true);
                response->set_message(e);
                response->set_unique_id((*job).job.unique_id());
            }

            return Status::OK;
        }

        void PushCachedJobs(job_wrapper* job) {
            cached_jobs.push_back(*job);
        }

    private:
        list<job_wrapper> cached_jobs;
        
        // GetJob finds the right job in the cache and returns it.
        job_wrapper * GetJob(const Job job) {
            list<job_wrapper>::iterator it = cached_jobs.begin();
            while (it != cached_jobs.end()) {
                if ((*it).job.unique_id() == job.unique_id()) {
                    return &(*it);
                }
                it++;
            }
            return nullptr;
        }
};

void Serve(list<job> jobs) {
    // Transform all given jobs to proto objects.
    list<job>::iterator it = jobs.begin();
    while (it != jobs.end()) {
        // Transform manual interaction.
        ManualInteraction* ma = new ManualInteraction();
        if ((*it).interaction != nullptr) {
            ma->set_description((*(*it).interaction).description);
            ma->set_type(ToString((*(*it).interaction).type));
            ma->set_value((*(*it).interaction).value);
        }
    }

    GRPCPluginImpl service;
    ServerBuilder builder;
    int * selectedPort = new int(0);
    builder.AddListeningPort(LISTEN_ADDRESS + string(":0"), grpc::InsecureServerCredentials(), selectedPort);
    unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << LISTEN_ADDRESS << ":" << *selectedPort << std::endl;
    delete selectedPort;
    server->Wait();
};

void example_job(list<argument> args) throw(string) {
    std::cout << "This is the output of an example job" << std::endl;
}

int main(int argc, char** argv) {
    list<job> jobs;
    job j;
    j.description = "Example job";
    j.title = "example job";
    j.handler = &example_job;
    jobs.push_back(j);

    Serve(jobs);
};
