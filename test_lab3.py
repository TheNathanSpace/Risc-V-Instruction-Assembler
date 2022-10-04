import re
from subprocess import Popen, PIPE, STDOUT
import csv


def read_csv(filepath: str) -> list:
    with open(filepath, encoding = 'utf-8') as f:
        r = csv.DictReader(f, delimiter = '|')
        return list(r)


failed = 0
for line in read_csv("lab3_unit_tests.csv"):
    instruction: str = line["instruction"]
    expected_result: str = line["binary encoding"]

    x = Popen(['Lab3.exe'], stdout = PIPE, stdin = PIPE, stderr = None)
    if x.poll() is None:
        input_bytes = str.encode(f"{instruction}\n")
        print(f"Testing {instruction}")
        output = str(x.communicate(input = input_bytes)[0])
        matched = re.match("([\D ]*)([0-1]*)(.*)", output)
        if matched:
            test_result = matched.group(2)
            if test_result != expected_result:
                print(f"{instruction}: Expected / Actual: {expected_result} / {test_result}")
                failed += 1

    x.stdout.close()
    x.stdin.close()
    x.kill()

print("\nAll testing completed.\nFailed: " + str(failed))
