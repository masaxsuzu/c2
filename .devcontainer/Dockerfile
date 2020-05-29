FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y gcc make git binutils libc6-dev gdb sudo
RUN git clone https://github.com/masaxsuzu/dotfiles.git ~/dotfiles && bash ~/dotfiles/deploy.sh