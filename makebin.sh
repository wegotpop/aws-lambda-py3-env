#/bin/bash

# TODO: Add documentation of makebin.sh here !!!

# NOTE: If any change happened in this file (makebin.sh) or in any of its
# dependencies (eg. constants.sh) the 'build' phase should not be skipped,
# because that one is copying these files into the image, and the later phases
# are depending on these copied files

# Skippable stages (flag: -s, --skip):
#   - clean
#   - build
#   - run (this will skip both custom and archive)
#       - custom
#       - archive
# Usage:
#   $ bash makebin.sh --skip build --skip archive


# Script level constants
BASH_THIS_FILE="$(basename "$0")";
BASH_CONSTANTS_FILE='constants.sh';
BASH_HELPERS_FILE='helpers.sh';

# Import shared constants
source "$BASH_CONSTANTS_FILE";
source "$BASH_HELPERS_FILE";


function build_image()
{
    # Install necessary binaries
    yum install --assumeyes wget tar gcc zip zlib-devel;

    # Download python source
    wget "$PYTHON_SOURCE_URL/$PYTHON_VER/$PYTHON_SOURCE_DIR.tgz";

    # Unarchive source and switch to the dir
    tar zxvf "$PYTHON_SOURCE_DIR.tgz";
    cd "$PYTHON_SOURCE_DIR";

    # Compile from source
    ./configure;
    make;
    cd ../;

    # Download python 2 source
    wget "$PYTHON_2_SOURCE_URL/$PYTHON_2_VER/$PYTHON_2_SOURCE_DIR.tgz";

    # Unarchive source and switch to the dir
    tar zxvf "$PYTHON_2_SOURCE_DIR.tgz";
    cd "$PYTHON_2_SOURCE_DIR";

    # Compile and install from source
    ./configure;
    make;
    make install;
    cd ../;
}


