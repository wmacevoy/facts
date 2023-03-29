FROM alpine:latest

RUN apk add --no-cache \
	bash \
	build-base \
	gdb \
	cmake \
	&& rm -rf /var/cache/apk/*

WORKDIR /app

RUN echo  export PATH='"'/app/bin:/app/bin/$(uname -s)-$(uname -m)':$PATH"' >> /root/.bashrc
ENTRYPOINT ["/bin/bash","-i","-c","\"$@\"","--"]

