# # Use Ubuntu 16.04 (Xenial)
# FROM ubuntu:16.04

# # Set a non-interactive frontend for apt
# ENV DEBIAN_FRONTEND=noninteractive

# # 1. Install base build tools and Java 8
# RUN apt-get update && \
#     apt-get install -y \
#         software-properties-common \
#         wget curl git make gcc g++ unzip vim \
#         && add-apt-repository ppa:openjdk-r/ppa -y && \
#     apt-get update && \
#     apt-get install -y openjdk-8-jdk && \
#     rm -rf /var/lib/apt/lists/*

# # Set JAVA_HOME for convenience
# ENV JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
# ENV PATH=$JAVA_HOME/bin:$PATH

# # 2. Install your project-specific dependencies here
# # (replace these examples with what your project actually needs)
# RUN apt-get update && \
#     apt-get install -y \
#         libgmp3-dev \
#         libmpfr-dev \
#         libmpfr-doc \
#         libmpfr4 \
#         libmpfr4-dbg \
#         gsl-bin \
#         libgsl0-dev \
#         bison \
#         flex \
#         gnuplot-x11 \
#         libglpk-dev \
#         libyaml-cpp-dev \
#         && rm -rf /var/lib/apt/lists/*

# # Copy dependency list and project files
# WORKDIR /app
# COPY . /app

# # Run make in each subdirectory
# RUN cd flowstar && make && \
#     cd ../verisig-src && ./gradlew installDist

# # Default command
# CMD ["/bin/bash"]


# Use Ubuntu 16.04 (Xenial)
FROM ubuntu:16.04

ENV DEBIAN_FRONTEND=noninteractive

# 1. Install base build tools and Java 8
RUN apt-get update && \
    apt-get install -y \
        software-properties-common \
        wget curl git make gcc g++ unzip vim \
        && add-apt-repository ppa:openjdk-r/ppa -y && \
    apt-get update && \
    apt-get install -y openjdk-8-jdk && \
    rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
ENV PATH=$JAVA_HOME/bin:$PATH

# 2. Install project dependencies
RUN apt-get update && \
    apt-get install -y \
        libgmp3-dev \
        libmpfr-dev \
        libmpfr-doc \
        libmpfr4 \
        libmpfr4-dbg \
        gsl-bin \
        libgsl0-dev \
        bison \
        flex \
        gnuplot-x11 \
        libglpk-dev \
        libyaml-cpp-dev \
        && rm -rf /var/lib/apt/lists/*

# Create work directory but do NOT copy code
WORKDIR /app

# Default command
CMD ["/bin/bash"]


