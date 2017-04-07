#/bin/bash

# Import shared constants
source constants.sh;

# Build directory mount point in the container
DIR="$(pwd)/$PROJECT_BUILD_DIR:/$DOCKER_WORKING_DIR/$PROJECT_BUILD_DIR";

# Run container in interactive mode
docker run --interactive                            \
           --tty                                    \
           --volume "$DIR"                          \
           --name "$DOCKER_PYTHON_CONTAINER_NAME"   \
           "$DOCKER_PYTHON_IMAGE_NAME"              \
           /bin/bash;

# Clean up
docker stop "$DOCKER_PYTHON_CONTAINER_NAME";
docker rm "$DOCKER_PYTHON_CONTAINER_NAME";
