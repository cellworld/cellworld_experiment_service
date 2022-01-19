#include <tcp_messages.h>
#include <experiment.h>

using namespace std;
using namespace tcp_messages;
using namespace experiment;

int main (int argc, char **argv){
    Message_server<Experiment_service> server;
    Experiment_service::connect_tracking("127.0.0.1");
    server.start(4590);
    server.join();
}