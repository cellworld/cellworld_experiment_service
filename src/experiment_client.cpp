#include <experiment/experiment_client.h>
#include <experiment/experiment_messages.h>
#include <cell_world.h>

using namespace cell_world;

namespace experiment {
    Experiment Experiment_client::start_experiment(const cell_world::World_info &world, const std::string &subject_name, int duration,
                                                   const std::string &prefix , const std::string &suffix) {
        auto parameters = Start_experiment_request();
        parameters.prefix = prefix;
        parameters.suffix = suffix;
        parameters.world = world;
        parameters.subject_name = subject_name;
        parameters.duration = duration;
        return send_request(tcp_messages::Message("start_experiment",parameters)).get_body<Experiment>();
    }

    bool Experiment_client::start_episode(const std::string &experiment_name) {
        auto parameters = Start_episode_request();
        parameters.experiment_name = experiment_name;
        return send_request(tcp_messages::Message("start_episode",parameters)).get_body<bool>();
    }

    bool Experiment_client::finish_episode() {
        return send_request(tcp_messages::Message("finish_episode")).get_body<bool>();
    }

    bool Experiment_client::finish_experiment(const std::string &experiment_name) {
        auto parameters = Finish_experiment_request();
        parameters.experiment_name = experiment_name;
        return send_request(tcp_messages::Message("finish_experiment",parameters)).get_body<bool>();
    }

}