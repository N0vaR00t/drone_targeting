FROM debian:12-slim
WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ \
    && rm -rf /var/lib/apt/lists/*

COPY main.cpp .
COPY input/ input/
COPY run-all.sh .

RUN g++ -std=c++17 -O2 -Wall -Wextra -o drop_calc main.cpp \
    && chmod +x run-all.sh

CMD ["sh", "-c", "./run-all.sh && echo '--- RESULTS ---' && for f in output/*.txt; do if [ -f \"$f\" ]; then echo \"--- $f\"; cat \"$f\"; fi; done"]
