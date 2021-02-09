FROM debian

WORKDIR /app

RUN apt-get update && apt-get -y install openssl libc6 libgcc1 libstdc++6 libuuid1 procps gpm build-essential

COPY ./ /app

RUN chmod +x drivers/vpn/linux/compile.cmd

RUN cd drivers/vpn/linux && ./compile.cmd

# CMD ["/bin/bash"]
