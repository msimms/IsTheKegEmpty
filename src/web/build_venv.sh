#!/bin/bash

# Create the virtual environment.
python3 -m venv venv

# Activate the virtual environment.
source venv/bin/activate

# Install the packages.
pip3 install -r requirements.txt
