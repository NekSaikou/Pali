import argparse, subprocess, time, datetime

parser = argparse.ArgumentParser()
parser.add_argument("-e", "--engine", default="bin/Pali")
parser.add_argument("-f", "--fen", default="bench.txt")
parser.add_argument("-d", "--depth", default=10)
parser.add_argument("-m", "--message", default="none")
parser.add_argument("--log", default=None)
args = parser.parse_args()

bench_fens = open(args.fen).readlines()

engine = subprocess.Popen(
    args.engine,
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
    universal_newlines=True,
)

assert engine.stdin is not None
assert engine.stdout is not None

depth = int(args.depth)
assert depth > 0

time_start = time.time()

total_nodes = 0

print("-" * 89)
print("| {0:^7} | {1:^75} |".format("Nodes", "FEN"))
print("-" * 89)

for fen in bench_fens:
    nodes = 0
    engine.stdin.write("position fen %s\n" % fen)
    engine.stdin.write("go depth %d\n" % depth)
    engine.stdin.flush()

    correct_depth = False

    while correct_depth is not True:
        tokens = engine.stdout.readline().strip().split(" ")
        for i in range(len(tokens)):

            if tokens[i] == "depth":
                correct_depth = int(tokens[i + 1]) == depth

            if correct_depth and tokens[i] == "nodes":
                nodes = int(tokens[i + 1])

    print("| {0:>7} | {1:<75} |".format(nodes, fen.strip()))

    total_nodes += nodes

engine.kill()

time_taken = time.time() - time_start

time = datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S")
print("-" * 89 + "\n")

print(time)
print("depth:", depth)
print("nodes:", total_nodes)
print("time: ", round(time_taken, 2), "seconds")
print("nps:  ", round(total_nodes / time_taken, 2))
print("message:", args.message)

if args.log is not None:
    logfile = open(args.log, "a")

    logfile.write(time)
    logfile.write("\n")
    logfile.write("cmd:   {}\n".format(args.engine))
    logfile.write("depth: {}\n".format(depth))
    logfile.write("nodes: {}\n".format(total_nodes))
    logfile.write("time : {} seconds\n".format(round(time_taken, 2)))
    logfile.write("nps  : {}\n".format(round(total_nodes / time_taken, 2)))
    logfile.write("message: {}\n\n".format(args.message))
