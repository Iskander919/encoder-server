# Используем образ Ubuntu
FROM ubuntu:24.04

# Установка компонентов
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y build-essential \
    cmake \
    qt6-base-dev \
    libqt6sql6-psql \
    qt6-httpserver-dev \
    qt6-tools-dev \
    qt6-websockets-dev \
    libssl-dev \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# Настройка рабочей директории
WORKDIR /app	

# Копирование исходников
COPY . .

# Сборка проекта через CMAKE
# RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release || (cat build/CMakeFiles/CMakeError.log && exit 1) && \
#    cmake --build build --verbose
RUN mkdir -p build && \ 
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Запуск сервера
ENV QT_QPA_PLATFORM=minimal
CMD [./build/security-server]

COPY ./config.ini ./build/config.ini