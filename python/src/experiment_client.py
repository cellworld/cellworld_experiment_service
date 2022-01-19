from .experiment_messages import *
from tcp_messages import MessageClient, Message
from .experiment_service import ExperimentService
from cellworld import World_info


class ExperimentClient(MessageClient):
    def __init__(self):
        MessageClient.__init__(self)
        self.router.add_route("experiment_started", self.__process_experiment_started__, StartExperimentMessage)
        self.router.add_route("episode_started", self.__process_episode_started__, int)
        self.router.add_route("episode_finished", self.__process_episode_finished__, int)
        self.router.add_route("experiment_finished", self.__process_experiment_finished__)
        self.on_experiment_start = None
        self.on_experiment_end = None
        self.on_episode_start = None
        self.on_episode_end = None

    def subscribe(self):
        return self.send_request(Message("!subscribe"), 0).body == "success"

    def __process_experiment_started__(self, parameters: StartExperimentMessage):
        if self.on_experiment_start:
            self.on_experiment_start(parameters)

    def __process_episode_started__(self, episode_number: int):
        if self.on_episode_start:
            self.on_episode_start(episode_number)

    def __process_episode_finished__(self, episode_number: int):
        if self.on_episode_end:
            self.on_episode_end(episode_number)

    def __process_experiment_finished__(self):
        if self.on_experiment_end:
            self.on_experiment_end()

    def connect(self, ip: str = "127.0.0.1"):
        MessageClient.connect(self, ip, ExperimentService.port())

    def start_experiment(self, name: str, world_configuration: str, world_implementation: str, occlusions: str, subject_name: str, duration: int):
        parameters = StartExperimentMessage(name=name, world=World_info(world_configuration, world_implementation, occlusions), subject_name=subject_name, duration=duration)
        return self.send_request(Message("start_experiment", parameters), 0).body == "success"

    def start_episode(self):
        return self.send_request(Message("start_episode"), 0).body == "success"

    def finish_episode(self):
        return self.send_request(Message("finish_episode"), 0).body == "success"

    def finish_experiment(self):
        return self.send_request(Message("finish_experiment"), 0).body == "success"




