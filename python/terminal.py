from cellworld_experiment_service import ExperimentClient
import inspect
import types

client = ExperimentClient()
members = [m for m in inspect.getmembers(client) if m[0][0] != "_" and isinstance(m[1],types.MethodType)]
commands = {}
a = ""
for member in members:
    try:
        a = inspect.getfullargspec(member[1])
        #print(type(member[1]), member[0], member[1])
        member_name = member[0]
        commands[member_name] = {"method": member[1], "parameters": []}
        parameters = [p for p in a.args if p != "self"]
        for parameter in parameters:
            commands[member_name]["parameters"].append((parameter, a[6][parameter]))
            #print(parameter, a[6][parameter].__name__)
    except:
        pass

command = ""
while command != "end":
    command = input("Habitat: ")
    if command == "":
        continue
    if command not in commands:
        print("command not found.")
    else:
        parameters_values = []
        for parameter in commands[command]["parameters"]:
            while True:
                parameter_value = input("- " + parameter[0] + " (" + parameter[1].__name__ + ") : ")
                try:
                    parameters_values.append(parameter[1](parameter_value))
                    break

                except:
                    print("cannot convert " + parameter_value + " to " + parameter[1].__name__)
        print(commands[command]["method"](*parameters_values))
