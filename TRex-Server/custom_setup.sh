#!/usr/bin/env bash

echo $(make -j12 -C `pwd`/../TRex2-lib) | grep "Making all in src libtool" || : &&
        make -j12 -C `pwd`/../TRex-Server
