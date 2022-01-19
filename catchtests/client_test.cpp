#include <catch.h>
#include <experiment.h>


using namespace cell_world;
using namespace experiment;
using namespace std;
using namespace this_thread;

std::atomic<bool> stop;

struct Client : experiment::Experiment_client {
    void on_experiment_started(const cell_world::Experiment &experiment) override{
        cout << "on_experiment_started: " <<  experiment << endl;
    }
    void on_episode_started(const std::string &experiment_name) override {
        cout << "on_episode_started: " <<  experiment_name << endl;
    }
    void on_episode_finished(const std::string &experiment_name) override{
        cout << "on_episode_finished: " <<  experiment_name << endl;
    }
    void on_experiment_finished(const std::string &experiment_name) override{
        cout << "on_experiment_finished: " <<  experiment_name << endl;
    }
};

TEST_CASE("client_test") {
    auto t1 = thread ([]() {
        Client client;
        client.connect("127.0.0.1", 4590);
        client.subscribe();
        client.join();
    });
    auto t2 = thread ([]() {
        World_info wi;
        wi.world_configuration = "hexagonal";
        wi.world_implementation = "cv";
        wi.occlusions = "10_05";
        Client client;
        client.connect("127.0.0.1", 4590);
        sleep_for(1s);
        auto experiment = client.start_experiment(wi,"test_subject",10,"prefix","suffix");
        sleep_for(1s);
        client.start_episode(experiment.name);
        sleep_for(4s);
        client.finish_episode();
        sleep_for(1s);
        client.finish_experiment(experiment.name);
        client.join();
    });
}