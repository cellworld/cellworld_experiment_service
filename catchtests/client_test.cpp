#include <catch.h>
#include <experiment.h>

using namespace cell_world;
using namespace experiment;
using namespace std;
using namespace this_thread;

struct Client : experiment::Experiment_client {
    void on_experiment_started(const Start_experiment_response &experiment) override{
        cout << "on_experiment_started: " <<  experiment << endl;
    }
    void on_episode_started(const std::string &experiment_name) override {
        cout << "on_episode_started: " <<  experiment_name << endl;
    }
    void on_episode_finished() override{
        cout << "on_episode_finished: " << endl;
    }
    void on_experiment_finished(const std::string &experiment_name) override{
        cout << "on_experiment_finished: " <<  experiment_name << endl;
        disconnect();
    }
};

TEST_CASE("client_test") {
    auto t1 = thread ([]() {
        Client client;
        client.connect("127.0.0.1");
        client.subscribe();
        client.join();
    });
    World_info wi;
    wi.world_configuration = "hexagonal";
    wi.world_implementation = "cv";
    wi.occlusions = "10_05";
    Client client;
    client.connect("127.0.0.1");
    sleep_for(1s);
    auto experiment = client.start_experiment(wi,"test_subject",10,"prefix","suffix");
    sleep_for(1s);
    client.start_episode(experiment.experiment_name);
    sleep_for(10s);
    client.finish_episode();
    sleep_for(1s);
    client.start_episode(experiment.experiment_name);
    sleep_for(10s);
    client.finish_episode();
    sleep_for(1s);
    client.start_episode(experiment.experiment_name);
    sleep_for(10s);
    client.finish_episode();
    sleep_for(1s);
    client.start_episode(experiment.experiment_name);
    sleep_for(10s);
    client.finish_episode();
    sleep_for(1s);
    client.start_episode(experiment.experiment_name);
    sleep_for(10s);
    client.finish_episode();
    sleep_for(1s);
    client.finish_experiment(experiment.experiment_name);
    sleep_for(1s);
    client.disconnect();
    t1.join();
}