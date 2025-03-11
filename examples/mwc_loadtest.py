#!/usr/bin/python3

import os
import subprocess
import sys
import tempfile


def mwc_loadtest():
    if len(sys.argv) != 3:
        print("Usage: ./mwc_loadtest.py /path/to/mwc /path/to/normal_wc")
        return -1
    with tempfile.NamedTemporaryFile() as fp:
        fp.write(os.urandom(4000000000))  # 1 GB
        mwc_but = sys.argv[1]
        baseline_wc_but = sys.argv[2]
        print(f"mwc_loadtest: testing mwc at {mwc_but} against {baseline_wc_but}")
        mwc_test = ["perf", "stat", mwc_but, fp.name]
        mwc_run = subprocess.run(mwc_test, capture_output=True)
        baseline_wc_test = ["perf", "stat", baseline_wc_but, fp.name]
        baseline_wc_run = subprocess.run(baseline_wc_test, capture_output=True)
        print(f"mwc result: {mwc_run.stdout.decode()}")
        print(f"mwc perf: {mwc_run.stderr.decode()}")
        print(f"baseline_wc result: {baseline_wc_run.stdout.decode()}")
        print(f"baseline_wc perf: {baseline_wc_run.stderr.decode()}")
        fp.close()

if __name__ == "__main__":
    mwc_loadtest()
