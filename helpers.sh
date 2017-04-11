#/bin/bash

function volumiser()
{
    printf "$(pwd)/$1:/$DOCKER_WORKING_DIR/$1";
}
