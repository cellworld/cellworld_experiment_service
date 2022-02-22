#pragma once
#include<tcp_messages.h>
#include<experiment/experiment_messages.h>
#include<agent_tracking/tracking_client.h>
#include<cell_world.h>

namespace experiment{
    struct Experiment_tracking_client : agent_tracking::Tracking_client {
        void on_step(const cell_world::Step &) override;
    };

    struct Experiment_service : tcp_messages::Message_service {
        Routes(
                Add_route_with_response("set_tracking_service_ip", set_tracking_service_ip, std::string);
                Add_route_with_response("start_experiment", start_experiment, Start_experiment_request);
                Add_route_with_response("start_episode", start_episode, std::string);
                Add_route_with_response("finish_episode", finish_episode);
                Add_route_with_response("finish_experiment", finish_experiment, Finish_experiment_request);
                Add_route_with_response("get_experiment", get_experiment, Get_experiment_request);
                Add_route_with_response("capture", capture, Capture_request);
                Add_route_with_response("set_behavior", set_behavior, Set_behavior_request);
                Allow_subscription();
                )
        Start_experiment_response start_experiment(const Start_experiment_request &);
        bool start_episode(const std::string &);
        bool finish_episode();
        bool finish_experiment(const Finish_experiment_request &);
        static Get_experiment_response get_experiment(const Get_experiment_request &);
        static bool set_tracking_service_ip(const std::string &ip);
        static void set_logs_folder(const std::string &path);
        static int get_port();
        bool capture(const Capture_request &);
        bool set_behavior(const Set_behavior_request &);
    };

    using Experiment_server = tcp_messages::Message_server<Experiment_service>;
}