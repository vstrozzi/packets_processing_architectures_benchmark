#!/bin/bash

mv *.txt 1.txt

mv ../*.txt .

head -n 32768 1.txt >> *proc.txt

rm 1.txt
