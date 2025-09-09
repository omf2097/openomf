#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Usage: $0 <build-dir>" >&2
    exit 1
fi

export BUILD_DIR="$1"
OPENOMF_BIN=$(find "$BUILD_DIR" -name openomf -type f -executable -print -quit)
if [ -z "$OPENOMF_BIN" ]; then
    echo "Could not find openomf executable from $BUILD_DIR" >&2
    exit 1
fi
export OPENOMF_BIN="./${OPENOMF_BIN#$BUILD_DIR}"

# Define your tests here (description:filename)
tests=(
    "Overhead throw should grab opponent on the first tick of anim 22, throw opponent to the right, and not be interrupted by the ground:JAG-THROW-LAND.REC"
    "Electra should be able to kick while doing inputs for rolling thunder:65K6P.REC"
    "Jaguar uses same animation for K and 6P but Shadow does not:6P.REC"
    "Novas 1P can hit more than once with the Q tag and use AI for air launch:NOVA-1P.REC"
    "Jaguar standing throw should not result in overlap with opponent in corner:JAG-CORNER-THROW.REC"
    "Jaguar doing a standing throw as the winning move should not cause a slide:JAG-NO-SLIDE-WIN-THROW.REC"
    "Flail charging punch should cancel when blocked:FLAIL_454P.REC"
    "Jaguar should keep walking right after doing a standing throw if the input is held:KEEP-WALKING-AFTER-JAG-STANDING-THROW.REC"
    "Jaguar should be able to walk underneath Gargoyle:WALK_UNDERNEATH.REC"
    "Test Jaguar destruction:JAG-DESTRUCT.REC"
    "Kreissack has a custom defeat animation:KREISSACK.REC"
    "Shadow grab should work in the middle and the corners of arena:SHADOW-GRAB.REC"
    "Projectiles should do knockback:PROJECTILE-KNOCKBACK.REC"
    "Enemy should recover from air hits unless it is a KO:AIR-HIT.REC"
    "Only some moves should wallslam:STADIUM-WALL-SPLAT.REC"
    "Power Plant with hazards enabled has a lower wallslam tolerance:POWERPLANT-WALL-SPLAT.REC"
    "HARs in stasis cannot hit opponent:STASISED-HARS-CANNOT-HIT.REC"
    "HARs play ANIM_DEFEAT instead of idle:NO-IDLE-DEFEAT.REC"
    "Electra P656 cannot be interrupted during anim 36:ELECTRA-MOVE36-EMFLAG.REC"
    "Chronos matter phasing (14K) noclips through both walls:CHRONOS-WALLNOCLIP.REC"
    "Chronos can control teleport by holding left/right:CHRONOS_TELEPORT_CONTROL.REC"
    "Electra Extended Rolling Thunder plays anim 40 upon hitting the wall:ELECTRA-EXROLLTHNDR-WALLBOUNCE.REC"
    "Katana Wall Spin (14K) cannot be interrupted:KATANA-WALLSPIN-NOINTERRUPT.REC"
    "Being kicked in the back of the head by chronos matter phasing knocks you forwards, flipping you:KNOCKBACK-DIR.REC"
    "While walking, face your jumping enemy:FACE-JUMPING-ENEMY.REC"
    "Nova's grenade should not fly over Electra's head:NOVA-GRENADE.REC"
    "Buffering 2 (DOWN) 6 ticks before jumping grants a SUPER JUMP. Only the first two jumps should be super.:SUPERJUMP.REC"
    "Buffered inputs are only used to match moves if the direction has changed in the last 9 ticks:INPUT-FRESHNESS.REC"
    "Stasis prevents walking, turning, and endurance regen:STASIS-FREEZES.REC"
    "Electra's shards can be blocked:SHARDS-ARE-BLOCKABLE.REC"
    "Electra can throw Katana on wakeup without getting hit:WAKEUP_THROW.REC"
    "Katana can chain stomps and end with a jump kick:STOMP.REC"
    "Gargoyle's diving claw doesn't flip the opponent around:DIVING-CLAW.REC"
    "Flail cannot hit air 66K throw out of range:THROW-RANGE.REC"
    "Gargoyle reaches the top of the stage:FLY.REC"
    "Shadow grab can be interrupted by hitting the originating HAR:SHADOW-GRAB-INTERRUPT-HIT.REC"
    "Jaguar throw deals half stun due to rehit rules:DAMPENED_STUN.REC"
    "Pyros's flames cannot be hit:PYROS_PRIORITY.REC"
    "Flail's destruct sequence works completely:FLAIL_DESTRUCT.REC"
    "Electra is stuck blocking until Thorn's ul tag wears off:UL_TAG_HOLD_UP.REC"
    "Pyros's destruct sequence works completely:PYROS_DESTRUCT.REC"
    "Katana's corkscrew blade has landing recovery:LANDING_RECOVERY.REC"
    "Flail 66K is unblockable:UNBLOCKABLE_THROW.REC"
    "Electra 66P dodges Earthquake Smash:DODGE_EARTHQUAKE.REC"
    "Katana's extended rising blade deals the correct amount of damage:KATANA_DAMAGE.REC"
    "Enemies have invuln after recovering from an air hit:AIR_IMMUNITY_NO_REHIT.REC"
    "Enemies cannot be juggled by the same move:AIR_IMMUNITY_REHIT.REC"
    "Jaguar's Bread And Butter throw combo:REHIT_COMBO.REC"
    "Jaguar cannot pick up the opponent off the ground:WAKEUP_THROW_PROTECTION.REC"
    "Players cannot blocking while attacking:BLOCKING_INTERRUPT.REC"
    "Katana cannot infinitely heel stomp to the moon with rehit mode:SPACE_JUMP.REC"
    "Katana can turn around after heel stomp connects:KATANA_STOMP_TURN.REC"
    "Nova can combo a stunned HAR from belly flop and enemy continues to be stunned:NOVA_FLOP_STUN_COMBO.REC"
    "Katana enters winpose after landing from heel stomp:KATANA_WINPOSE.REC"
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

export LSAN_OPTIONS="suppressions=../lsan.supp"
i=0
for test in "${tests[@]}"; do
    IFS=':' read -r desc filename <<< "$test"
    # Trim whitespace from description and filename
    desc=$(echo "$desc" | sed -re 's/^[[:blank:]]+|[[:blank:]]+$//g')
    filename=$(echo "$filename" | sed -re 's/^[[:blank:]]+|[[:blank:]]+$//g')
    output_file="$temp_dir/output_$i.log"

    echo -n "${desc} :"
    if $OPENOMF_BIN --force-audio-backend=NULL --force-renderer=NULL --speed=10 -P "$RUNDIR/rectests/${filename}" >"$output_file"  2>&1; then
        echo " PASS"
    else
        echo " FAILED (${filename})"
        fail_summary="${fail_summary} ${filename}"
        cat $output_file
        ((fail_count++))
    fi
    ((i++))
done

if [ $fail_count -ne 0 ]; then
    fail_summary="Failed ${fail_count} tests:${fail_summary}"
    echo "${fail_summary}"

    # Exit with non-zero status if any test failed
    exit $fail_count
fi
