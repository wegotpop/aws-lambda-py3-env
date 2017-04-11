#/bin/bash

# Import shared constants
source constants.sh;
source helpers.sh;

# Run container in interactive mode
docker run --interactive                                \
           --tty                                        \
           --volume $(volumiser $POP_PYTHON_DIR)        \
           --volume $(volumiser $PROJECT_SOURCES)       \
           --volume $(volumiser $PROJECT_BUILD_DIR)     \
           --volume $(volumiser $PROJECT_ENTRY_POINT)   \
           --name "$DOCKER_PYTHON_CONTAINER_NAME"       \
           "$DOCKER_PYTHON_IMAGE_NAME"                  \
           /bin/bash;

# Clean up
docker stop "$DOCKER_PYTHON_CONTAINER_NAME";
docker rm "$DOCKER_PYTHON_CONTAINER_NAME";
