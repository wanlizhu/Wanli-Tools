#!/bin/bash

if [[ $UID -ne 0 ]] && ! sudo grep -q "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers; then
    echo "Setting up NoPasswd sudo for $USER"
    echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers
fi 