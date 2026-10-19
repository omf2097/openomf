"""Helpers for generating and replaying deterministic REC files."""

import os
import subprocess
from pathlib import Path


def get_binaries():
    """Return the configured build directory, game binary, and REC generator."""
    build_dir = Path(os.environ.get("BUILD_DIR", "build")).resolve()
    openomf_bin = Path(os.environ.get("OPENOMF_BIN", str(build_dir / "openomf")))
    if not openomf_bin.is_absolute():
        # run_pytest.sh exports OPENOMF_BIN relative to the build directory;
        # resolve it against BUILD_DIR rather than the pytest working directory.
        openomf_bin = build_dir / openomf_bin
    openomf_bin = openomf_bin.resolve()
    generator = build_dir / "gen_move_test"
    return build_dir, openomf_bin, generator


def generate_rec(generator, har_id, opponent_id, output_file, *, actions=(), seed=None,
                assertions=(), ticks=None):
    """Generate a REC file using the project's deterministic REC tool."""
    command = [str(generator), str(har_id), str(opponent_id), str(output_file)]
    if seed is not None:
        command.extend(("--seed", str(seed)))
    if ticks is not None:
        command.extend(("--ticks", str(ticks)))
    for assertion in assertions:
        command.extend(("--assert", assertion))
    command.extend(str(action) for action in actions)
    return subprocess.run(command, capture_output=True, text=True, timeout=20)


def run_rec(openomf_bin, build_dir, rec_file, *, timeout=90):
    """Replay a REC file headlessly; a nonzero result indicates failure."""
    environment = os.environ.copy()
    environment["OPENOMF_RESOURCE_PATH"] = str(Path(build_dir).resolve())
    return subprocess.run(
        [
            str(openomf_bin),
            "--force-audio-backend=NULL",
            "--force-renderer=NULL",
            "--speed=10",
            "-P",
            str(rec_file),
        ],
        capture_output=True,
        text=True,
        timeout=timeout,
        cwd=str(build_dir),
        env=environment,
    )
