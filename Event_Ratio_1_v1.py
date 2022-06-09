#Event Ratio Simulation For TPC,
#Version 1, crossed scintillators below the chamber

import random
import math

#w=0.5 #vertical width of scintillators
#u=11  #distance to bottom of TPC
r=11  #radius of TPC
nr=-11
#l=11  #length of TPC
hl=5.5
nhl=-5.5
#a=5   #Horizontal width of one scintillator
ha=2.5
#b=6   #Horizontal width of the other scintillator
hb=3
h=10.5 #height from table to CENTER of TPC (=u+r-w)
halfpi=1.57079633
#pi=3.1415926535897984626433
twopi=6.28318531

scint_events=0
tpc_after_scint_events=0

num_events_requested=1000,0000 #total number of events requested

def generateEvent():
    x=random.uniform(0,ha)
    z=random.uniform(0,hb)
    theta=random.uniform(0,twopi)
    phi=random.uniform(0,halfpi)
    return [x, z, theta, phi]

def checkTPCIntersect(event):#this is an approximation for now, when I have time I will do the full-blown version
    m=h*math.tan(event[3])
    dx=m*math.sin(event[2])
    dz=m*math.cos(event[2])
    ex=event[0]+dx
    ez=event[1]+dz
    if ((ex>nr)and(ex<r)and(ez>nhl)and(ez<hl)): #put here the condition for an event to cross the chamber
        return math.cos(event[3])**2
    else:
        return 0
                        
def checkScintIntersect(event):
    return 1 #this will be a lot more complicated for the other configuration, but here the ray must always pass through both scintillators

for k in range(num_events_requested):
    currentevent=generateEvent()
    if (1==checkScintIntersect(currentevent)):
        scint_events=scint_events+1
        tpc_after_scint_events=tpc_after_scint_events+checkTPCIntersect(currentevent)

ratio=tpc_after_scint_events/scint_events
print('Ratio of events to triggers:', ratio)
