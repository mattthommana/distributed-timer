FROM ubuntu:22.04 as base

# Set environment variables to non-interactive (this prevents some prompts)
ENV DEBIAN_FRONTEND=noninteractive

ARG UID
ARG GID

# Install necessary packages
RUN apt-get update && apt-get install -y \
    ca-certificates

# Copy all certificate files from the 'docker/certs/' directory
COPY docker/certs/* /usr/local/share/ca-certificates/

# Update CA certificates
RUN update-ca-certificates

RUN apt-get update && apt-get install -y \
    wget \
    cmake \
    build-essential \
    libssl-dev \
    autoconf \
    automake \
    libtool \
    openssh-client \
    openssh-server \
    sudo \
    netcat \
    iputils-ping \
    net-tools \
    && rm -rf /var/lib/apt/lists/*


ENV OPENSSL_ROOT_DIR=/usr/include/openssl
ADD docker/install-curl.sh install-curl.sh
RUN bash ./install-curl.sh && rm install-curl.sh

ADD docker/install-git.sh install-git.sh
RUN bash ./install-git.sh && rm install-git.sh

ADD docker/install-cmake.sh install-cmake.sh
RUN bash ./install-cmake.sh && rm install-cmake.sh

# Set the working directory inside the container
WORKDIR /workspace

# Copy the current directory contents into the container at /workspace
COPY . /workspace/
RUN rm -rf build cache
RUN groupadd -g ${GID} user && \
    useradd -u ${UID} -g ${GID} -ms /bin/bash user && \
    usermod -aG sudo user && \
    chown -R user:user /workspace/ && \
    echo "user ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

COPY docker/.bashrc /home/user/.bashrc
USER user

# Compile using CMake
RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make

RUN sudo chown -R $(id -u):$(id -g) /workspace/logs
