#/bin/bash

# TODO: Add documentation of makebin.sh here !!!

# NOTE: If any change happened in this file (makebin.sh) or in any of its
# dependencies (eg. constants.sh) the 'build' phase should not be skipped,
# because that one is copying these files into the image, and the later phases
# are depending on these copied files

# Usage:
#   $ bash makebin.sh --skip build --skip finalise


# Script level constants
BASH_ENV_VARIABLE='PROCESS_PHASE';
BASH_THIS_FILE="$(basename "$0")";
BASH_CONSTANTS_FILE='constants.sh';

# Import shared constants
source "$BASH_CONSTANTS_FILE";

# Construct and store docker-file content
read -r -d '' DOCKER_FILE_CONTENT <<- END_OF_DOCKER_FILE
    FROM    $DOCKER_IMAGE_NAME
    WORKDIR $DOCKER_WORKING_DIR
    ENV     $BASH_ENV_VARIABLE BUILDING_PHASE
    COPY    $BASH_THIS_FILE .
    COPY    $BASH_CONSTANTS_FILE .
    RUN     bash $BASH_THIS_FILE
END_OF_DOCKER_FILE


function build_python()
{
    # Install necessary binaries
    yum install --assumeyes wget tar gcc zip zlib-devel;

    # Download python source
    wget "$PYTHON_SOURCE_URL/$PYTHON_VERSION/$PYTHON_SOURCE_DIR.tgz";

    # Unarchive source and swithc to dir
    tar zxvf "$PYTHON_SOURCE_DIR.tgz";
    cd "$PYTHON_SOURCE_DIR";

    # Compile from source and install it
    ./configure;
    make;
}


function prepare_archive()
{
    # Create local-build directory if it does not exist yet and copy the
    # interpreter files and folders there
    mkdir --parents "$PROJECT_LOCAL_DIR";
    cp --recursive "$PYTHON_SOURCE_DIR"/* "$PROJECT_LOCAL_DIR";

    # Create remote-build directory if it does not exist yet and create an
    # archive of the interpreter and copy that there
    mkdir --parents "$PROJECT_REMOTE_DIR";
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
}


function make_binary()
{
    CLEAN_BUILD_ENABLED=true;
    BUILD_PHASE_ENABLED=true;
    RUN_PHASE_ENABLED=true;
    FINALISE_PHASE_ENABLED=true;

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
                    finalise) FINALISE_PHASE_ENABLED='';;
                    *)
                        printf "Unknown parameter for \`--skip': \`$1'\n";
                        exit 1;;
                esac;;
            *)
                printf "Unknown parameter: \`$1'\n";
                exit 1;;
        esac;
        shift;
    done;

    if [ -n "$CLEAN_BUILD_ENABLED" ];
    then
        sudo rm --recursive --force "$PROJECT_BUILD_DIR";
    fi;

    # Create build folder if it does not exist yet
    mkdir --parents "$PROJECT_BUILD_DIR";

    if [ -n "$BUILD_PHASE_ENABLED" ];
    then
        # Save the docker-file
        printf "$DOCKER_FILE_CONTENT" > "$PROJECT_BUILD_DIR/$DOCKER_FILE_NAME";

        # Build container
        docker build --tag "$DOCKER_PYTHON_IMAGE_NAME"              \
                     --file "$PROJECT_BUILD_DIR/$DOCKER_FILE_NAME"   \
                     .;
    fi;

    if [ -n "$RUN_PHASE_ENABLED" ];
    then
        # Remove previous archive file if present
        rm -f "$PROJECT_BUILD_DIR/$PROJECT_ARCHIVE";

        # Build directory mount point in the container
        DIR="$(pwd)/$PROJECT_BUILD_DIR:/$DOCKER_WORKING_DIR/$PROJECT_BUILD_DIR";

        # Run container
        docker run --volume $DIR                            \
                   --env $BASH_ENV_VARIABLE=RUNNING_PHASE   \
                   --name $DOCKER_PYTHON_CONTAINER_NAME     \
                   "$DOCKER_PYTHON_IMAGE_NAME"              \
                   bash $BASH_THIS_FILE;

        # Clean up
        docker stop "$DOCKER_PYTHON_CONTAINER_NAME";
        docker rm "$DOCKER_PYTHON_CONTAINER_NAME";
    fi;

    if [ -n "$FINALISE_PHASE_ENABLED" ];
    then
        mkdir --parents "$PROJECT_LOCAL_DIR";
        sudo cp --recursive "$PROJECT_SOURCES" "$PROJECT_LOCAL_DIR";

        mkdir --parents "$PROJECT_REMOTE_DIR";
        # Extend the archive with the source of the project
        sudo zip -ur "$PROJECT_REMOTE_DIR/$PROJECT_ARCHIVE" \
                 "$PROJECT_SOURCES"                         \
                 -x \*__pycache__\*;
    fi;
}


case "${!BASH_ENV_VARIABLE}" in
    # If inside docker
    BUILDING_PHASE) build_python;;
    RUNNING_PHASE) prepare_archive;;
    # If outside docker
    *) make_binary $@;;
esac;
