FROM alpine:latest
RUN apk update && apk add --no-cache gcc musl-dev make
WORKDIR /app
COPY . .
RUN make test
CMD ["./uds_stack_demo"]
