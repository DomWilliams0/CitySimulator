#!/usr/bin/env bash

TARGET_USAGE="run or tests"
BUILD_DIR=".build"
action="$1"
original_target="$2"
target="$2"
args="${@:3}"

# default action
if [ -z "$action" ]; then
  action="all"
fi

fail() {
	echo "Invalid target ($TARGET_USAGE)"
	exit 1
}

convert_target() {
	case "$target" in
		"run")
			target="CitySimulator_run"
			;;
		"tests")
			target="CitySimulator_tests"
			;;
		*)
			fail
			;;
	esac
}

execute_target() {
	case "$original_target" in
		"run")
			$BUILD_DIR/$target CitySimulator $args
			;;
		"tests")
			cd $BUILD_DIR/CitySimulator_tests/tests
			./$target $args
			;;
		*)
			fail
			;;
	esac
}

do_cmake() {
  echo "Running cmake"

  mkdir -p "$BUILD_DIR" &&
  cd "$BUILD_DIR" &&
  cmake .. &&
  cd ..
}

do_make() {
  echo "Running make"
  cd "$BUILD_DIR" &&
  make $target &&
  cd ..
}

do_run() {
	if [ -z "$target" ] || [ ! check_target ]; then
		fail
	fi

	execute_target
}

do_clean() {
	echo "Clean..."
	if [ ! -z "$BUILD_DIR" ]; then
		rm -rf "$BUILD_DIR"
	else
		echo "Something went terribly wrong, aborting clean!"
		exit 2
	fi
}

convert_target
case "$action" in
  "all")
    if do_cmake && do_make ; then
      do_run
    fi
    ;;
  "cmake")
    do_cmake
    ;;
  "make")
	  echo "target is now $target"
    do_make
    ;;
  "run")
    do_run
    ;;
  "clean")
	  do_clean
	  ;;
  *)
	  echo "Usage: $0 [all <target> | cmake | make | run <target>]"
	exit 0
	;;
esac

