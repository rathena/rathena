# pip install pyyaml

import yaml

# Custom Dumper class to handle tab indentation
class MyDumper(yaml.Dumper):
    def increase_indent(self, flow=False, indentless=False):
        return super(MyDumper, self).increase_indent(flow, False)

# Modify representer to handle tab-based indentation
def represent_dict(dumper, data):
    return dumper.represent_mapping('tag:yaml.org,2002:map', data, flow_style=False)

yaml.add_representer(dict, represent_dict, Dumper=MyDumper)

# Function to check if a string should be converted to boolean
def str_to_bool(value):
    return value.lower() in ("yes", "true")

# Function to read the input database from the text file
def parse_db_file(input_file):
    data = []
    with open(input_file, 'r') as file:
        for line in file:
            line = line.strip()
            # Ignore comments or empty lines
            if line.startswith("//") or not line:
                continue

            # Split the line by comma
            values = line.split(',')
            entry = {
                "MobID": int(values[0]),
                "Name": values[1],
                "State": values[2].capitalize(),
                "SkillID": int(values[3]),
                "SkillLv": int(values[4]),
                "Rate": int(values[5]),
                "CastTime": int(values[6]),
                "Delay": int(values[7]),
                "Cancelable": str_to_bool(values[8]),
                "Target": values[9],
                "ConditionType": values[10],
            }

            # Optionally add remaining values, including ConditionValue, if they exist and are not empty
            optional_fields = ["ConditionValue", "val1", "val2", "val3", "val4", "val5", "Emotion", "Chat"]
            for i, field in enumerate(optional_fields, start=11):
                if i < len(values) and values[i].strip():
                    entry[field.capitalize()] = int(values[i]) if values[i].isdigit() else values[i]

            data.append(entry)

    return data

# Function to convert parsed data to YAML and save to output file with tab indentation
def save_to_yaml(data, output_file):
    output_data = {
        "Header": {
            "Type": "MOBSKILL_DB",
            "Version": 1
        },
        "Body": data
    }
    with open(output_file, 'w') as file:
        yaml.dump(output_data, file, Dumper=MyDumper, sort_keys=False)

# Main execution
input_file = 'mob_skill_db.txt'  # Input file name
output_file = 'mob_skill_db.yml'  # Output file name

parsed_data = parse_db_file(input_file)
save_to_yaml(parsed_data, output_file)

print(f"Conversion complete! Saved as {output_file}.")
