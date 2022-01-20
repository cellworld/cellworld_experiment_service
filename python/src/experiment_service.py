import os
from .experiment_messages import *
from cellworld import *
from tcp_messages import MessageServer, Message
from cellworld_tracking import TrackingClient


class ExperimentService(MessageServer):
    def __init__(self, tracker_ip: str = "127.0.0.1"):
        MessageServer.__init__(self)
        self.tracking_service = None
        self.router.add_route("start_experiment", self.start_experiment, StartExperimentRequest)
        self.router.add_route("start_episode", self.start_episode, StartEpisodeRequest)
        self.router.add_route("finish_episode", self.finish_episode)
        self.router.add_route("finish_experiment", self.finish_experiment, FinishExperimentRequest)
        self.router.add_route("get_experiment", self.get_experiment, GetExperimentRequest)

        self.current_experiment = None
        self.experiment_timer = Timer()
        self.allow_subscription = True
        self.current_episode = None
        self.tracker = TrackingClient()
        self.tracker.connect(tracker_ip)

    @staticmethod
    def get_experiment_file(experiment_name: str):
        return "logs/" + experiment_name + ".json"

    def start(self):
        MessageServer.start(self, self.port())

    def __process_step__(self, step):
        print(step)
        if self.current_episode:
            self.current_episode.trajectories.append(step)

    def start_experiment(self, parameters: StartExperimentRequest) -> bool:
        if self.current_experiment:
            return False
        new_experiment = Experiment(name=parameters.name,
                                    subject_name=parameters.subject_name,
                                    world_configuration_name=parameters.world.world_configuration,
                                    world_implementation_name=parameters.world.world_implementation,
                                    occlusions=parameters.world.occlusions,
                                    duration=parameters.duration)

        str(new_experiment)

        self.current_episode = None
        self.experiment_timer.reset()
        self.broadcast_subscribed(Message("experiment_started", parameters))
        return True

    def start_episode(self, m) -> bool:
        self.tracker.register_consumer(self.__process_step__)
        if self.current_episode:
            return False
        self.current_episode = Episode()
        self.broadcast_subscribed(Message("episode_started", len(self.current_experiment.episodes)))
        return True

    def finish_episode(self, m) -> bool:
        self.tracker.unregister_consumer()
        if not self.current_episode:
            return False
        self.current_experiment.episodes.append(self.current_episode)
        self.current_episode = None
        self.broadcast_subscribed(Message("episode_finished", len(self.current_experiment.episodes) - 1))
        return True

    def finish_experiment(self, m) -> bool:
        if not self.current_experiment:
            return False
        self.broadcast_subscribed(Message("experiment_finished", self.current_experiment.name))
        self.current_experiment = None
        return True

    @staticmethod
    def port() -> int:
        default_port = 4540
        if os.environ.get("CELLWORLD_EXPERIMENT_SERVICE_PORT"):
            try:
                return int(os.environ.get("CELLWORLD_EXPERIMENT_SERVICE_PORT"))
            finally:
                pass
        return default_port
