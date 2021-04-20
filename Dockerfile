FROM ubuntu:18.04
RUN mkdir /usr/src/ICN
RUN sed -i s:/archive.ubuntu.com:/mirrors.tuna.tsinghua.edu.cn/ubuntu:g /etc/apt/sources.list
RUN cat /etc/apt/sources.list
RUN apt-get clean
RUN apt-get update --fix-missing && apt-get install -y make gcc build-essential libjsoncpp-dev iproute2 iproute2-doc
COPY ./ /usr/src/ICN

WORKDIR /usr/src/ICN

RUN make
CMD ["/usr/src/ICN/ICNNode"]