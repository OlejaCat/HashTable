import argparse
import re

def parse_args():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('input')
    parser.add_argument('output')
    return parser.parse_args()

args = parse_args()

with open(args.input, 'r') as f:
    words = re.findall(r'[A-Za-z]+', f.read())

with open(args.output, 'w') as f:
    f.write('\n'.join(words))
