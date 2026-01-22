#!/bin/bash

sudo sysctl -w net.core.rmem_max=212992
sudo sysctl -w net.core.rmem_default=212992
