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
#   03 Jan. 2016 - RB - added multi-file and multi-compiler (8-bit only)
#   12 Sep. 2016 - RB - added 32-bit support
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
RM='rm -rf'
#HOMEDIR=`pwd`
#LOGDIR='log'
PINGUINO='../pinguino-ide/pinguino/pinguino_cmd.py'

CMP8LIST=('--xc8' '--sdcc')
BRD8LIST=('-p1459' '-c1708' '-p13k50' '-p14k50' '-p2455' '-p4455' '-p2550' '-p4550' '-p25k50' '-p45k50' '-p26j50' '-p46j50' '-p27j53' '-p47j53')
BRD32LIST=('-p220' '-p250' '-p270' '--olimex220' '--olimex440' '--olimex440OTG' '--olimex440Micro' '--olimexT795' '--emperor460' '--emperor795' '--ubw460' '--ubw795')

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
    printf '%s\r\n' '$RED WARNING : missing argument'
    printf '%s\r\n' '$RED Usage : ./regression.sh /directory'
    printf '%s\r\n' '$YELLOW We use examples per default'
    TARGET='examples'
else
    TARGET="$1"
fi


# create an empty log dir.
#if [ -d $LOGDIR ]; then
#    ${RM} ${LOGDIR}/*
#else
#    mkdir -p ${LOGDIR}
#fi
${RM} regression.log

# FILE  : .pde file name with path
# FNAME : .pde file name without path
# redirection  2> ${LOGDIR}/${FNAME}.log
find ${TARGET} -type f -name *.pde | while read FILE ; do
    FNAME=$(basename "${FILE}" .pde)
    printf '%s\n' "$GREEN Compiling $FNAME ..."
    printf '%s\n' "Compiling $FNAME ..." >> regression.log

    # 8-bit
    for board in "${BRD8LIST[@]}"; do
        printf '%-40s' "$YELLOW $board"
        printf '%-40s' "$board" >> regression.log
        for compiler in "${CMP8LIST[@]}"; do
            OUTPUT=`python $PINGUINO $compiler $board -f $FILE`
            END=${OUTPUT:(-2)}
            if [ "$END" != "OK" ]; then
                printf '%s\t' "$RED $compiler"
                printf '%s=fa\t' "$compiler" >> regression.log
            else
                printf '%s\t' "$GREEN $compiler"
                printf '%s=ok\t' "$compiler" >> regression.log
            fi
        done
        printf "$NORMAL\n"
        printf '\n' >> regression.log
    done

    # 32-bit
    for board in "${BRD32LIST[@]}"; do
        printf '%-40s' "$YELLOW $board"
        printf '%-40s' "$board" >> regression.log
        OUTPUT=`python $PINGUINO $board -f $FILE`
        END=${OUTPUT:(-2)}
        if [ "$END" != "OK" ]; then
            printf '%s\t' "$RED --p32"
            printf '%s=fa\t' "--p32" >> regression.log
        else
            printf '%s\t' "$GREEN --p32"
            printf '%s=ok\t' "--p32" >> regression.log
        fi
        printf "$NORMAL\n"
        printf '\n' >> regression.log
    done

done
