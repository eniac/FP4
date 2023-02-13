#!/bin/bash
x=''
for j in `seq ${2}`
do
	x+=`printf %01448d $n`
done

for i in `seq ${1}`
do
  echo $x
  sleep 1
done
