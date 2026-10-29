#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Usage: $0 <build-dir>" >&2
    exit 1
fi

export BUILD_DIR="$1"
OPENOMF_BIN=$(find "$BUILD_DIR" "(" -name openomf -or -name openomf.exe ")" -type f -exec test -x {} \; -print | head -n1)
if [ -z "$OPENOMF_BIN" ]; then
    echo "Could not find openomf executable from $BUILD_DIR" >&2
    exit 1
fi
export OPENOMF_BIN="./${OPENOMF_BIN#$BUILD_DIR}"

# Descrube your tests here, with ["filename"]="description"
# NOTE: Whitespace in the wrong place will break everything, e.g. ["filename"] = "description"
# NOTE: double quotes cannot be present in the description, as i'm not escaping them down below.
declare -A descriptions=(
    ["JUMP_DELAY.REC"]="There is a 3 tick delay between jumps"
    ["JAG-THROW-LAND.REC"]="Overhead throw should grab opponent on the first tick of anim 22, throw opponent to the right, and not be interrupted by the ground"
    ["65K6P.REC"]="Electra should be able to kick while doing inputs for rolling thunder"
    ["6P.REC"]="Jaguar uses same animation for K and 6P but Shadow does not"
    ["NOVA-1P.REC"]="Novas 1P can hit more than once with the Q tag and use AI for air launch"
    ["JAG-CORNER-THROW.REC"]="Jaguar standing throw should not result in overlap with opponent in corner"
    ["JAG-NO-SLIDE-WIN-THROW.REC"]="Jaguar doing a standing throw as the winning move should not cause a slide"
    ["FLAIL_454P.REC"]="Flail charging punch should cancel when blocked"
    ["KEEP-WALKING-AFTER-JAG-STANDING-THROW.REC"]="Jaguar should keep walking right after doing a standing throw if the input is held"
    ["WALK_UNDERNEATH.REC"]="Jaguar should be able to walk underneath Gargoyle"
    ["JAG-DESTRUCT.REC"]="Test Jaguar destruction"
    ["KREISSACK.REC"]="Kreissack has a custom defeat animation"
    ["SHADOW-GRAB.REC"]="Shadow grab should work in the middle and the corners of arena"
    ["PROJECTILE-KNOCKBACK.REC"]="Projectiles should do knockback"
    ["AIR-HIT.REC"]="Enemy should recover from air hits unless it is a KO"
    ["STADIUM-WALL-SPLAT.REC"]="Only some moves should wallslam"
    ["POWERPLANT-WALL-SPLAT.REC"]="Power Plant with hazards enabled has a lower wallslam tolerance"
    ["STASISED-HARS-CANNOT-HIT.REC"]="HARs in stasis cannot hit opponent"
    ["NO-IDLE-DEFEAT.REC"]="HARs play ANIM_DEFEAT instead of idle"
    ["ELECTRA-MOVE36-EMFLAG.REC"]="Electra P656 cannot be interrupted during anim 36"
    ["CHRONOS-WALLNOCLIP.REC"]="Chronos matter phasing (14K) noclips through both walls"
    ["CHRONOS_TELEPORT_CONTROL.REC"]="Chronos can control teleport by holding left/right"
    ["ELECTRA-EXROLLTHNDR-WALLBOUNCE.REC"]="Electra Extended Rolling Thunder plays anim 40 upon hitting the wall"
    ["KATANA-WALLSPIN-NOINTERRUPT.REC"]="Katana Wall Spin (14K) cannot be interrupted"
    ["KNOCKBACK-DIR.REC"]="Being kicked in the back of the head by chronos matter phasing knocks you forwards, flipping you"
    ["FACE-JUMPING-ENEMY.REC"]="While walking, face your jumping enemy"
    ["NOVA-GRENADE.REC"]="Nova's grenade should not fly over Electra's head"
    ["SUPERJUMP.REC"]="Buffering 2 (DOWN) 6 ticks before jumping grants a SUPER JUMP. Only the first two jumps should be super."
    ["INPUT-FRESHNESS.REC"]="Buffered inputs are only used to match moves if the direction has changed in the last 9 ticks"
    ["STASIS-FREEZES.REC"]="Stasis prevents walking, turning, and endurance regen"
    ["SHARDS-ARE-BLOCKABLE.REC"]="Electra's shards can be blocked"
    ["WAKEUP_THROW.REC"]="Electra can throw Katana on wakeup without getting hit"
    ["STOMP.REC"]="Katana can chain stomps and end with a jump kick"
    ["DIVING-CLAW.REC"]="Gargoyle's diving claw doesn't flip the opponent around"
    ["THROW-RANGE.REC"]="Flail cannot hit air 66K throw out of range"
    ["THROW_RANGE_BUG.REC"]="Flail cannot hit 66K if the followup animation is out of range"
    ["THROW_INTERRUPT.REC"]="Flail cannot be interrupted by hazards after throwing"
    ["THROW_KO.REC"]="Flail's 66K attempt KO's the opponent and the game is fine with it"
    ["STUN_THROW.REC"]="Throwing a stunned opponent resets their stun bar"
    ["FLY.REC"]="Gargoyle reaches the top of the stage"
    ["SHADOW-GRAB-INTERRUPT-HIT.REC"]="Shadow grab can be interrupted by hitting the originating HAR"
    ["DAMPENED_STUN.REC"]="Jaguar throw deals half stun due to rehit rules"
    ["PYROS_PRIORITY.REC"]="Pyros's flames cannot be hit"
    ["FLAIL_DESTRUCT.REC"]="Flail's destruct sequence works completely"
    ["UR_TAG_HOLD_UP.REC"]="Electra is minus after blocking Katana's slide"
    ["PYROS_DESTRUCT.REC"]="Pyros's destruct sequence works completely"
    ["LANDING_RECOVERY.REC"]="Katana's corkscrew blade has landing recovery"
    ["UNBLOCKABLE_THROW.REC"]="Flail 66K is unblockable"
    ["DODGE_EARTHQUAKE.REC"]="Electra 66P dodges Earthquake Smash"
    ["KATANA_DAMAGE.REC"]="Katana's extended rising blade deals the correct amount of damage"
    ["AIR_IMMUNITY_NO_REHIT.REC"]="Enemies have invuln after recovering from an air hit"
    ["AIR_IMMUNITY_REHIT.REC"]="Enemies cannot be juggled by the same move"
    ["REHIT_COMBO.REC"]="Jaguar's Bread And Butter throw combo"
    ["WAKEUP_THROW_PROTECTION.REC"]="Jaguar cannot pick up the opponent off the ground"
    ["BLOCKING_INTERRUPT.REC"]="Players cannot blocking while attacking"
    ["SPACE_JUMP.REC"]="Katana cannot infinitely heel stomp to the moon with rehit mode"
    ["KATANA_STOMP_TURN.REC"]="Katana can turn around after heel stomp connects"
    ["NOVA_FLOP_STUN_COMBO.REC"]="Nova can combo a stunned HAR from belly flop and enemy continues to be stunned"
    ["NO_STUN_JUGGLE.REC"]="Rehit juggles are not allowed against stunned opponents"
    ["KATANA_WINPOSE.REC"]="Katana enters winpose after landing from heel stomp"
    ["ROUNDONE.REC"]="Round one starts on tick 90"
    ["CHRONOS_AIR_STASIS.REC"]="Chronos can attack opponent when in stasis in the air and rehit mode is disabled"
    ["CORNER_TEST.REC"]="Player 2 can crossup in the left corner and neither player can crossup in the right corner"
    ["BLOCK_DIR.REC"]="Player can hold back to block even after being crossed up"
)

