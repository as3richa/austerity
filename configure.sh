#!/usr/bin/env sh

set -eu

rm -rf build config.in
mkdir build

SOURCES="$(echo src/*.c)"
DEPFILES=""
OBJECTS=""

FMT='%s: %s\n\t$(CC) -c $(CFLAGS) -MMD -MF %s -o %s %s\ninclude %s\n\n'

for SOURCE in $SOURCES; do
  OBJECT="$(echo $SOURCE | sed 's/^src\/\(.*\).c$/build\/\1.o/g')"
  DEPFILE="$(echo $SOURCE | sed 's/^src\/\(.*\).c$/build\/\1.in/g')"

  if [ -z "$OBJECTS" ]; then
    OBJECTS="$OBJECT"
    DEPFILES="$DEPFILE"
  else
    OBJECTS="$OBJECTS $OBJECT"
    DEPFILES="$DEPFILES $DEPFILE"
  fi

  printf "$FMT" $OBJECT $SOURCE $DEPFILE $OBJECT $SOURCE $DEPFILE >> config.in
  touch "$DEPFILE"
done

echo "OBJECTS = $OBJECTS" >> config.in
echo "DEPFILES = $DEPFILES" >> config.in
