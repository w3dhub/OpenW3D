#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 OpenW3D Contributors.
"""Optional integration regression; requires an existing LAN server asset setup."""
import argparse
import pathlib
import shutil
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("server", type=pathlib.Path)
    parser.add_argument("gamedir", type=pathlib.Path, help="Use a disposable copy of the game assets")
    parser.add_argument("ini", type=pathlib.Path, help="Engine settings; copied before launching")
    parser.add_argument("settings", type=pathlib.Path, help="LAN server startup settings")
    args = parser.parse_args()
    marker = b"Interactive profiling display is unavailable"
    with tempfile.TemporaryDirectory(prefix="openw3d-console-") as directory:
        directory = pathlib.Path(directory)
        ini = directory / "test.ini"
        shutil.copyfile(args.ini, ini)
        output = directory / "stdout.log"
        errors = directory / "stderr.log"
        with output.open("wb") as out, errors.open("wb") as err:
            process = subprocess.Popen(
                [str(args.server.resolve()), "--multi", "--ini", str(ini),
                 "--gamedir", str(args.gamedir.resolve()), "--startserver", str(args.settings.resolve())],
                cwd=args.gamedir.resolve(), stdin=subprocess.PIPE, stdout=out, stderr=err)
            try:
                def wait_for(text):
                    deadline = time.monotonic() + 30
                    while text not in output.read_bytes():
                        if process.poll() is not None or time.monotonic() >= deadline:
                            raise AssertionError(("Server did not become ready", process.poll(),
                                                  output.read_bytes()[-4000:], errors.read_bytes()[-2000:]))
                        time.sleep(0.05)

                wait_for(b"Level loaded OK")
                # First prove this command is available; otherwise its absence
                # after quit would not tell us anything about command dispatch.
                process.stdin.write(b"profile\n")
                process.stdin.flush()
                wait_for(marker)
                process.stdin.write(b"quit\nprofile\n")
                process.stdin.flush()
                process.stdin.close()
                process.wait(timeout=30)
                text = output.read_bytes()
                assert b"Terminating game on demand" in text, text[-4000:]
                assert text.count(marker) == 1, "Command after quit was executed"
                assert "AutoRestartFlag=0" in ini.read_text(), "Automatic restart was not disabled"
                # Engine teardown has a separately reproduced, pre-existing
                # crash. This regression checks command dispatch, not teardown.
                print(f"Command-batch checks passed; server exit code: {process.returncode:#x}")
            finally:
                if process.poll() is None:
                    process.kill()
                    process.wait(timeout=5)
                process.stdin.close()


if __name__ == "__main__":
    main()
