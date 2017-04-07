# AWS Lambda Python 3 Deploy Environment

> **TODO:** Add **real** documentation here !!!!!

### Dependencies:

- `docker`

### Usage:

```bash
# Build everything
$ bash makebin.sh

# Skip `build` and `run` phases
$ bash makebin.sh --skip build --skip run

# Run lambda environment
$ bash test_lambda.sh
```

### For those who are developing the environment itself:

```bash
# After the `makebin.sh` get interactive shell into the building container
$ bash test_container.sh
```
