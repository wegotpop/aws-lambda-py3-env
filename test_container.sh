#/bin/bash

# Import shared constants
source constants.sh;

# Run container in interactive mode
docker run --interactive                                            \
           --tty                                                    \
           --volume "$(pwd)/$PROJECT_DIRECTORY:/$PROJECT_DIRECTORY" \
           --name $DOCKER_PYTHON_CONTAINER_NAME                     \
           "$DOCKER_PYTHON_IMAGE_NAME"                              \
           /bin/bash;

# Clean up
docker stop "$DOCKER_PYTHON_CONTAINER_NAME";
docker rm "$DOCKER_PYTHON_CONTAINER_NAME";
