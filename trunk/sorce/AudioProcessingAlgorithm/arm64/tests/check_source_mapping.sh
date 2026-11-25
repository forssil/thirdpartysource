#!/bin/sh
set -eu

root=${1:-.}
arm_root="$root/arm64"
independent='
arm64/common_audio/source/Amp_compute.c
arm64/common_audio/source/DCRemover.cpp
arm64/common_audio/SampleRateConvert/src/src_linear.c
arm64/acoustic_echo_cancellation/source/PSDsmooth.cpp
arm64/acoustic_echo_cancellation/source/AdapFilterGroup.cpp
arm64/acoustic_echo_cancellation/source/postfilter.cpp
arm64/rnnoise/source/celt_lpc.c
arm64/rnnoise/source/denoise.c
arm64/rnnoise/source/rnn.c
'

is_independent()
{
    printf '%s' "$independent" | grep -Fxq "$1"
}

count=0
original_count=0

for original_source in \
    "$root/AudioLog.cpp" \
    $(find "$root/acoustic_echo_cancellation" "$root/agc" \
        "$root/audio_processing" "$root/beam_forming" "$root/common_audio" \
        "$root/mixer" "$root/resample" "$root/rnnoise" "$root/vad" \
        -type f \( -name '*.c' -o -name '*.cpp' \) \
        ! -path '*/unitest/*' ! -path '*/unittest/*' | LC_ALL=C sort); do
    relative=${original_source#"$root/"}
    arm_source="$arm_root/$relative"
    if test ! -f "$arm_source"; then
        echo "Missing ARM64 source mapping: $relative" >&2
        exit 1
    fi
    original_count=$((original_count + 1))
done

for arm_source in $(find "$arm_root" -type f \( -name '*.c' -o -name '*.cpp' \) \
    ! -path '*/tests/*' | LC_ALL=C sort); do
    relative=${arm_source#"$root/"}
    case "$relative" in
        arm64/*/unitest/*|arm64/*/unittest/*)
            continue
            ;;
    esac
    if is_independent "$relative"; then
        count=$((count + 1))
        continue
    fi

    include=$(sed -n 's/^#include "\([^"]*\.[cC][pP]*\)"$/\1/p' \
        "$arm_source" | head -n 1)
    if test -z "$include"; then
        echo "Missing original-source include: $relative" >&2
        exit 1
    fi
    include_dir=$(dirname "$arm_source")
    original=$(cd "$include_dir" && cd "$(dirname "$include")" && \
        printf '%s/%s\n' "$PWD" "$(basename "$include")")
    if test ! -f "$original"; then
        echo "Broken original-source include: $relative -> $include" >&2
        exit 1
    fi
    count=$((count + 1))
done

if test "$count" -ne "$original_count"; then
    echo "Source mapping count mismatch: original=$original_count arm64=$count" >&2
    exit 1
fi

independent_count=$(printf '%s\n' "$independent" |
    awk 'NF { count++ } END { print count + 0 }')
echo "ARM64 source mapping audit passed: $count/$original_count production sources, $independent_count independent implementations"
