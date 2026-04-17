# Mock Swish payment request

> [!NOTE]
> This application is designed for a certain purpose but has support for the endpoints below.

## Requirements

- Docker
- nlohmann-json (installed in Docker)
- curl
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)

## Description 

Container should serve as a mock server for the possibility of making a payment request.
This is all for personal educational purposes.
Used with [pay-per-weigh](https://github.com/bubba-94/pay-per-weigh) application.

## Building

### Docker container 

Image on DockerHub([link](https://hub.docker.com/r/moodin/ppw-server)).
- Builds binary in the container.

1. Build container: `docker build -t ppw-server .`
2. Open container: `docker run -p 8443:8443 --name ppw-server -it ppw-server:latest /bin/sh`
3. Inside container shell: `./ppw-server`

_____

## Building binary

Right now the Makefile is building for aarch64 (Raspberry Pi).
- Both application and server running on Pi.
- 1. `make` in root repo to compile binary.
- 2. `./ppw-server` from root repo to run server.
- 3. Requires a test file in root repo named: `payment.json`.

## Testing with curl

- Container binds to 0.0.0.0 and port 8443
- Example URL: localhost:8443/hi.

Example flow:
```bash

# Welcome message
curl -X GET localhost:8443/hi

# Get all valid ids
curl -X GET localhost:8443/allPaymentRequests

# Get status of certain id
curl -X GET localhost:8443/paymentRequest/:id

# Patch a valid payment status
curl -X PATCH localhost:8443/paymentRequest/:id \
-H "Content-Type: application/json" \
-d '{"status": "VALID"}' # value == VALID, PENDING, DECLINED, ERROR, CANCELLED

# Post new test payment
curl -X POST http://localhost:8443/paymentRequest \
-H "Content-Type: application/json" \
-d '{
    "amount": 20,
    "payeeAlias": "liatest",
    "currency": "SEK",
    "callbackUrl": "http://localhost:8443/callback",
    "message": "TEST"
}'
```
