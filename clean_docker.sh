#/bin/bash

docker ps -a | cut -d ' ' -f 1 | xargs -n 1 docker rm;
docker volume rm `docker volume ls -q -f dangling=true`;
docker image rmi `docker image ls -q -f dangling=true`;
