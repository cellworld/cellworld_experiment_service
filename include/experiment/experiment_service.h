#pragma once
#include<tcp_messages.h>
#include<experiment/experiment_messages.h>
#include<agent_tracking/tracking_client.h>
#include<experiment/experiment_client.h>
#include<cell_world.h>

namespace experiment{
    struct Experiment_tracking_client : agent_tracking::Tracking_client {
        void on_step(const cell_world::Step &) override;
        Experiment_server *experiment_server;
    };

    struct Experiment_service : tcp_messages::Message_service {
        Routes(
                Add_route_with_response("start_experiment", start_experiment, Start_experiment_request);
                Add_route_with_response("start_episode", start_episode, Start_episode_request);
                Add_route_with_response("finish_episode", finish_episode);
                Add_route_with_response("finish_experiment", finish_experiment, Finish_experiment_request);
                Add_route_with_response("get_experiment", get_experiment, Get_experiment_request);
                Add_route_with_response("capture", capture, Capture_request);
                Add_route_with_response("set_behavior", set_behavior, Set_behavior_request);
                Allow_subscription();
                )

        Start_experiment_response start_experiment(const Start_experiment_request &);
        bool start_episode(const Start_episode_request &);
        bool finish_episode();
        bool finish_experiment(const Finish_experiment_request &);
        static Get_experiment_response get_experiment(const Get_experiment_request &);

        static void set_logs_folder(const std::string &path);
        static int get_port();
        bool capture(const Capture_request &);
        bool set_behavior(const Set_behavior_request &);
    };

    struct Experiment_server : tcp_messages::Message_server<Experiment_service> {
        ~Experiment_server();
        bool start_episode(const Start_episode_request &);
        bool finish_episode();
        bool finish_experiment(const Finish_experiment_request &);
        bool capture(const Capture_request &);
        bool set_behavior(const Set_behavior_request &);
        void set_tracking_client(Experiment_tracking_client &);

        template<class T>
        T &create_local_client(){
            static_assert(std::is_base_of<Experiment_client, T>::value, "T must inherit from Key");
            auto new_local_client = new T();
            local_clients.push_back((Experiment_client *) new_local_client);
            new_local_client->local_server = this;
            return *new_local_client;
        }
        std::vector<Experiment_client *> local_clients;
        std::vector<Experiment_client *> subscribed_local_clients;

        std::string active_experiment = "";
        cell_world::Episode active_episode;
        bool episode_in_progress = false;
        Experiment_tracking_client *tracking_client = nullptr;
        std::string tracking_service_ip = "";
    };
}