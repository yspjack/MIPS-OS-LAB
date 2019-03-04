#!/bin/bash

SRCFILE=$1
OUTFILE=$2


sed -n '8p' $SRCFILE > $OUTFILE
sed -n '32p' $SRCFILE >> $OUTFILE
sed -n '128p' $SRCFILE >> $OUTFILE
sed -n '512p' $SRCFILE >> $OUTFILE
sed -n '1024p' $SRCFILE >> $OUTFILE
