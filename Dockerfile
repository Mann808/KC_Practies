FROM ubuntu:22.04

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN dpkg --add-architecture i386 && \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    wget \
    software-properties-common \
    apt-transport-https \
    winbind \
    xvfb \
    x11vnc \
    fluxbox \
    xdg-utils \
    cabextract \
    unzip \
    dbus-x11 \
    metacity \
    xterm \
    && rm -rf /var/lib/apt/lists/*

RUN wget -nc https://dl.winehq.org/wine-builds/winehq.key && \
    apt-key add winehq.key && \
    add-apt-repository 'deb https://dl.winehq.org/wine-builds/ubuntu/ jammy main' && \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --install-recommends winehq-stable

RUN useradd -m wineuser && \
    mkdir -p /app && \
    chown -R wineuser:wineuser /app

USER wineuser

ENV HOME=/home/wineuser \
    WINEPREFIX=/home/wineuser/.wine \
    WINEARCH=win32 \
    DISPLAY=:0

RUN wineboot --init && \
    sleep 10

WORKDIR /home/wineuser
RUN wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks && \
    chmod +x winetricks

RUN xvfb-run ./winetricks -q dotnet48

WORKDIR /app

COPY --chown=wineuser:wineuser ./app/ /app/

RUN echo '#!/bin/bash\n\
Xvfb :0 -screen 0 1024x768x24 &\n\
sleep 1\n\
metacity --replace &\n\
x11vnc -display :0 -forever -passwd password123 &\n\
sleep 2\n\
WINEDEBUG=-all wine SimpleWpfApp.exe' > /app/start.sh && \
    chmod +x /app/start.sh

EXPOSE 5900

CMD ["/app/start.sh"]