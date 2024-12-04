import sys
import os
import time
from pexpect import popen_spawn, EOF
from contextlib import contextmanager


@contextmanager
def start_openomf():
    lsan_env = os.environ
    lsan_env["LSAN_OPTIONS"] = "suppressions=../lsan.supp"
    openomf_bin = os.environ["OPENOMF_BIN"]
    build_dir = os.environ["BUILD_DIR"]
    p = popen_spawn.PopenSpawn([openomf_bin, "--force-renderer", "NULL",
                                "--force-audio-backend", "NULL"],
                                timeout=60, cwd=build_dir, encoding='utf-8',
                                logfile=sys.stdout, env=lsan_env)
    try:
        yield p
    finally:
        p.wait()


def test_tournament_menu():
    with start_openomf() as p:
        time.sleep(1)
        p.sendline("scene SCENE_MENU")
        p.expect("Loaded scene SCENE_MENU", timeout=180)
        time.sleep(1)
        p.sendline("scene SCENE_MECHLAB")
        p.expect("Loaded scene SCENE_MECHLAB", timeout=180)
        time.sleep(1)
        p.sendline("quit")
        p.expect(EOF, timeout=180)


def test_quit():
    with start_openomf() as p:
        time.sleep(1)
        p.sendline("quit")
        p.expect(EOF, timeout=60)
