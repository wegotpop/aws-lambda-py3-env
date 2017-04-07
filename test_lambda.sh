# Import shared constants
source constants.sh;

DOCKER_LAMBDA_IMAGE_NAME='fake-aws-lambda'

# Test archived project in 'fake' AWS Lambda environment
docker run --volume "$(pwd)/$PROJECT_LOCAL_DIR":/var/task   \
           --name $DOCKER_LAMBDA_IMAGE_NAME                 \
           lambci/lambda:python2.7

# Clean up
docker stop "$DOCKER_LAMBDA_IMAGE_NAME";
docker rm "$DOCKER_LAMBDA_IMAGE_NAME";
