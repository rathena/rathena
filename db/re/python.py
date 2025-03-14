import yaml
import re

def load_lua_ids(lua_file):
    """Extract item IDs from iteminfo_EN.lua."""
    with open(lua_file, 'r', encoding='utf-8') as file:
        content = file.read()
    
    return set(re.findall(r'\[(\d+)\] = {', content))

def extract_costume_ids(input_file, lua_file, output_file):
    with open(input_file, 'r', encoding='utf-8') as file:
        data = yaml.safe_load(file)
    
    valid_ids = load_lua_ids(lua_file)
    costume_ids = []
    
    if 'Body' in data:
        for item in data['Body']:
            if 'Id' in item and str(item['Id']) in valid_ids:
                if ('AegisName' in item and 'costume' in item['AegisName'].lower()) or \
                   ('Name' in item and 'costume' in item['Name'].lower()):
                    costume_ids.append(f"{item['Id']}:500000,")
    
    with open(output_file, 'w', encoding='utf-8') as file:
        for i in range(0, len(costume_ids), 500):
            file.write(''.join(costume_ids[i:i+500]) + '\n')

# Example usage
extract_costume_ids('item_db_equip.yml', 'iteminfo_EN1.lua', 'costume_ids.txt')