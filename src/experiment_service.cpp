#include <experiment/experiment_service.h>

using namespace cell_world;
using namespace std;


namespace experiment {
    string active_experiment = "";
    Episode active_episode;
    bool episode_in_progress = false;
    Experiment_tracking_client tracking_client;


    string get_experiment_file(const string &experiment_name){
        return "logs/" + experiment_name;
    }

    cell_world::Experiment Experiment_service::start_experiment(const Start_experiment_request &parameters) {
        Experiment new_experiment;
        new_experiment.world_configuration_name = parameters.world.world_configuration;
        new_experiment.world_implementation_name = parameters.world.world_implementation;
        new_experiment.occlusions = parameters.world.occlusions;
        new_experiment.duration = parameters.duration;
        new_experiment.subject_name = parameters.subject_name;
        new_experiment.start_time = json_cpp::Json_date::now();
        new_experiment.set_name(parameters.prefix, parameters.suffix);
        new_experiment.save(get_experiment_file(new_experiment.name));
        broadcast_subscribed(tcp_messages::Message("experiment_started",new_experiment));
        return new_experiment;
    }

    bool Experiment_service::start_episode(const Start_episode_request &parameters) {
        if (episode_in_progress) return false;
        if (cell_world::file_exists(get_experiment_file(parameters.experiment_name))){
            active_experiment = parameters.experiment_name;
            active_episode = Episode();
            episode_in_progress = true;
            tracking_client.register_consumer();
            broadcast_subscribed(tcp_messages::Message("episode_started",active_experiment));
            return true;
        }
        return false;
    }

    bool Experiment_service::finish_episode() {
        if (!episode_in_progress) return false;
        if (!cell_world::file_exists(get_experiment_file(active_experiment))) return false;
        auto experiment = json_cpp::Json_from_file<Experiment>(get_experiment_file(active_experiment));
        active_experiment = "";
        active_episode.end_time = json_cpp::Json_date::now();
        experiment.episodes.push_back(active_episode);
        experiment.save(get_experiment_file(active_experiment));
        episode_in_progress = false;
        tracking_client.unregister_consumer();
        broadcast_subscribed(tcp_messages::Message("episode_finished",active_experiment));
        return true;
    }

    bool Experiment_service::finish_experiment(const Finish_experiment_request &parameters) {
        broadcast_subscribed(tcp_messages::Message("experiment_finished",parameters.experiment_name));
        return true;
    }

    bool Experiment_service::connect_tracking(const string &ip) {
        return tracking_client.connect("127.0.0.1");
    }

    void Experiment_service::disconnect_tracking() {
        tracking_client.disconnect();
    }

    bool Experiment_service::get_experiment_state(const Get_experiment_state_request &parameters) {
        if (!cell_world::file_exists(get_experiment_file(parameters.experiment_name))) return false;
        auto experiment = json_cpp::Json_from_file<Experiment>(get_experiment_file(parameters.experiment_name));
        auto end_time = experiment.start_time + chrono::seconds(experiment.duration);
        return end_time > json_cpp::Json_date::now();
    }

    void Experiment_tracking_client::on_step(const Step &step) {
        cout << "new_step: " << step << endl;
        active_episode.trajectories.emplace_back(step);
    }
}