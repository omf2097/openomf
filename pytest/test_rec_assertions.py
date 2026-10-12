"""Headless deterministic REC smoke tests."""

import pytest

from rec_harness import generate_rec, get_binaries, run_rec


build_dir, openomf_bin, generator = get_binaries()
requires_tools = pytest.mark.skipif(
    not generator.is_file(),
    reason="gen_move_test is not built; configure with -DUSE_TOOLS=On",
)


def make_rec(tmp_path, name, **kwargs):
    rec_file = tmp_path / name
    generated = generate_rec(generator, 0, 1, rec_file, **kwargs)
    assert generated.returncode == 0, generated.stderr
    return rec_file


@requires_tools
def test_true_assertion_passes(tmp_path):
    rec_file = make_rec(tmp_path, "true.rec", assertions=("har1.health>0@200",))
    result = run_rec(openomf_bin, build_dir, rec_file)
    assert result.returncode == 0, result.stdout + result.stderr


@requires_tools
def test_false_assertion_fails(tmp_path):
    rec_file = make_rec(tmp_path, "false.rec", assertions=("har1.health==99999@200",))
    result = run_rec(openomf_bin, build_dir, rec_file)
    assert result.returncode != 0


@requires_tools
def test_seeded_record_is_accepted(tmp_path):
    rec_file = make_rec(
        tmp_path,
        "seeded.rec",
        seed=12345,
        assertions=("har1.health>0@200",),
    )
    result = run_rec(openomf_bin, build_dir, rec_file)
    assert result.returncode == 0, result.stdout + result.stderr


@requires_tools
def test_current_ai_controller_replay_is_deterministic(tmp_path):
    """Run the existing AI controller behind a fixed-seed REC replay."""
    rec_file = make_rec(
        tmp_path,
        "current-ai.rec",
        actions=("40", "40", "44", "01"),
        seed=0x2097,
        ticks=240,
        assertions=("har2.health>0@200",),
    )
    result = run_rec(openomf_bin, build_dir, rec_file)
    assert result.returncode == 0, result.stdout + result.stderr
