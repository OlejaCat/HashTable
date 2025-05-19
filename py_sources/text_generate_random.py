import argparse
import re
import random

parser = argparse.ArgumentParser()
parser.add_argument('input_file')
parser.add_argument('output_file')
parser.add_argument('count', type=int)
args = parser.parse_args()

with open(args.input_file) as f:
    words = re.findall(r'\b[a-zA-Z]+\b', f.read())

selected = random.choices(words, k=args.count)

with open(args.output_file, 'w') as f:
    f.write('\n'.join(selected))
