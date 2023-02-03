#!/bin/bash
# -fqbn=arduino-git:mbed:nicla_voice

PROJECT=firmware-arduino-nicla-voice
COMMAND=$1
# use --with-imu flag as 2nd argument to build firmware for IMU
SENSOR=$2

# used for grepping
ARDUINO_CORE="arduino-git:mbed" # temporary !

BOARD="${ARDUINO_CORE}":nicla_voice

if [ -z "$ARDUINO_CLI" ]; then
	ARDUINO_CLI=$(which arduino-cli || true)
fi

DIRNAME="$(basename "$SCRIPTPATH")"
EXPECTED_CLI_MAJOR=0
EXPECTED_CLI_MINOR=21

if [ ! -x "$ARDUINO_CLI" ]; then
    echo "Cannot find 'arduino-cli' in your PATH. Install the Arduino CLI before you continue."
    echo "Installation instructions: https://arduino.github.io/arduino-cli/latest/"
    exit 1
fi

CLI_MAJOR=$($ARDUINO_CLI version | cut -d. -f1 | rev | cut -d ' '  -f1)
CLI_MINOR=$($ARDUINO_CLI version | cut -d. -f2)
CLI_REV=$($ARDUINO_CLI version | cut -d. -f3 | cut -d ' '  -f1)

if (( CLI_MINOR < EXPECTED_CLI_MINOR)); then
    echo "You need to upgrade your Arduino CLI version (now: $CLI_MAJOR.$CLI_MINOR.$CLI_REV, but required: $EXPECTED_CLI_MAJOR.$EXPECTED_CLI_MINOR.x or higher)"
    echo "See https://arduino.github.io/arduino-cli/installation/ for upgrade instructions"
    exit 1
fi

if (( CLI_MAJOR != EXPECTED_CLI_MAJOR || CLI_MINOR != EXPECTED_CLI_MINOR )); then
    echo "You're using an untested version of Arduino CLI, this might cause issues (found: $CLI_MAJOR.$CLI_MINOR.$CLI_REV, expected: $EXPECTED_CLI_MAJOR.$EXPECTED_CLI_MINOR.x)"
fi

#$ARDUINO_CLI config dump | grep 'user: '
get_arduino_root() {
	local OUTPUT=$($ARDUINO_CLI config dump | grep 'user: ')
	local arduino_root="${OUTPUT:8}"
	echo "$arduino_root"
}
ARDUINO_ROOT_DIR="$(get_arduino_root)"

get_library_dir() {
	local OUTPUT=$($ARDUINO_CLI config dump | grep 'user: ')
	local lib="${OUTPUT:8}"/libraries
	echo "$lib"
}

ARDUINO_LIB_DIR="$(get_library_dir)"
if [[ -z "$ARDUINO_LIB_DIR" ]]; then
    echo "Arduino libraries directory not found"
    exit 1
fi

create_library_dir() {
	local OUTPUT=$($ARDUINO_CLI config dump | grep 'user: ')
	local lib="${OUTPUT:8}"/libraries
	mkdir $OUTPUT
    mkdir $lib
}

if [ ! -d "$ARDUINO_LIB_DIR" ]; then
    create_library_dir
fi

# Check for libraries - sperimental for now.
# Board lib
has_mbed_core() {
    $ARDUINO_CLI core list | grep -e "arduino-git:mbed.*9.9.9"
}
HAS_ARDUINO_CORE="$(has_mbed_core)"
if [ -z "$HAS_ARDUINO_CORE" ]; then
    echo "Installing Arduino Nicla Voice core..."

    wget -O "${ARDUINO_ROOT_DIR}/hardware-mbed-git-nicla-voice-v91-imu.zip" https://cdn.edgeimpulse.com/build-system/hardware-mbed-git-nicla-voice-v91-imu.zip
    unzip "${ARDUINO_ROOT_DIR}/hardware-mbed-git-nicla-voice-v91-imu.zip" -d "${ARDUINO_ROOT_DIR}/"

    rm ${ARDUINO_ROOT_DIR}/hardware-mbed-git-nicla-voice-v91-imu.zip 
    echo "Installing Arduino Nicla Voice core OK"
fi

has_nicla_core() {
    $ARDUINO_CLI core list | grep -e "arduino:mbed_nicla.*3.1.1"
}
HAS_NICLA_CORE="$(has_nicla_core)"
if [ -z "$HAS_NICLA_CORE" ]; then
    echo "Installing Arduino Nicla core..."
    $ARDUINO_CLI core update-index
    $ARDUINO_CLI core install arduino:mbed_nicla@3.1.1
    echo "Installing Arduino Nicla core OK"
