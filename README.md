# Server Client TCP examples and simple framework (serclicpp)

## What is this

An attempt to incrementally "bit by bit" build a minimal c++ networking framework for playing around with linux networking.

Originally based on some [code I wrote earlier](https://gitlab.com/thosteg/tcp_client_server), which was largely based on `man epoll` and some networking examples on the internet.

## Goals and Non-Goals

Goals:

* Build a simple (not excessive) framework to allow me easily and simply fool around with linux networking primitives using c++

Non-Goals (Do not do):

* Boilerplate that abstracts away from basic concepts such as fd, socket

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
