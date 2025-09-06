#!/usr/bin/env python3
import sys
import argparse
import re
import xml.etree.ElementTree as ET
from pathlib import Path
import pandas as pd
import json


def convert_results_to_data(path_str):
    path = Path(path_str or "")
    if not path.exists() or path.is_dir():
        return None
    
    with open(path, 'r', encoding='utf-8') as file:
        content = file.read()
    
    all_data = []
    for block in re.findall(r'<Results[^>]*>.*?</Results>', content, re.DOTALL):
        root = ET.fromstring(block)
        viewset_name = root.get('Name')
        all_data.append({
            'Viewset': viewset_name,
            'Subtest': 'Composite', 
            'FPS': float(root.find('.//Composite').get('Score'))
        })
        for test in root.findall('.//Test'):
            all_data.append({
                'Viewset': viewset_name,
                'Subtest': test.get('Index'),
                'FPS': float(test.get('FPS', '0'))
            })
    
    return pd.DataFrame(all_data) if all_data else None


def main():
    if len(sys.argv) > 1:
        parser = argparse.ArgumentParser()
        parser.add_argument("input1", type=str, default="")
        parser.add_argument("input2", type=str, default="")
        args = parser.parse_args()
        input1 = args.input1 
        input2 = args.input2 
    else:
        recent_file = Path("/tmp/viewperf_recent.json")
        recent = json.load(open(recent_file)) if recent_file.exists() else ["", ""]
        input1 = input(f"The 1st input path (required) [{recent[0]}]: ").strip() or recent[0]
        input2 = input(f"The 2nd input path for comparison [{recent[1]}]: ").strip() or recent[1]
        json.dump([input1, input2], open(recent_file, "w"))

    input1_data = convert_results_to_data(input1)
    input2_data = convert_results_to_data(input2)
    if input2_data is None:
        print(f"\nFIRST RESULT: {Path(input1).name}")
        print(input1_data.to_string(index=False))
    else:
        merged = input1_data.merge(input2_data, on=['Viewset', 'Subtest'], suffixes=('_old', '_new'))
        merged['FPS_ratio_%'] = (merged['FPS_new'] / merged['FPS_old'] * 100).round(1).astype(str) + '%'
        comparison = merged[['Viewset', 'Subtest', 'FPS_old', 'FPS_new', 'FPS_ratio_%']].copy()
        print(f"\nCOMPARISON: {Path(input2).name}(new) vs {Path(input1).name}(old)")
        print(comparison.to_string(index=False)) 

if __name__ == '__main__':
    main()
