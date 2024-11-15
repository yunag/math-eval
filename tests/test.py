import math
import sys
from subprocess import PIPE, Popen

if len(sys.argv) != 2:
    print("Test binary path is not provided")
    exit(-1)

test_executable = sys.argv[1]

globals = {
    "sin": math.sin,
    "cos": math.cos,
    "tan": math.tan,
    "sqrt": math.sqrt,
    "pow": math.pow,
    "log": math.log,
    "exp": math.exp,
    "abs": abs,
    "pi": math.pi,
    "e": math.e,
    "a": 55,
    "b": 99,
    "c": 27,
    "x": 102,
    "y": 999,
    "z": 2,
    "w": 501,
}

p = Popen(
    [
        test_executable,
        str(globals["a"]),
        str(globals["b"]),
        str(globals["c"]),
        str(globals["x"]),
        str(globals["y"]),
        str(globals["z"]),
        str(globals["w"]),
    ],
    stdout=PIPE,
    stdin=PIPE,
    stderr=PIPE,
    text=True,
)

failed = False

with open("test_complete.txt") as f:
    for line in f:
        try:
            p.stdin.write(line)
            p.stdin.flush()
            stdout_data = p.stdout.readline().strip("\r\n")

            eval_data = eval(line.replace("^", "**"), globals)
            if not math.isclose(float(stdout_data), float(eval_data)):
                print(
                    f"[FAIL] {line.strip('\r\n')}:\n\tpython_eval({eval_data}), my_eval({stdout_data})"
                )
                failed = True

        except Exception as e:
            print(line, e)
            failed = True

if failed:
    exit(-1)