function compile_and_distribute()
{
    COMPILE_CUSTOM_INTERPRETER=true;
    CREATE_LOCALS_AND_ARCHIVE=true;

    # Deal with the arguments
    while [ -n "$1" ];
    do
        case "$1" in
            -s|--skip)
                shift;
                case "$1" in
                    custom) COMPILE_CUSTOM_INTERPRETER='';;
                    archive) CREATE_LOCALS_AND_ARCHIVE='';;
                    *)
                        printf "Error: Unknown parameter for --skip: \`$1'\n";
                        exit 1;;
                esac;;
            *)
                printf "Error: Unknown parameter: \`$1'\n";
                exit 1;;
        esac;
        shift;
    done;

    if [ -n "$COMPILE_CUSTOM_INTERPRETER" ];
    then
        # Compile customised interpreter
        gcc -std=c99                                \
            -O3                                     \
            -DNDEBUG                                \
            -march=native                           \
            -Wall                                   \
            -Wbad-function-cast                     \
            -Wcast-qual                             \
            -Wconversion                            \
            -Wdouble-promotion                      \
            -Wextra                                 \
            -Wfloat-equal                           \
            -Wformat=2                              \
            -Wmissing-prototypes                    \
            -Wpedantic                              \
            -Wredundant-decls                       \
            -Wstrict-prototypes                     \
            -Wundef                                 \
            -Wunsuffixed-float-constants            \
            -Wwrite-strings                         \
            -I$PYTHON_SOURCE_DIR/Include            \
            -I$PYTHON_SOURCE_DIR                    \
            -fwrapv                                 \
            -fdiagnostics-color=always              \
            -lpthread                               \
            -ldl                                    \
            -lutil                                  \
            -lm                                     \
            -Xlinker -export-dynamic                \
            -o $PROJECT_LOCAL_DIR/$POP_PYTHON_OUT   \
            $POP_PYTHON_SRC                         \
            $PYTHON_SOURCE_DIR/$PYTHON_STATIC_LIBRARY;

        # Compile the lambda-function entry point
        gcc -std=c99                                                        \
            -O3                                                             \
            -DNDEBUG                                                        \
            -march=native                                                   \
            -Wall                                                           \
            -Wbad-function-cast                                             \
            -Wcast-qual                                                     \
            -Wconversion                                                    \
            -Wdouble-promotion                                              \
            -Wextra                                                         \
            -Wfloat-equal                                                   \
            -Wformat=2                                                      \
            -Wmissing-prototypes                                            \
            -Wpedantic                                                      \
            -Wredundant-decls                                               \
            -Wstrict-prototypes                                             \
            -Wundef                                                         \
            -Wunsuffixed-float-constants                                    \
            -Wwrite-strings                                                 \
            -I/usr/local/include/python$PYTHON_2_VER_MAIN.$PYTHON_2_VER_SUB \
            -I$POP_PYTHON_DIR                                               \
            -pipe                                                           \
            -fdiagnostics-color=always                                      \
            -fexceptions                                                    \
            -fno-strict-aliasing                                            \
            -fPIC                                                           \
            -fstack-protector                                               \
            -fwrapv                                                         \
            -lpthread                                                       \
            -shared                                                         \
            -o $PROJECT_LOCAL_DIR/$PROJECT_ENTRY_NAME.so                    \
            $POP_PYTHON_DIR/$PROJECT_ENTRY_NAME.c;
    fi;

    if [ -n "$CREATE_LOCALS_AND_ARCHIVE" ];
    then
        # Create local-build directory if it does not exist yet and copy the
        # interpreter files and folders there
        mkdir -p "$PROJECT_LOCAL_DIR";
        cp -r "$PYTHON_SOURCE_DIR"/* "$PROJECT_LOCAL_DIR";
        cp -r "$PROJECT_SOURCES" "$PROJECT_LOCAL_DIR";
        cp "$PROJECT_ENTRY_POINT" "$PROJECT_LOCAL_DIR";

        # Create remote-build directory if it does not exist yet and create an
        # archive of the interpreter and copy that there
        mkdir -p "$PROJECT_REMOTE_DIR";
        cd "$PYTHON_SOURCE_DIR";
        zip -r "../$PROJECT_REMOTE_DIR/$PROJECT_ARCHIVE"    \
            python                                          \
            Grammar                                         \
            Lib                                             \
            Mac                                             \
            Objects                                         \
            PCbuild                                         \
            Python                                          \
            Tools                                           \
            Include                                         \
            Misc                                            \
            Parser                                          \
            Programs                                        \
            Doc                                             \
            Modules                                         \
            PC;

        # Extend the archive with the source of the project
        cd ../;
        zip -ur "$PROJECT_REMOTE_DIR/$PROJECT_ARCHIVE" \
                "$PROJECT_SOURCES"                     \
                -x \*__pycache__\*                     \
                "$PROJECT_ENTRY_POINT";
    fi;
}


function main()
{
    CLEAN_BUILD_ENABLED=true;
    BUILD_PHASE_ENABLED=true;
    RUN_PHASE_ENABLED=true;
    RUN_OPTIONS='';

    # Deal with the arguments
    while [ -n "$1" ];
    do
        case "$1" in
            -s|--skip)
                shift;
                case "$1" in
                    clean) CLEAN_BUILD_ENABLED='';;
                    build) BUILD_PHASE_ENABLED='';;
                    run) RUN_PHASE_ENABLED='';;
                    custom) RUN_OPTIONS="$RUN_OPTIONS --skip custom";;
                    archive) RUN_OPTIONS="$RUN_OPTIONS --skip archive";;
                    *)
                        printf "Error: Unknown parameter for --skip: \`$1'\n";
                        exit 1;;
                esac;;
            *)
                printf "Error: Unknown parameter: \`$1'\n";
                exit 1;;
        esac;
        shift;
    done;

    if [ -n "$CLEAN_BUILD_ENABLED" ];
    then
        sudo rm -r -f "$PROJECT_BUILD_DIR";
    fi;

    # Create build folder if it does not exist yet
    mkdir -p "$PROJECT_BUILD_DIR";
    mkdir -p "$PROJECT_LOCAL_DIR";
    mkdir -p "$PROJECT_REMOTE_DIR";

    if [ -n "$BUILD_PHASE_ENABLED" ];
    then
        # Construct and save the docker-file
        printf '%s\n'                                   \
            "FROM    $DOCKER_IMAGE_NAME"                \
            "WORKDIR $DOCKER_WORKING_DIR"               \
            "ENV     $BASH_ENV_VARIABLE BUILDING_PHASE" \
            "COPY    $BASH_THIS_FILE ."                 \
            "COPY    $BASH_CONSTANTS_FILE ."            \
            "COPY    $BASH_HELPERS_FILE ."              \
            "RUN     bash $BASH_THIS_FILE"              \
            > "$PROJECT_BUILD_DIR/$DOCKER_FILE_NAME";

        # Build container
        docker build --tag "$DOCKER_PYTHON_IMAGE_NAME"              \
                     --file "$PROJECT_BUILD_DIR/$DOCKER_FILE_NAME"  \
                     .;
    fi;

    if [ -n "$RUN_PHASE_ENABLED" ];
    then
        # Remove previous archive file if present
        rm -f "$PROJECT_BUILD_DIR/$PROJECT_ARCHIVE";

        # Run container
        docker run --volume $(volumiser $POP_PYTHON_DIR)        \
                   --volume $(volumiser $PROJECT_SOURCES)       \
                   --volume $(volumiser $PROJECT_BUILD_DIR)     \
                   --volume $(volumiser $PROJECT_ENTRY_POINT)   \
                   --env $BASH_ENV_VARIABLE=RUNNING_PHASE       \
                   --name $DOCKER_PYTHON_CONTAINER_NAME         \
                   "$DOCKER_PYTHON_IMAGE_NAME"                  \
                   bash $BASH_THIS_FILE $RUN_OPTIONS;

        # Clean up
        docker stop "$DOCKER_PYTHON_CONTAINER_NAME";
        docker rm "$DOCKER_PYTHON_CONTAINER_NAME";
    fi;
}


case "${!BASH_ENV_VARIABLE}" in
    # If inside docker
    BUILDING_PHASE) build_image $@;;
    RUNNING_PHASE) compile_and_distribute $@;;
    # If outside docker
    *) main $@;;
esac;
