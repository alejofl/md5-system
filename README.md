# MD5 Computing System

## Introduction

In order to internalize the inter process communication mechanisms of Linux, a system that calculates the MD5 hash of files has been developed. Said system is composed by three executables:

- `md5`: the main program. Receives the files to analyse as arguments and delegates its analysis to child processes. Lastly, it receives the results and stores them in a file.
- `slave`: the program that calculates the MD5 hash of the file received by standard input. This executable works independently of the rest of the system.
- `view`: the program that, if it's running, prints to standard output the results of the rest of the system.

## Prerequisites

To be able to compile this project, the following binaries are required:
- Docker
- gcc
- make

</aside>

## Compiling

The first time only, download the Docker image:

```bash
docker pull agodio/itba-so:1.0
```

Then, every time that you want to compile, execute the following command in the project directory:

```bash
docker run -v ${PWD}:/root --privileged -ti agodio/itba-so:1.0
```

The Docker container will start. Then, execute the following commands:

```bash
cd root
make all
```

## Executing

It's recommended to execute the system inside the Docker image, to obtain the optimum results. Thus, the following instructions assume Docker is being executed and that the current working directory is the project's.

### Option 1 (no stdout printing)

Simply execute the following in a terminal:

```bash
./md5 <path_list>
```

`<path_list>` must be a space-separated list of relative paths which points to the files that are going to be analyzed.

Results will be stored in `results.txt`.

### Option 2 (piping)

Execute the following command in a terminal:

```bash
./md5 <path_list> | ./view
```

`<path_list>` must be a space-separated list of relative paths which points to the files that are going to be analyzed.

Results will be stored in `results.txt`. Moreover, results will also be printed to standard output

### Option 3 (two terminals)

Two instances of the same Docker image are required:

1. In a terminal (*Terminal 1*), open a Docker instance as explained before.
2. In a new terminal (*Terminal 2*), run `docker ps`.
3. Copy the *Container ID*. It will be used in the next step.
4. Run the following command:
    
    ```bash
    docker exec -ti <container_id> bash
    ```
    
5. Change directory to the project's

Now, execute the following in *Terminal 1:*

```bash
./md5 <path_list>
```

`<path_list>` must be a space-separated list of relative paths which points to the files that are going to be analyzed.

The amount of files to analyze will be printed in standard output.

Then, in *Terminal 2*, run the following line:

```bash
./view <file_qty>
```

Results will be stored in `results.txt`. Moreover, results will also be printed to standard output of *Terminal 2*

## Memory Leaks and Code Quality check

> 🚧 Warning
>
>  Two extra binaries are required for this part:
>   - Valgrind
>   - PVS-Studio

### Valgrind

Run the following commands in a terminal:

```bash
make all
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	./md5 <lista_de_paths> | ./view
```

`<path_list>` must be a space-separated list of relative paths which points to the files that are going to be analyzed.

Check results will be printed.

### PVS-Studio

Run the following:

```bash
make check-pvs
```

Check results will be stored in `report.tasks`

## Resource cleanup

Files created by the system can be removed:

- To remove the executables and result files, `make clean`
- To remove PVS-Studio check files, `make clean-pvs`
- To remove all of the files created by the system, `make clean-all`