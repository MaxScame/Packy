import json

json_string = ''
with open('.~3d', 'r', encoding='utf-8') as f:
    json_string = f.read()

res = json.loads(json_string)
print(json.dumps(res, sort_keys=False, indent=4))
