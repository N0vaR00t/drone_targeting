FROM debian:12-slim
WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake \
    && rm -rf /var/lib/apt/lists/*

COPY CMakeLists.txt main.cpp library.cpp library.h provided_coverage.h ./
COPY input/ input/
COPY run-all.sh .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel \
    && chmod +x run-all.sh

CMD ["sh", "-c", "./run-all.sh && echo '--- RESULTS ---' && for f in output/*.txt; do if [ -f \"$f\" ]; then echo \"--- $f\"; cat \"$f\"; fi; done"]
