#pragma once
#include <json_cpp.h>
#include <cell_world.h>

namespace experiment {
    struct Start_experiment_request : json_cpp::Json_object{
        Json_object_members(
                Add_member(prefix);
                Add_member(suffix);
                Add_member(world);
                Add_member(subject_name);
                Add_member(duration);
                )
        std::string prefix;
        std::string suffix;
        cell_world::World_info world;
        std::string subject_name;
        int duration;
    };
    struct Start_episode_request : json_cpp::Json_object{
        Json_object_members(
                Add_member(experiment_name);
        )
        std::string experiment_name;
    };
    struct Finish_experiment_request : json_cpp::Json_object{
        Json_object_members(
                Add_member(experiment_name);
        )
        std::string experiment_name;
    };
}