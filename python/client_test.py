from src import ExperimentClient
from cellworld import Timer


def on_experiment_start(parameters):
    print("started a new experiment", parameters)


def on_episode_start(episode_number):
    print("started a new episode", episode_number)


def on_episode_end(episode_number):
    print("ended a new episode", episode_number)


def on_experiment_end():
    print("ended a new experiment")


def print_step(step):
    print(step)


client = ExperimentClient()
client.on_experiment_start = on_experiment_start
client.on_episode_start = on_episode_start
client.on_episode_end = on_episode_end
client.on_experiment_end = on_experiment_end
client.connect()
client.subscribe()

client2 = ExperimentClient()
client2.connect()
print("start_experiment")
if not client2.start_experiment("test_experiment", "hexagonal", "mice", "10_05", "test_subject", 10):
    print("failed to start experiment")
    exit(1)
t = Timer(.5)
while t:
    pass
for i in range(15):
    print("start_episode")
    if not client2.start_episode():
        print("failed to start episode")
    t = Timer(.5)
    while t:
        pass
    print("finish_episode")
    if not client2.finish_episode():
        print("failed to finish episode")
    t = Timer(.5)
    while t:
        pass
print("finish_experiment")
if not client2.finish_experiment():
    print("failed to finish experiment")
