FROM ubuntu:latest
RUN apt update && apt install -y gcc make git binutils libc6-dev gdb sudo clang-format
RUN git clone https://github.com/masaxsuzu/dotfiles.git ~/dotfiles && bash ~/dotfiles/deploy.sh