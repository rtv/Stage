#!/bin/bash

echo "/** \mainpage The Stage Robot Simulator"
./Markdown.pl ../README.txt
echo "**/"

echo "/** \page install Installation"
./Markdown.pl ../INSTALL.txt
echo "**/"

echo "/** \page release Release Notes"
./Markdown.pl ../RELEASE.txt
echo "**/"

echo "/** \page authors Authors"
./Markdown.pl ../AUTHORS.txt
echo "**/"
