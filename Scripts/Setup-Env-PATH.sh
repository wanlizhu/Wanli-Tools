#!/bin/bash

if [ ! -f ~/.bashrc ] || ! grep -q "WZhu Env PATH" ~/.bashrc; then
cat << "END_OF_FILE" >> ~/.bashrc

# === WZhu Env PATH ===
export PATH="$PATH:$HOME/WZhu/Scripts"
END_OF_FILE
fi 