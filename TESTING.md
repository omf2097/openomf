# OpenOMF Testing

## Prerequisites

Testing requires a build configured with tests and tools enabled. From the
repository root:

    cmake -S . -B build -DUSE_TESTS=ON -DUSE_TOOLS=ON
    cmake --build build -j

The replay-based tests also need the original OMF:2097 game data files (see
[BUILD.md](BUILD.md), "Data Files") present in `build/resources/` — for example
`SOUNDS.DAT`, `ARENA*.BK`, `FIGHTR*.AF`, and `ENGLISH.DAT`.

## Test entry points

| Command | What it runs |
| --- | --- |
| `ctest --test-dir build --output-on-failure` | CUnit unit tests |
| `./run_rectests.sh build` | REC replay regression corpus |
| `./run_pytest.sh build` | Python tests, including deterministic REC tests |

## CUnit unit tests

The CUnit binary is built as `build/openomf_test_main` and is run through
CTest:

    ctest --test-dir build --output-on-failure

## REC replay regression tests

`run_rectests.sh` replays the recorded-match corpus under `rectests/` with null
audio and rendering, and reports any failures.

## Python tests

`run_pytest.sh` runs the suite under `pytest/`, which includes the menu tests
and the deterministic REC smoke tests in `pytest/test_rec_assertions.py`.

### Deterministic REC tests

These tests generate temporary REC files on the fly with `build/gen_move_test`,
replay them headlessly with null audio and video, and check the assertions
embedded in the REC:

    poetry run pytest -vrP pytest/test_rec_assertions.py

The generator emits a REC from a compact description:

    build/gen_move_test <har_id> <opponent_id> <out.rec> \
        --seed 0x2097 --ticks 240 --assert 'har2.health>0@200' 40 40 44 01

The deterministic tests require an `openomf` executable and `gen_move_test` in
the selected build directory. Set `BUILD_DIR` and `OPENOMF_BIN` when using a
non-default build layout. Generated REC files are written to temporary
directories and are never added to the source tree.
