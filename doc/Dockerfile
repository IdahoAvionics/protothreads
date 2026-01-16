FROM texlive/texlive:latest-small

RUN apt update && \
    apt install doxygen -qy --no-install-recommends && \
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /var/cache/apt/

RUN tlmgr install \
    float \
    varwidth \
    xcolor \
    xltabular \
    ltablex \
    tabularray \
    ninecolors \
    fancyvrb \
    multirow \
    hanging \
    adjustbox \
    stackengine \
    enumitem \
    alphalph \
    ulem \
    times \
    courier
