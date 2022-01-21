#include <experiment/experiment_client.h>
#include <experiment/experiment_service.h>
#include <experiment/experiment_messages.h>
#include <cell_world.h>

using namespace cell_world;
using namespace tcp_messages;

namespace experiment {
    Start_experiment_response Experiment_client::start_experiment(const cell_world::World_info &world, const std::string &subject_name, int duration,
                                                   const std::string &prefix , const std::string &suffix) {
        auto parameters = Start_experiment_request();
        parameters.prefix = prefix;
        parameters.suffix = suffix;
        parameters.world = world;
        parameters.subject_name = subject_name;
        parameters.duration = duration;
        return send_request(tcp_messages::Message("start_experiment",parameters),0).get_body<Start_experiment_response>();
    }

    bool Experiment_client::start_episode(const std::string &experiment_name) {
        auto parameters = Start_episode_request();
        parameters.experiment_name = experiment_name;
        return send_request(tcp_messages::Message("start_episode",parameters),0).get_body<bool>();
    }

    bool Experiment_client::finish_episode() {
        return send_request(tcp_messages::Message("finish_episode"),0).get_body<bool>();
    }

    bool Experiment_client::finish_experiment(const std::string &experiment_name) {
        auto parameters = Finish_experiment_request();
        parameters.experiment_name = experiment_name;
        return send_request(tcp_messages::Message("finish_experiment",parameters),0).get_body<bool>();
    }

    bool Experiment_client::is_active(const std::string &experiment_name) {
        Get_experiment_response response = get_experiment(experiment_name);
        return response.remaining_time > 0;
    }

    Get_experiment_response Experiment_client::get_experiment(const std::string &experiment_name) {
        Get_experiment_request request;
        request.experiment_name = experiment_name;
        return send_request(tcp_messages::Message("get_experiment",request),0).get_body<Get_experiment_response>();
    }

    bool Experiment_client::connect(const std::string &ip) {
        auto port = Experiment_service::get_port();
        return tcp_messages::Message_client::connect(ip, port);
    }

    bool Experiment_client::set_tracking_service_ip(const std::string &ip) {
        return send_request(Message("set_tracking_service_ip", ip)).get_body<bool>();
    }

}