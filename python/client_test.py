from src import ExperimentClient
from time import sleep


def on_experiment_started(parameters):
    print("started a new experiment", parameters)


def on_episode_started(episode_number):
    print("started a new episode", episode_number)


def on_episode_finished(episode_number):
    print("ended a new episode", episode_number)


def on_experiment_finished():
    print("ended a new experiment")


def print_step(step):
    print(step)


client = ExperimentClient()
client.on_experiment_started = on_experiment_started
client.on_episode_started = on_episode_started
client.on_episode_finished = on_episode_finished
client.on_experiment_finished = on_experiment_finished
client.connect()

client.subscribe()

client2 = ExperimentClient()
client2.connect()
print("start_experiment")
response = client2.start_experiment("PREFIX", "SUFFIX", "hexagonal", "mice", "10_05", "test_subject", 10)
experiment_name = response.experiment_name
sleep(1)
for i in range(15):
    print("start_episode")
    if not client2.start_episode(experiment_name):
        print("failed to start episode")
    sleep(5)
    print("finish_episode")
    if not client2.finish_episode():
        print("failed to finish episode")
    sleep(1)
print("finish_experiment")
if not client2.finish_experiment(experiment_name):
    print("failed to finish experiment")
