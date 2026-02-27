# Build stage
FROM gcc:13 AS builder
RUN apt update && \ 
    apt install nlohmann-json3-dev
WORKDIR /app
COPY . .
RUN make

# Runtime stage (small image)
FROM debian:stable-slim
WORKDIR /app
COPY --from=builder /app/ppw-server .
EXPOSE 8443
CMD ["./ppw-server"]