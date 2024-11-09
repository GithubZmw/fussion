### Introduction

This code implements a threshold signature scheme with a customizable threshold, involving four main entities: **TA (Trusted Authority)**, **Signer**, **Aggregator**, and **Verifier**. The system operates as follows:

- **TA (Trusted Authority)**: Issues share reconstruction keys to each signer.
- **Verifier**: Specifies a threshold \( t \), after which signers use their share reconstruction keys to generate corresponding shares and produce partial signatures.
- **Signers**: Generate partial signatures based on their share reconstruction keys and the threshold specified by the verifier, then send these partial signatures to the aggregator.
- **Aggregator**: Collects the partial signatures from signers, aggregates them into a threshold signature, and sends it to the verifier.
- **Verifier**: Uses its private key to verify the aggregated threshold signature.

### Note
This code uses the **brpc** library for communication, so users need to download and compile the **brpc** library themselves and place the resulting `.a` static library file in the `libs` folder within the project. The compilation steps are as follows:

#### Install Dependencies
```shell
sudo apt-get install -y git g++ make libssl-dev libgflags-dev libprotobuf-dev libprotoc-dev protobuf-compiler libleveldb-dev
```

#### Compile brpc with `config_brpc.sh`
Clone the brpc repository, navigate to the project directory, and run:
```shell
$ sh config_brpc.sh --headers=/usr/include --libs=/usr/lib
$ make
```
After compiling, an `output` folder will be created in the current directory. Copy `output/lib/libbrpc.a` to the `libs` folder in the project directory.

### Example Execution

The project includes a simple example, which you can run with the following commands:

```shell
$ cd example/echo_c++
$ make
$ ./echo_server &
$ ./echo_client
```

### More Information
For further details on brpc, please refer to the [brpc documentation](https://github.com/apache/brpc/blob/master/docs/cn/getting_started.md).

--- 

You can directly use this as your README file content on GitHub.