# Setup temp directory for outputs
temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT

interrupt() {
    echo "Test run interrupted" >&2
    exit 1
}
trap interrupt INT

fail_count=0
fail_summary=""

echo "Running tests..."

RUNDIR=$(pwd)

cd $BUILD_DIR

export OPENOMF_RESOURCE_PATH="${OPENOMF_RESOURCE_PATH:-.}"
export LSAN_OPTIONS="suppressions=../lsan.supp"

output_file="$temp_dir/output_shouldfail.log"
if $OPENOMF_BIN --force-audio-backend=NULL --force-renderer=NULL --speed=10 --play "$RUNDIR/rectests/SHOULDFAIL.REC" >"$output_file" 2>&1; then
    cat "$output_file"
    echo "CRITICAL ERROR: SHOULDFAIL.REC succeeded."
    exit 1
fi

cmd="\$OPENOMF_BIN --force-audio-backend=NULL --force-renderer=NULL --speed 10"

for filename in "$RUNDIR"/rectests/*.REC; do
    if [[ "$filename" == *SHOULDFAIL.REC ]]; then
        continue
    fi
    basename="$(basename "${filename}")"
    cmd="$cmd --play \"$filename\" --play-description \"${descriptions[${basename}]}\""
    found_something=1
done

if [ -z "${found_something}" ]; then
    echo "Failed to find any rectests!"
    exit 1
fi

if RUNDIR="${RUNDIR}" OPENOMF_BIN="${OPENOMF_BIN}" sh -c "$cmd"; then
    echo "ALL PASSED."
else
    echo "RECTESTS FAILED, GOODBYE."
    exit 1
fi
