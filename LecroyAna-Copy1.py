#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Oct  4 14:54:00 2021

@author: robertamarinei
"""


    

import numpy as np
import matplotlib.pyplot as plt
import matplotlib as matplotlib
from datetime import datetime
from scipy.stats import norm




#f  = open('caen_test.txt')
#f  = open('test_data.txt')
#path = 'excel_acq/1400PMT_017_noDrift/result.txt'
#path = 'excel_acq/1400PMT_017/result.txt'
path =  "excel_acq/ScopeSet_1/1400TPC_017/"
f = open(path+"result.txt",'r')
#f = open('CAENGECO2020_update.txt')
ampl_TPC = []
time_TPC = []

all_ampl_TPC = []
ampl_Scint = []
time_Scint = []
dum_wf_sum= 0
wf_sum_Scint = []
wf_sum_TPC = []
time_st = []
bin_info=[]
Int_aft = []
Int_bef = []

tot_evt = 200;


for i in range(tot_evt):
    ampl_TPC.append([])    
    time_TPC.append([])
    ampl_Scint.append([])
    time_Scint.append([])
    Int_bef.append(0)
    Int_aft.append(0)


#read data and store them in a list of list
dis = f.readlines()
f.close()
for i in dis:
        evt_, ch_, time_, ampl_ = i.split(',')
        if (ch_ == "C1"):
                ampl_TPC[int(evt_)].append(float(ampl_))
                time_TPC[int(evt_)].append(float(time_))
        elif (ch_ == "C2"): 
                ampl_Scint[int(evt_)].append(float(ampl_))   
                time_Scint[int(evt_)].append(float(time_))


for i in range(len(ampl_TPC[0])):
    wf_sum_Scint.append(0)
    wf_sum_TPC.append(0)



trigger_level = -0.017#volts

   
#    wf_sum_Scint.append(dum_wf_sum) 
#    wf_sum_TPC.append(dum_wf_sum_TPC)    
#    time_st.append(time_Scint[0][i])
            
fake_trigger = 0

 
for i in  range(len(ampl_TPC)):
        
    for j in range(len(ampl_TPC[0])):
        if (time_TPC[i][j]<0): Int_bef[i]+=ampl_TPC[i][j]
        elif (time_TPC[i][j]>0): Int_aft[i]+=ampl_TPC[i][j]

int_bef_hist = plt.hist(Int_bef)
plt.title("Integral before the trigger for all events")
plt.set_xlabel("Amplitude [V]")
int_aft_hist = plt.hist(Int_aft)
plt.title("Integral after the trigger for all events")
plt.set_xlabel("Amplitude [V]")

for i in  range(len(ampl_TPC)):
    res = True in (ele < trigger_level  for ele in ampl_Scint[i])     
    if (res == False):
        fake_trigger+=1
        continue
    for j in range(len(ampl_TPC[0])):
        all_ampl_TPC.append(ampl_Scint[i][j])
        wf_sum_Scint[j] += ampl_Scint[i][j]
        wf_sum_TPC[j] += ampl_TPC[i][j]

time_st = time_Scint[0] 




#plt.bar(time_TPC[0], wf_sum)
#plt.bar(x,y)
fig, axs = plt.subplots(1, 2, figsize=(10, 4), sharey=False)
# Set common labels

axs[0].set_xlabel('Acquisiton Time [s]')
axs[0].set_ylabel('Amplitude Added [V]')
axs[1].set_xlabel('Acquistion Time [s]')
axs[0].plot(time_st, wf_sum_Scint)
axs[1].plot(time_st,wf_sum_TPC)





fig2, axs2 = plt.subplots(1, 2, figsize=(10, 4), sharey=False)
axs2[0].set_xlabel('Acquisiton Time [s]')
axs2[0].set_ylabel('Amplitude  [V]')
axs2[1].set_xlabel('Acquistion Time [s]')

axs2[0].plot(time_Scint[4],ampl_Scint[3])
axs2[0].plot(time_TPC[4],ampl_TPC[3])

axs2[1].plot(time_Scint[4],ampl_Scint[5])
axs2[1].plot(time_TPC[4],ampl_TPC[7])


fig3, axs3 = plt.subplots(1, 2, figsize=(10, 4), sharey=False)
#axs3[0].hist(all_ampl_TPC,1000, (-0.001, 0.001))
axs3[1].hist(time_TPC)

mean,std=norm.fit(all_ampl_TPC)

axs3[0].hist(all_ampl_TPC, 1000, (-0.001, 0.001), density = True)

xmin = -0.001
xmax = 0.0005
x = np.linspace(xmin, xmax, 100)
y = norm.pdf(x, mean, std)
axs3[0].plot(x, y)
axs3[0].show()

print ("fake trigger events: ", fake_trigger, "out of ",len(ampl_TPC))



    




#plt.legend()
#plt.xticks(rotation=70)
fig.savefig(path+"wf_added.pdf")
fig2.savefig(path+"single_events.pdf")








