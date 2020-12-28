# Build container: docker build . -t almond
# Run Almond: docker run -it --rm --net=host -v /root/.Xauthority:/root/.Xauthority --env="DISPLAY" almond

FROM ubuntu:20.04

ARG THREADS=4

ENV DEBIAN_FRONTEND noninteractive

ENV SOURCE_DIR /opt/Almond
ENV VCPKG_DIR /opt/vcpkg
ENV VCPKG_CMAKE_TOOLCHAIN ${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake

RUN apt-get update
RUN apt-get install -y \
                    apt-utils \
                    tzdata \
                    cmake \
                    make \
                    git \
                    curl \
                    unzip \
                    tar \
                    libx11-dev \
                    libglu1-mesa-dev \
                    libxi-dev \
                    libxext-dev \
                    xorg

# install dependencies using package manager
RUN apt-get install -y \
                    qt5-default \
                    qttools5-dev \
                    libavcodec-dev \
                    libavdevice-dev \
                    libavfilter-dev \
                    libavformat-dev \
                    libavutil-dev \
                    libswscale-dev \
                    pkg-config \
                    libboost-dev \
                    ocl-icd-opencl-dev \
                    zlib1g-dev \
                    libpng-dev \
                    libjpeg-dev \
                    libpng++-dev

ENV CMAKE_MAKE_PROGRAM make

# use clang
# RUN apt-get install -y clang llvm
# ENV CMAKE_C_COMPILER clang
# ENV CMAKE_CXX_COMPILER clang++

# use gcc
RUN apt-get install -y gcc g++
ENV CMAKE_C_COMPILER gcc
ENV CMAKE_CXX_COMPILER g++

# Use vcpkg, takes a long time
# Also it gives following errow when installing qt5: Failed. Status: 35;"SSL connect error"
#
# RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_DIR}
# WORKDIR ${VCPKG_DIR}
#
# RUN ./bootstrap-vcpkg.sh
# RUN ./vcpkg integrate install
# RUN ./vcpkg install qt5:x64-linux
# RUN ./vcpkg install boost:x64-linux
# RUN ./vcpkg install ffmpeg:x64-linux
# RUN ./vcpkg install opencl:x64-linux

WORKDIR ${SOURCE_DIR}
COPY . ${SOURCE_DIR}

ENV RELEASE_DIR ${SOURCE_DIR}/build_release

RUN mkdir ${RELEASE_DIR}
WORKDIR ${RELEASE_DIR}

RUN cmake .. \
            -DCMAKE_BUILD_TYPE=Release \
            # -DCMAKE_TOOLCHAIN_FILE=${VCPKG_CMAKE_TOOLCHAIN} \
            -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} \
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} \
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}

RUN make -j ${THREADS}

RUN touch ~/.Xauthority
RUN xauth add :0 . `mcookie`

CMD ./almond
