import json
import os

if __name__ == "__main__":
    robot_dir = 'Robot'
    for file in os.scandir(robot_dir):
        if file.name.endswith(".json"):
            json_string = ''
            with open(robot_dir + '/' + file.name, 'r', encoding='utf-8') as f:
                json_string = f.read()
            res = json.loads(json_string)
            print(f'=== {file.name} ===')
            print(json.dumps(res, sort_keys=False, indent=4))
            print(f'End of {file.name}\n')
