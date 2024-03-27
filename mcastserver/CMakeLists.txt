cmake_minimum_required(VERSION 3.10)

project(mcastserver VERSION 1.0 LANGUAGES C)

add_executable(mcastserver
    main.c
    Server.c
    ServerConfiguration.c
    NetworkInterface.c
    MulticastListener.c
    MessageQueue.c
    Sender.c)
