# Mock Swish payment request

> ![NOTE]
> Still under construction

## Requires

- Docker
- nlohmann-json (installed in Docker)
- curl

## Description 

Container should serve as a mock server for the possibility of making a payment request.
This is all for personal educational purposes.
Used with [pay-per-weigh](https://github.com/bubba-94/pay-per-weigh) application.

## Building container 

Image will be available on DockerHub([link here]()) when ready.


1. Build container: `docker build -t ppw-server .`
2. Open container: `docker run -p 8443:8443 --name ppw-server -it ppw-server:v0.1 /bin/sh`
3. Inside container shell: `./ppw-server`

## Testing with curl

Container binds to 0.0.0.0 and port 8443
Example URL: localhost/welcome.
```
curl -X GET `localhost:8443/welcome`