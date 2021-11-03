#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Oct  1 20:55:09 2021

@author: robertamarinei
"""


import glob

import re
numbers = re.compile(r'(\d+)')
def numericalSort(value):
    parts = numbers.split(value)
    parts[1::2] = map(int, parts[1::2])
    return parts

path =  "excel_acq/ScopeSet_1/1400TPC_017_noDrift/"
read_files = sorted(glob.glob(path+"*.txt"),key=numericalSort)


with open(path+"result.txt", "wb") as outfile:
    for f in read_files:
        with open(f, "rb") as infile:  
            outfile.write(infile.read())
            outfile.write("\n".encode())
            outfile.write(infile.read())
    

  
            
            
            
    
"""
this next part deletes the empty lines from the output file
"""    
with open(path+"result.txt", 'r+') as fd:
    lines = fd.readlines()
    fd.seek(0)
    fd.writelines(line for line in lines if line.strip())
    fd.truncate()