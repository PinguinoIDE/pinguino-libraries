#!/bin/bash
#
#   --------------------------------------------------------------------
#   file:     regression.sh
#   desc.:    Test all pde. files
#   projects: Pinguino 
#   author:   Régis Blanchot
#   usage:    ./regression.sh directory-to-scan
#   --------------------------------------------------------------------
#   CHANGELOG:
#   09 Nov. 2012 - RB - first release
#   26 Jan. 2016 - RB - updated to use with Pinguino IDE v12
#   03 Jan. 2016 - RB - multi-file, multi-compiler tests
#   --------------------------------------------------------------------
#   This is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#   --------------------------------------------------------------------

# delete command
RM="rm -rf"
HOMEDIR=`pwd`
LOGDIR="log"
PINGUINO="../pinguino-ide/pinguino/pinguino_cmd.py"

COM8LIST=("--xc8" "--sdcc")
CPU8LIST=("-p1459" "-c1708" "-p2455" "-p4455" "-p2550" "-p4550" "-p25k50" "-p45k50" "-p26j50" "-p46j50" "-p27j53" "-p47j53")

# colors
#RED='\e[31;1m>'
#GREEN='\e[32;1m>'
#YELLOW='\e[33;1m'
#TERM='\e[m'
BLACK=$(tput setaf 0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
LIME_YELLOW=$(tput setaf 190)
POWDER_BLUE=$(tput setaf 153)
BLUE=$(tput setaf 4)
MAGENTA=$(tput setaf 5)
CYAN=$(tput setaf 6)
WHITE=$(tput setaf 7)
BRIGHT=$(tput bold)
NORMAL=$(tput sgr0)
BLINK=$(tput blink)
REVERSE=$(tput smso)
UNDERLINE=$(tput smul)

# test arguments
if [ "$1" == "" ]; then
    printf "%s\r\n" "$RED WARNING : missing argument"
    printf "%s\r\n" "$RED Usage : ./regression.sh /directory"
    printf "%s\r\n" "$YELLOW We use examples per default"
    TARGET="examples"
else
    TARGET="$1"
fi


# create an empty log dir.
if [ -d $LOGDIR ]; then
    ${RM} ${LOGDIR}/*
else
    mkdir -p ${LOGDIR}
fi

# FILE  : .pde file name with path
# FNAME : .pde file name without path
find ${TARGET} -type f -name *.pde | while read FILE ; do
    FNAME=$(basename "${FILE}" .pde)
    printf "%s\n" "$GREEN Compiling $FNAME ..."
    for cpu in "${CPU8LIST[@]}"; do
        printf "%-20s" "$YELLOW $cpu"
        for cc in "${COM8LIST[@]}"; do
            OUTPUT=`python $PINGUINO $cc $cpu -f $FILE 2> ${LOGDIR}/${FNAME}.log`
            END=${OUTPUT:(-2)}
            if [ "$END" != "OK" ]; then
                printf "%s\t" "$RED $cc"
                #echo ${OUTPUT} >> "${LOGDIR}/${FNAME}.log"
                #echo -e '\t' $YELLOW "See ${LOGDIR}/${FNAME}.log" $TERM
            else
                printf "%s\t" "$GREEN $cc"
            fi
        done
        printf "$NORMAL\n"
    done
done
