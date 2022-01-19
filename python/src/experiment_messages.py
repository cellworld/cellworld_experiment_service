from json_cpp import JsonObject
from cellworld import *
from datetime import datetime
from uuid import uuid1

class StartExperimentMessage(JsonObject):
    def __init__(self, name: str = "", world: World_info = None, subject_name: str = "", duration: int = 0):
        self.name = name
        if world:
            self.world = world
        else:
            self.world = World_info()
        self.subject_name = subject_name
        self.duration = duration
