FROM node:18-bullseye

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    				   build-essential cmake curl \
                       emacs-nox gdb git nano octave \
                       python3 time wget vim \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
EXPOSE 8000

RUN echo  export PATH='"'/app/bin:/app/bin/$(uname -s)-$(uname -m):/app/node_modules/.bin:/app:'${PATH}''"' >> /root/.bashrc
ENTRYPOINT ["/bin/bash","-i","-c","\"$@\"","--"]