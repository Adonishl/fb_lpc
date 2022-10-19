# trans_func

This project we will implement a LPC(local procedure call) framework. LPC is a concept in WinOS, but I cannot find some for linux OS.
Indeed, those RPC(remote procedure call) can also be deployed in local envirionment, but its underground implementation is based on socket.
While shared memory is likely to be a more efficient way for IPC(Inter-process Communication), We'd like to replace that with it.
Typical RPC framework like gRPC is based on protobuf to define its protocol. By protoc toolkit, it will generate the code. However we cannot use
the code directly, cuz the automaticly generated code include the client/server side code. What we can use is the request/response data struct.
Furthermore, flatbuffer would be a better way while we just use the data-struct for SerDes. Then we use some other techniques for function/protocol registry.

## workflow

From client to server and then back to server, the basic workflow will be:
1. Serialize the input data. Using flatbuffer to serialize struct data to string buffer
2. Combine the input string data with lpc_service function(pre-registered func) to new buffer(namely func-input buffer)
3. Send func-input buffer to server by shared memory
4. Server get the func label and input data to generate server-side func
5. Servr run its func and produce the output
6. Serialize the output and send back
7. Client deserialize the output to struct data

Thus we need some classes like below:

Common Base:

* SharedMemory

Client Side:

* ClientBase

Server Side:

* ServerBase

## principles

On the client side, we use template tool(based on jinja2) to produce the code(both header and source codes) automaticly. It will registry a member function for CustomerClient(in source code derived from ClientBase). Then user can imediately use the class objects and their member functions.

On the server side, template tool will help to produce the header file. User need to implement the detail source code.

So our implementation will based on:
1. shared memory communication
2. derive class
3. template tool jinjia2
4. serialize/deserialize framework flatbuffer

## directories

```
|--lpc            // source code
   |--templates   // code templates(will be used by jinjia2)
|--tools          // tools & scripts
|--example        // simple examples for lpc
   |--proto       // protocol definition for example
   |--test_simple_example_server.cc
   |--test_simple_example_client.cc
```

## protocol definition format

Like gRPC protobuf service define, our protocol definition is simple. You can refer to ```example/proto/simple_example_service.pc```

syntax

* **import** indicates the flatbuffer file which will be included
* **service** indicates the latter description in brace is a kind of service. 
* **returns** input& output for service is seperated by returns. both input and output is described as ```name (data_type)```

## roadmap

The simplest version of this will only implement 1:1 client&server synchronized LPC, there should be a lot of limits. In the future, we will implement:

* m:n Clients/Servers
* async mode
* stream mode
