#!/bin/sh

cd $(dirname $0)
rm -r html
git checkout develop -- html
b2 -a fully-standalone
git add -A html
