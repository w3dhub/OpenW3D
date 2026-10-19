#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 OpenW3D Contributors.
"""Exercise OS input readiness, stream ownership, and real interrupt delivery."""
import os
import pathlib
import queue
import signal
import subprocess
import sys
import tempfile
import threading
import time
import unittest

HARNESS = os.path.abspath(sys.argv.pop(1))
WINDOWS_TEST = os.path.abspath(sys.argv.pop(1)) if os.name == "nt" else None


class Process:
    def __init__(self, *args, stdin=subprocess.PIPE):
        self.process = subprocess.Popen([HARNESS, *args], stdin=stdin,
                                        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.lines = []
        self.queue = queue.Queue()
        self.reader = threading.Thread(target=self.read, daemon=True)
        self.reader.start()

    def read(self):
        for line in self.process.stdout:
            line = line.replace(b"\r\n", b"\n")
            self.lines.append(line)
            self.queue.put(line)

    def wait_line(self, expected, timeout=5):
        deadline = time.monotonic() + timeout
        while True:
            line = self.queue.get(timeout=max(0, deadline - time.monotonic()))
            if line == expected:
                return

    def finish(self):
        self.process.wait(timeout=10)
        self.reader.join(timeout=2)
        stderr = self.process.stderr.read().replace(b"\r\n", b"\n")
        assert self.process.returncode == 0, (self.process.returncode, self.lines, stderr)
        assert self.lines[-1] == b"CLEANUP\n", self.lines
        assert b"STDERR caf\xc3\xa9\n" in stderr, stderr
        assert b"STDERR CLEANUP\n" in stderr, stderr
        assert not any(b"\x1b" in line or b"\b" in line or b">" in line for line in self.lines)
        return stderr

    def close(self):
        if self.process.poll() is None:
            self.process.kill()
        self.process.wait(timeout=5)
        self.reader.join(timeout=2)
        for stream in (self.process.stdin, self.process.stdout, self.process.stderr):
            if stream:
                stream.close()


class ConsoleTests(unittest.TestCase):
    def launch(self, *args, **kwargs):
        proc = Process(*args, **kwargs)
        self.addCleanup(proc.close)
        proc.wait_line(b"READY\n")
        return proc

    def test_fragmented_input_and_eof(self):
        proc = self.launch()
        proc.process.stdin.write(b"game_")
        proc.process.stdin.flush()
        for _ in range(5):
            proc.wait_line(b"TICK\n")
        self.assertFalse(any(line.startswith(b"COMMAND ") for line in proc.lines))
        proc.process.stdin.write(b"info\r")
        proc.process.stdin.flush()
        proc.wait_line(b"COMMAND game_info\n")
        proc.process.stdin.write(b"\nhelp\n\n \t\nmsg caf\xc3")
        proc.process.stdin.flush()
        proc.wait_line(b"COMMAND help\n")
        proc.process.stdin.write(b"\xa9\nfinal")
        proc.process.stdin.close()
        proc.wait_line(b"EOF\n")
        for _ in range(5):
            proc.wait_line(b"TICK\n")
        stderr = proc.finish()
        self.assertEqual([line for line in proc.lines if line.startswith(b"COMMAND ")],
                         [b"COMMAND game_info\n", b"COMMAND help\n", b"COMMAND msg caf\xc3\xa9\n", b"COMMAND final\n"])
        self.assertNotIn(b"Ignoring", stderr)
        self.assertEqual(proc.lines.count(b"EOF\n"), 1)

    def test_oversized_lines_and_bounded_queue(self):
        proc = self.launch()
        proc.process.stdin.write(b"x" * 200 + b"\n" + b"x" * 201 + b"quit\r\n" + b"ok\n" * 90 + b"z" * 201)
        proc.process.stdin.close()
        stderr = proc.finish()
        commands = [line for line in proc.lines if line.startswith(b"COMMAND ")]
        self.assertEqual(commands, [b"COMMAND " + b"x" * 200 + b"\n"] + [b"COMMAND ok\n"] * 90)
        self.assertIn(b"Ignoring console command", stderr)

    def test_missing_stdin(self):
        proc = self.launch("--missing-stdin", "20")
        proc.finish()
        self.assertEqual(proc.lines.count(b"TICK\n"), 20)
        self.assertEqual(proc.lines.count(b"EOF\n"), 1)

    def test_null_stdin(self):
        proc = self.launch("20", stdin=subprocess.DEVNULL)
        proc.finish()
        self.assertEqual(proc.lines.count(b"TICK\n"), 20)

    def test_file_input_and_output(self):
        with tempfile.TemporaryFile() as source, tempfile.TemporaryFile() as output, tempfile.TemporaryFile() as errors:
            source.write(b"help\r\ngame_info\nlast")
            source.seek(0)
            subprocess.run([HARNESS, "20"], stdin=source, stdout=output, stderr=errors, check=True, timeout=5)
            output.seek(0)
            lines = output.read().splitlines()
            self.assertEqual([line for line in lines if line.startswith(b"COMMAND ")],
                             [b"COMMAND help", b"COMMAND game_info", b"COMMAND last"])
            self.assertEqual(lines.count(b"EOF"), 1)
            self.assertEqual(lines.count(b"TICK"), 20)
            self.assertEqual(lines[-1], b"CLEANUP")
            errors.seek(0)
            self.assertIn(b"STDERR CLEANUP", errors.read())

    @unittest.skipIf(os.name == "nt", "POSIX signals")
    def test_signals(self):
        for event in (signal.SIGINT, signal.SIGTERM):
            with self.subTest(event=event):
                proc = self.launch("500")
                os.kill(proc.process.pid, event)
                proc.wait_line(b"STOP\n")
                os.kill(proc.process.pid, event)
                proc.finish()
                self.assertEqual(proc.lines.count(b"STOP\n"), 1)

    @unittest.skipUnless(os.name == "nt", "Windows console events")
    def test_isolated_windows_console(self):
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = subprocess.SW_HIDE
        for args in ([], ["private"]):
            with self.subTest(args=args):
                result = subprocess.run([WINDOWS_TEST, *args], stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                        creationflags=subprocess.CREATE_NEW_CONSOLE,
                                        startupinfo=startup, timeout=10)
                self.assertEqual(result.returncode, 0, (result.stdout, result.stderr))
                # Even terminal input must not echo into a redirected stdout.
                self.assertEqual(result.stdout, b"WINDOWS CONSOLE OK\n")
        # Also exercise untouched CRT terminal descriptors, not just redirected
        # handles. The result goes to a file so Popen need not override stdio.
        with tempfile.TemporaryDirectory() as directory:
            report = pathlib.Path(directory) / "console-report.txt"
            result = subprocess.run([WINDOWS_TEST, "private", str(report)],
                                    creationflags=subprocess.CREATE_NEW_CONSOLE,
                                    startupinfo=startup, timeout=10)
            self.assertEqual(result.returncode, 0, report.read_bytes())
            self.assertEqual(report.read_bytes(), b"WINDOWS CONSOLE OK\n")


if __name__ == "__main__":
    unittest.main()
