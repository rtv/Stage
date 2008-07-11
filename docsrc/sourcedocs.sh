#!/bin/bash

echo "/** \mainpage The Stage Robot Simulator"
./Markdown.pl ../README
echo "**/"

echo "/** \page install Installation"
./Markdown.pl ../INSTALL
echo "**/"

echo "/** \page release Release Notes"
./Markdown.pl ../RELEASE
echo "**/"