fi

# Check Nicla_System
has_Nicla_sense_lib() {
	$ARDUINO_CLI lib list Nicla_System | grep 1.0 || true
}
HAS_NICLA_SENSE_LIB="$(has_Nicla_sense_lib)"
if [ -z "$HAS_NICLA_SENSE_LIB" ]; then
    if [ -d "$HAS_NDP_UTILS_LIB/Nicla_System" ]; then
        rm -r "$HAS_NDP_UTILS_LIB/Nicla_System"
    fi
    echo "Installing Nicla_Sense_System library..."
    cp -R lib/Nicla_System "${ARDUINO_LIB_DIR}"
    echo "Installing Nicla_Sense_System library OK"
fi

# Check for NDP disabled, we use the one in the /src for now
#has_NDP_lib() {
#	$ARDUINO_CLI lib list NDP | grep 1.0.0 || true
#}
#HAS_NDP_LIB="$(has_NDP_lib)"
#if [ -z "$HAS_NDP_LIB" ]; then
#    echo "Installing NDP library..."
#    cp -R lib/NDP "${ARDUINO_LIB_DIR}"
#    echo "Installing NDP library OK"
#fi

# Check NDP_utils v1.0.2
has_NDP_utils_lib() {
	$ARDUINO_CLI lib list NDP_utils | grep 1.0.2 || true
}
HAS_NDP_UTILS_LIB="$(has_NDP_utils_lib)"
if [ -z "$HAS_NDP_UTILS_LIB" ]; then
    if [ -d "$HAS_NDP_UTILS_LIB/NDP_utils" ]; then
        rm -r "$HAS_NDP_UTILS_LIB/NDP_utils"
    fi
    echo "Installing NDP_utils library..."
    cp -R lib/NDP_utils "${ARDUINO_LIB_DIR}"
    echo "Installing NDP_utils library OK"
fi

# Functions

# Include path
INCLUDE="-I ./src"
INCLUDE+=" -I./src/ingestion-sdk"
INCLUDE+=" -I./src/ingestion-sdk-platform/sensors"
INCLUDE+=" -I./src/firmware-sdk"
INCLUDE+=" -I ./src/QCBOR/inc"
INCLUDE+=" -I ./src/sensor_aq_mbedtls"
# Flags
FLAGS="-DARDUINOSTL_M_H"
#FLAGS+=" -DMBED_HEAP_STATS_ENABLED=1"
#FLAGS+=" -DMBED_STACK_STATS_ENABLED=1"
FLAGS+=" -O1"
#FLAGS+=" -DMBED_DEBUG"
#FLAGS+=" -g3"
FLAGS+=" -DEI_SENSOR_AQ_STREAM=FILE"
FLAGS+=" -DEI_PORTING_ARDUINO=1"

if [ "$SENSOR" = "--with-imu" ];
then    
    CPP_FLAGS="-DWITH_IMU"
else
    CPP_FLAGS=""
fi

if [ "$COMMAND" = "--build" ];
then
    echo "Building $PROJECT"
    arduino-cli compile --fqbn  $BOARD --build-property build.extra_flags="$INCLUDE $FLAGS" --build-property "compiler.cpp.extra_flags=${CPP_FLAGS}" --output-dir . &
    pid=$! # Process Id of the previous running command
    while kill -0 $pid 2>/dev/null
    do
        echo "Still building..."
        sleep 2
    done
    wait $pid
    ret=$?
    if [ $ret -eq 0 ]; then
        echo "Building $PROJECT done"
    else
        exit "Building $PROJECT failed"
    fi
elif [ "$COMMAND" = "--flash" ];
then
    echo "Flashing board"
    arduino-cli upload --fqbn  $BOARD
elif [ "$COMMAND" = "--all" ];
then
    echo "Building and flashing $PROJECT"
        echo "Building $PROJECT"
    arduino-cli compile --fqbn  $BOARD --build-property build.extra_flags="$INCLUDE $FLAGS" --build-property "compiler.cpp.extra_flags=${CPP_FLAGS}" --output-dir . &
    pid=$! # Process Id of the previous running command
    while kill -0 $pid 2>/dev/null
    do
        echo "Still building..."
        sleep 2
    done
    wait $pid
    ret=$?
    if [ $ret -eq 0 ]; then
        echo "Building $PROJECT done"
        echo "Flashing board"
        arduino-cli upload --fqbn  $BOARD
    else
        exit "Building $PROJECT failed"
    fi
else
    echo "Nothing to do for target"
fi