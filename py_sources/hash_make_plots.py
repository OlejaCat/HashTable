import matplotlib.pyplot as plt
import argparse
import os

def read_datafile(path):
    with open(path, 'r') as f:
        header = f.readline().strip().split()
        count = int(header[0])
        variance = float(header[1])
        ns_time = int(header[2])
        
        title = f.readline().strip()
        
        numbers = []
        while len(numbers) < 4096:
            line = f.readline()
            numbers.extend(map(int, line.strip().split()))
        
        return {
            'title': title,
            'variance': variance,
            'ms_time': ns_time / 1_000_000,
            'points': numbers[:4096]
        }

def create_plots(files):
    for filepath in files:
        data = read_datafile(filepath)

        plt.figure(figsize=(12, 4))
        plt.bar(range(4096), data['points'], width=1)
        plt.title(f"{data['title']}\nVariance: {data['variance']:.3f}, Time: {data['ms_time']:.3f} ms")
        plt.xlabel("Index")
        plt.ylabel("Collisions")
        
        filename = os.path.splitext(os.path.basename(filepath))[0]
        plt.savefig(f"plots/{filename}.png", bbox_inches='tight')
        plt.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate collision histograms')
    parser.add_argument('files', nargs='+', help='Input data files')
    args = parser.parse_args()
    
    create_plots(args.files)

