# Server Client TCP examples and simple framework (serclicpp)

## What is this

An attempt to incrementally "bit by bit" build a minimal c++ networking framework for playing around with linux networking.

Originally based on some [code I wrote earlier](https://gitlab.com/thosteg/tcp_client_server), which was largely based on `man epoll` and some networking examples on the internet.

## Goals and Non-Goals

Main goal:

Make it simple to play with c-networking under linux/unix.

Goals:

* Make it simple to get started with basic c-networking
* Make it simple to play around with basic c-networking (blocking, non-blocking, select...)
* Do not obscure underlying functions and interfaces

Non-Goals (Do not do):

* Build a real-world networking library (catch errors other than via dying-guard)

## Requirements

Linux (due to usage of `epoll`).
On Debian, packages:

* `build-essential`  for building
* `psmisc`  for killall

## Usage

```
make      # shows available target
make run  # builds and runs example
```

## Examples desired

Examples desired:

* echo server + client which sends two requests
* proxy which forwards all connections to a given server

Options:

* multiple / single tcp client
