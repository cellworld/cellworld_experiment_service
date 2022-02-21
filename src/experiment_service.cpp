#include <experiment/experiment_service.h>
#include <filesystem>

using namespace tcp_messages;
using namespace cell_world;
using namespace std;


namespace experiment {
    string active_experiment = "";
    Episode active_episode;
    bool episode_in_progress = false;
    Experiment_tracking_client *tracking_client = nullptr;
    string tracking_service_ip = "";
    string logs_path = "";

    string get_experiment_file(const string &experiment_name){
        return logs_path + experiment_name + ".json";
    }

    Start_experiment_response Experiment_service::start_experiment(const Start_experiment_request &parameters) {
        Experiment new_experiment;
        new_experiment.world_configuration_name = parameters.world.world_configuration;
        new_experiment.world_implementation_name = parameters.world.world_implementation;
        new_experiment.occlusions = parameters.world.occlusions;
        new_experiment.duration = parameters.duration;
        new_experiment.subject_name = parameters.subject_name;
        new_experiment.start_time = json_cpp::Json_date::now();
        new_experiment.set_name(parameters.prefix, parameters.suffix);
        new_experiment.save(get_experiment_file(new_experiment.name));
        Start_experiment_response response;
        response.experiment_name = new_experiment.name;
        response.start_date = new_experiment.start_time;
        response.subject_name = parameters.subject_name;
        response.world = parameters.world;
        response.duration = parameters.duration;
        broadcast_subscribed(tcp_messages::Message("experiment_started",response));
        return response;
    }

    bool Experiment_service::start_episode(const Start_episode_request &parameters) {
        if (episode_in_progress) return false;
        if (cell_world::file_exists(get_experiment_file(parameters.experiment_name))){

            active_experiment = parameters.experiment_name;
            active_episode = Episode();
            episode_in_progress = true;
            if (!tracking_service_ip.empty()) {
                tracking_client = new Experiment_tracking_client();
                tracking_client->connect("127.0.0.1");
                tracking_client->register_consumer();
            }
            broadcast_subscribed(tcp_messages::Message("episode_started",active_experiment));
            return true;
        }
        return false;
    }

    bool Experiment_service::finish_episode() {
        if (!episode_in_progress) return false;
        if (!cell_world::file_exists(get_experiment_file(active_experiment))) return false;
        auto experiment = json_cpp::Json_from_file<Experiment>(get_experiment_file(active_experiment));
        active_episode.end_time = json_cpp::Json_date::now();
        experiment.episodes.push_back(active_episode);
        experiment.save(get_experiment_file(active_experiment));
        episode_in_progress = false;
        if (tracking_client) {
            tracking_client->unregister_consumer();
            tracking_client->disconnect();
            delete tracking_client;
            tracking_client = nullptr;
        }
        broadcast_subscribed(tcp_messages::Message("episode_finished",active_experiment));
        return true;
    }

    bool Experiment_service::finish_experiment(const Finish_experiment_request &parameters) {
        if (!cell_world::file_exists(get_experiment_file(active_experiment))) return false;
        auto experiment = json_cpp::Json_from_file<Experiment>(get_experiment_file(parameters.experiment_name));
        auto end_time = experiment.start_time + chrono::minutes(experiment.duration);
        float remaining = ((float)(end_time - json_cpp::Json_date::now()).count()) / 1000;
        if (remaining>0){
            experiment.duration = (unsigned int) (((float)(json_cpp::Json_date::now() - experiment.start_time).count()) / 1000 / 60);
            experiment.save(get_experiment_file(parameters.experiment_name));
        }
        broadcast_subscribed(tcp_messages::Message("experiment_finished",parameters.experiment_name));
        return true;
    }

    Get_experiment_response Experiment_service::get_experiment(const Get_experiment_request &parameters) {
        Get_experiment_response response;
        if (!cell_world::file_exists(get_experiment_file(parameters.experiment_name))) return response;
        auto experiment = json_cpp::Json_from_file<Experiment>(get_experiment_file(parameters.experiment_name));
        auto end_time = experiment.start_time + chrono::minutes(experiment.duration);
        float remaining = ((float)(end_time - json_cpp::Json_date::now()).count()) / 1000;
        if (remaining<0) remaining = 0;
        response.world_info.world_configuration = experiment.world_configuration_name;
        response.world_info.world_implementation = experiment.world_implementation_name;
        response.world_info.occlusions = experiment.occlusions;
        response.experiment_name = experiment.name;
        response.start_date = experiment.start_time;
        response.duration = experiment.duration;
        response.episode_count = experiment.episodes.size();
        response.remaining_time = remaining;
        return response;
    }

    bool Experiment_service::set_tracking_service_ip(const string &ip) {
        tracking_service_ip = ip;
        return true;
    }

    int Experiment_service::get_port() {
        string port_str(std::getenv("CELLWORLD_EXPERIMENT_SERVICE_PORT") ? std::getenv("CELLWORLD_EXPERIMENT_SERVICE_PORT") : "4540");
        return atoi(port_str.c_str());
    }

    void Experiment_service::set_logs_folder(const string &path) {
        logs_path = path;
        filesystem::create_directory(filesystem::path(logs_path));
    }

    bool Experiment_service::capture(const Capture_request &request) {
        if (episode_in_progress) {
            active_episode.captures.push_back(request.frame);
            broadcast_subscribed(Message("capture",request.frame));
            return true;
        }
        return false;
    }

    bool Experiment_service::set_behavior(const Set_behavior_request &request) {
        broadcast_subscribed(Message("behavior_set", request.behavior));
        return true;
    }

    void Experiment_tracking_client::on_step(const Step &step) {
        if (episode_in_progress){
            active_episode.trajectories.push_back(step);
        }
    }
}