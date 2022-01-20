from src import ExperimentService

def new_connection(conn):
    print("new connection!")

service = ExperimentService()
service.on_new_connection = new_connection
service.start()
print("service running..")
service.join()
