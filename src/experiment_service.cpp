#include <experiment/experiment_service.h>
#include <filesystem>

using namespace tcp_messages;
using namespace cell_world;
using namespace std;


namespace experiment {
    std::string logs_path = "";

    string get_experiment_prefix(const string &experiment_name){
        return experiment_name.substr(0,experiment_name.find('_'));
    }

    string get_experiment_file(const string &experiment_name){
        return logs_path + get_experiment_prefix(experiment_name) + '/' + experiment_name + "/" + experiment_name + "_experiment.json";
    }

    Start_experiment_response Experiment_service::start_experiment(const Start_experiment_request &parameters) {
        auto server = (Experiment_server *)_server;
        return server->start_experiment(parameters);
    }

    bool Experiment_service::start_episode(const Start_episode_request &parameters) {
        auto server = (Experiment_server *)_server;
        return server->start_episode(parameters);
    }

    bool Experiment_service::finish_episode() {
        auto server = (Experiment_server *)_server;
        return server->finish_episode();
    }

    bool Experiment_service::finish_experiment(const Finish_experiment_request &parameters) {
        auto server = (Experiment_server *)_server;
        return server->finish_experiment(parameters);
    }

    bool Experiment_service::capture(const Capture_request &parameters) {
        auto server = (Experiment_server *)_server;
        return server->capture(parameters);
    }

    bool Experiment_service::set_behavior(const Set_behavior_request &request) {
        auto server = (Experiment_server *)_server;
        return server->set_behavior(request);
    }

    bool Experiment_service::prey_enter_arena() {
        auto server = (Experiment_server *)_server;
        return server->prey_enter_arena();
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
        response.subject_name = experiment.subject_name;
        return response;
    }

    int Experiment_service::get_port() {
        string port_str(std::getenv("CELLWORLD_EXPERIMENT_SERVICE_PORT") ? std::getenv("CELLWORLD_EXPERIMENT_SERVICE_PORT") : "4540");
        return atoi(port_str.c_str());
    }

    void Experiment_service::set_logs_folder(const string &path) {
        logs_path = path;
        filesystem::create_directory(filesystem::path(logs_path));
    }

    void Experiment_server::set_tracking_client(Experiment_tracking_client &new_tracking_client) {
        new_tracking_client.experiment_server = this;
        tracking_client = &new_tracking_client;

    }

    void Experiment_tracking_client::on_step(const Step &step) {
        if (experiment_server->episode_in_progress){
            experiment_server->step_insertion_mtx.lock();
            experiment_server->active_episode.trajectories.push_back(step);
            experiment_server->step_insertion_mtx.unlock();
        }
    }

    bool Experiment_server::start_episode(const Start_episode_request &parameters) {
        if (episode_in_progress) return false;
        if (cell_world::file_exists(get_experiment_file(parameters.experiment_name))){
            active_experiment = parameters.experiment_name;
            active_episode = Episode();
            //active_episode.trajectories.reserve(50000);
            episode_in_progress = true;
            if (!clients.empty()) broadcast_subscribed(tcp_messages::Message("episode_started",active_experiment));
            for (auto &local_client:subscribed_local_clients) local_client->on_episode_started(active_experiment);
            return true;
        }
        return false;
    }

    bool Experiment_server::finish_episode() {
        if (!episode_in_progress) return false;
        if (!cell_world::file_exists(get_experiment_file(active_experiment))) return false;
        auto experiment = json_cpp::Json_from_file<Experiment>(get_experiment_file(active_experiment));
        active_episode.end_time = json_cpp::Json_date::now();
        experiment.episodes.push_back(active_episode);
        experiment.save(get_experiment_file(active_experiment));
        episode_in_progress = false;
        if (!clients.empty()) broadcast_subscribed(tcp_messages::Message("episode_finished",active_experiment));
        for (auto &local_client:subscribed_local_clients) local_client->on_episode_finished();
        return true;
    }

    bool Experiment_server::finish_experiment(const Finish_experiment_request &parameters) {
        if (!cell_world::file_exists(get_experiment_file(active_experiment))) return false;
        auto experiment = json_cpp::Json_from_file<Experiment>(get_experiment_file(parameters.experiment_name));
        auto end_time = experiment.start_time + chrono::minutes(experiment.duration);
        float remaining = ((float)(end_time - json_cpp::Json_date::now()).count()) / 1000;
        if (remaining>0){
            experiment.duration = (unsigned int) (((float)(json_cpp::Json_date::now() - experiment.start_time).count()) / 1000 / 60);
            experiment.save(get_experiment_file(parameters.experiment_name));
        }
        if (!clients.empty()) broadcast_subscribed(tcp_messages::Message("experiment_finished",parameters.experiment_name));
        for (auto &local_client:subscribed_local_clients) local_client->on_experiment_finished(parameters.experiment_name);
        return true;
    }

    bool Experiment_server::capture(const Capture_request &request) {
        if (episode_in_progress) {
            active_episode.captures.push_back(request.frame);
            if (!clients.empty()) broadcast_subscribed(Message("capture",request.frame));
            for (auto &local_client:subscribed_local_clients) local_client->on_capture(request.frame);
            return true;
        }
        return false;
    }

    bool Experiment_server::set_behavior(const Set_behavior_request &request) {
        if (!clients.empty()) broadcast_subscribed(Message("behavior_set", request.behavior));
        for (auto &local_client:subscribed_local_clients) local_client->on_behavior_set(request.behavior);
        return true;
    }

    bool Experiment_server::prey_enter_arena() {
        if (!clients.empty()) broadcast_subscribed(Message("prey_entered_arena"));
        for (auto &local_client:subscribed_local_clients) local_client->on_prey_entered_arena();
        return true;
    }


    Experiment_server::~Experiment_server() {
        for (auto local_client: local_clients){
            delete local_client;
        }
    }

    Start_experiment_response Experiment_server::start_experiment(const Start_experiment_request &parameters) {
        std::filesystem::create_directories(logs_path + '/' + parameters.prefix);
        Experiment new_experiment;
        new_experiment.world_configuration_name = parameters.world.world_configuration;
        new_experiment.world_implementation_name = parameters.world.world_implementation;
        new_experiment.occlusions = parameters.world.occlusions;
        new_experiment.duration = parameters.duration;
        new_experiment.subject_name = parameters.subject_name;
        new_experiment.start_time = json_cpp::Json_date::now();
        new_experiment.set_name(parameters.prefix, parameters.suffix);
        std::filesystem::create_directories(logs_path + '/' + parameters.prefix + "/" + new_experiment.name);
        new_experiment.save(get_experiment_file(new_experiment.name));
        Start_experiment_response response;
        response.experiment_name = new_experiment.name;
        response.start_date = new_experiment.start_time;
        response.subject_name = parameters.subject_name;
        response.world = parameters.world;
        response.duration = parameters.duration;
        if (!clients.empty()) broadcast_subscribed(tcp_messages::Message("experiment_started",response));
        for (auto &local_client:subscribed_local_clients) local_client->on_experiment_started(response);
        return response;
    }

    bool Experiment_service::human_intervention(const Human_intervention_request &request) {
        auto server = (Experiment_server *)_server;
        return server->prey_enter_arena();
    }

    bool Experiment_server::human_intervention(const Human_intervention_request &request) {
        if (!clients.empty()) broadcast_subscribed(tcp_messages::Message("human_intervention",request));
        for (auto &local_client:subscribed_local_clients) local_client->on_human_intervention(request.active);
        return false;
    }
}