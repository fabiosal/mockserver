# MOCKSERVER
The objective of this project is to create a simple way to mock api calls locally in order to enable testing and developing locally without the need to call the real API, either because the API is not reachable or it doesn't exists yet.

## Install
Run `make mockserver` to compile the code.  

## Run
Move the executable to you path or run `./mockserver`  
Without arguments the mock server will be bound to address 127.0.0.1 and port 9999  
otherwise you can specify address and port running the program like `./mockserver 127.0.0.1 6666`

## How it works
When the Mockserver receive a request ie "http://127.0.0.1:9999/test"
It will understand the `request-method` (ie GET) and the `request-target`  (/test) and search for the response in file : `$HOME/mockserver/[request-target]/[request-method]`   
so in this example case `$HOME/mockserver/test/GET` 

The file should contain a valid http message like.
```
HTTP/1.1 200 OK
Content-Type: application/json

{"key": "value"}

```



In case the request  
- contains an id "http://127.0.0.1:9999/test/123"
- or a uuid "http://127.0.0.1:9999/test/641ad3e7-5e25-4e63-9b15-c8f4f9afce56"
it will be mapped on the '\_' directory, so the response file for the above examples will be searched in  `$HOME/mockserver/test/_/GET` file


## Improvement
Make it work for https







