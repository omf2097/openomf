import os
import pytest
import sys
import time
from contextlib import contextmanager
from pexpect import spawn, EOF


@contextmanager
def start_openomf():
    lsan_env = os.environ
    lsan_env["LSAN_OPTIONS"] = "suppressions=../lsan.supp"
    openomf_bin = os.environ["OPENOMF_BIN"]
    build_dir = os.environ["BUILD_DIR"]
    orig_dir = os.getcwd()
    os.chdir(build_dir)
    p = spawn(openomf_bin, args=["--force-renderer", "NULL",
              "--force-audio-backend", "NULL"], logfile=sys.stdout,
              encoding='utf-8', env=lsan_env)
    try:
        yield p
    except Exception as e:
        pytest.fail(f"Caught exception {e}")
    finally:
        p.close()
        os.chdir(orig_dir)
        assert p.exitstatus == 0, f"openomf exit status was {p.exitstatus}"


def test_tournament_menu():
    with start_openomf() as p:
        p.expect("Starting OpenOMF", timeout=180)
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
        p.expect("Starting OpenOMF", timeout=180)
        time.sleep(1)
        p.sendline("quit")
        p.expect(EOF, timeout=60)
