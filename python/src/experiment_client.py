from .experiment_messages import *
from tcp_messages import MessageClient, Message
from .experiment_service import ExperimentService
from cellworld import World_info


class ExperimentClient(MessageClient):
    def __init__(self):
        MessageClient.__init__(self)
        self.router.add_route("experiment_started", self.__process_experiment_started__, StartExperimentResponse)
        self.router.add_route("episode_started", self.__process_episode_started__, str)
        self.router.add_route("episode_finished", self.__process_episode_finished__, str)
        self.router.add_route("experiment_finished", self.__process_experiment_finished__, str)
        self.router.add_route("capture", self.__process_capture__, int)
        self.router.add_route("behavior_set", self.__process_behavior_set__, int)
        self.on_experiment_started = None
        self.on_experiment_finished = None
        self.on_episode_started = None
        self.on_episode_finished = None
        self.on_capture = None
        self.on_behavior_set = None

    def subscribe(self):
        return self.send_request(Message("!subscribe"), 0).body == "success"

    def __process_behavior_set__(self, behavior: int):
        if self.on_behavior_set:
            self.on_behavior_set(behavior)

    def __process_capture__(self, frame: int):
        if self.on_capture:
            self.on_capture(frame)

    def __process_experiment_started__(self, parameters: StartExperimentResponse):
        if self.on_experiment_started:
            self.on_experiment_started(parameters)

    def __process_episode_started__(self, experiment_name: str):
        if self.on_episode_started:
            self.on_episode_started(experiment_name)

    def __process_episode_finished__(self, experiment_name: str):
        if self.on_episode_finished:
            self.on_episode_finished(experiment_name)

    def __process_experiment_finished__(self, experiment_name: str):
        if self.on_experiment_finished:
            self.on_experiment_finished(experiment_name)

    def set_behavior(self, behavior: int):
        return self.send_request(Message("set_behavior", SetBehaviorRequest(behavior=behavior))).get_body(bool)

    def connect(self, ip: str = "127.0.0.1"):
        return MessageClient.connect(self, ip, ExperimentService.port())

    def capture(self, frame: int) -> bool:
        return self.send_request(Message("capture", CaptureRequest(frame=frame))).get_body(bool)

    def start_experiment(self, prefix: str, suffix: str, world_configuration: str, world_implementation: str, occlusions: str, subject_name: str, duration: int) -> StartExperimentResponse:
        parameters = StartExperimentRequest(prefix=prefix, suffix=suffix, world=World_info(world_configuration, world_implementation, occlusions), subject_name=subject_name, duration=duration)
        return self.send_request(Message("start_experiment", parameters), 5000).get_body(StartExperimentResponse)

    def start_episode(self, experiment_name: str) -> str:
        return self.send_request(Message("start_episode", StartEpisodeRequest(experiment_name=experiment_name)), 5000).get_body(bool)

    def finish_episode(self) -> str:
        return self.send_request(Message("finish_episode"), 5000).get_body(bool)

    def set_tracking_service_ip(self, ip) -> str:
        return self.send_request(Message("set_tracking_service_ip", ip), 5000).get_body(bool)

    def finish_experiment(self, experiment_name: str):
        return self.send_request(Message("finish_experiment", FinishExperimentRequest(experiment_name=experiment_name)), 5000).get_body(bool)

    def get_experiment(self, experiment_name: str) -> GetExperimentResponse:
        return self.send_request(Message("get_experiment", GetExperimentRequest(experiment_name=experiment_name)), 5000).get_body(GetExperimentResponse)

    def is_active(self, experiment_name: str) -> bool:
        return self.get_experiment(experiment_name).remaining_time > 0

