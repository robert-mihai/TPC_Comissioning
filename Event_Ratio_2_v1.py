#Event Ratio Simulation For TPC,
#Version 2, 1 scintillator above the chamber and one below the chamber

import random
import math

#w=0.5 #vertical width of scintillators
#u=11  #distance to bottom of TPC
r=11  #radius of TPC
nr=-11
l=11  #length of TPC
hl=5.5
nhl=-5.5
a=5   #Horizontal width of 1st scintillator
ha=2.5
f=22 #length of 1st scintillator
hf=11
#b=6   #Horizontal width of the 2nd scintillator
hb=3
nhb=-3
g=22 #length of 2nd scintillator
hg=11
nhg=5.5
h1=10.5 #height from top of bottom scintillator to CENTER of TPC (=u+r-w)
h2=21.5 #height from top of bottom scint to bottom of top scint
halfpi=1.57079633
#pi=3.1415926535897984626433
twopi=6.28318531

scint_events=0
tpc_after_scint_events=0

num_events_requested=10000000 #total number of events requested

def generateEvent():
    x=random.uniform(0,ha)
    z=random.uniform(0,hf)
    theta=random.uniform(0,twopi)
    phi=random.uniform(0,halfpi)
    return [x, z, theta, phi]

def checkTPCIntersect(event):
    m=h1*math.tan(event[3])
    dx=m*math.sin(event[2])
    dz=m*math.cos(event[2])
    ex=event[1]+dx
    ez=event[2]+dz
    if ((ex>nr)and(ex<r)and(ez>nhl)and(ez<hl)): #put here the condition for an event to cross the chamber
        return math.cos(event[3])**2
    else:
        return 0
                        
def checkScintIntersect(event):
    m=h2*math.tan(event[3])
    dx=m*math.sin(event[2])
    dz=m*math.cos(event[2])
    ex=event[1]+dx
    ez=event[2]+dz
    if ((ex>nhb)and(ex<hb)and(ez>nhg)and(ez<hg)): #put here the condition for an event to cross the chamber
        return 1
    else:
        return 0
    
for k in range(num_events_requested):
    currentevent=generateEvent()
    if (1==checkScintIntersect(currentevent)):
        scint_events=scint_events+1
        tpc_after_scint_events=tpc_after_scint_events+checkTPCIntersect(currentevent)

ratio=tpc_after_scint_events/scint_events
print('Ratio of events to triggers:', ratio)
