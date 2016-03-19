#!/bin/bash

git add -A
printf 'commit msg: '
read message
git commit -m "$message"
git push origin master
