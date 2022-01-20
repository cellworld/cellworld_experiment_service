#include <tcp_messages.h>
#include <experiment.h>
#include <params_cpp.h>

using namespace std;
using namespace tcp_messages;
using namespace experiment;
using namespace params_cpp;

int main (int argc, char **argv){
    Parser parser(argc,argv);
    Key tracking_service_ip("-tip", "-tracking_ip", "-tracking_service_ip");
    Message_server<Experiment_service> server;
    Experiment_service::set_tracking_service_ip(parser.get(tracking_service_ip));
    server.start(4590);
    server.join();
}