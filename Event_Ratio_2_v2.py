#Event Ratio Simulation For TPC,
#Version 2, 1 scintillator above the chamber and one below the chamber

import random
from random import uniform
import math
from math import cos
from math import sin
from math import tan

#w=0.5 #vertical width of scintillators
#u=11  #distance to bottom of TPC
r=11  #radius of TPC
r2=r**2 #121
nr=-11
l=11  #length of TPC
hl=l*0.5
nhl=-hl
a=7   #Horizontal width of 1st scintillator
ha=a*0.5
f=50.5 #length of 1st scintillator
hf=f*0.5
b=7   #Horizontal width of the 2nd scintillator
hb=b*0.5
nhb=-hb
g=50.5 #length of 2nd scintillator
hg=g*0.5
nhg=-hg
h1=22 #height from top of bottom scintillator to CENTER of TPC (=u+r-w)
h1sqrd=h1**2
h2=42 #height from top of bottom scint to bottom of top scint
halfpi=1.57079633
#pi=3.1415926535897984626433
twopi=6.28318531

scint_events=0
tpc_after_scint_events=0

num_events_requested=100000000 #total number of events requested

def generateEvent():
    x=uniform(0,ha)
    z=uniform(0,hf)
    theta=uniform(0,twopi)
    phi=uniform(0,halfpi)
    return [x, z, theta, phi]

def checkTPCIntersect(event):#updated for the correct geometry
    alpha=tan(event[3])*cos(event[2])
    alpha1=alpha**-1
    alpha2=alpha**-2
    disc=(alpha2*(event[0]*alpha1+h1)**2)-((alpha2+1)*(alpha2+(2*alpha1*h1*event[0])+h1sqrd+r2)) #the 'discriminant' determines if there's a soln for which the infinite cylinder intersects the line. I divided by 4 since not needed to check
    if (disc<0):
        return 0
    else:
        diff=(disc**0.5)
        base=alpha2*event[0]+alpha1*h1
        invdenom=(alpha2+1)**-1 #inverse of denominator
        #for readability, this is what is going on: 
        #x1=(base+diff)*invdenom
        #x2=(base-diff)*invdenom
        #beta=tan(event[3])*sin(event[2])
        #z1=event[1]+beta*alpha1*(x1-event[0])
        #z2=event[1]+beta*alpha1*(x2-event[0])
        #for computational efficiency, we don't save the intermediate numbers x1, x2
        beta=tan(event[3])*sin(event[2])*alpha1
        z1=event[1]+beta*(((base+diff)*invdenom)-event[0])
        z2=event[1]+beta*(((base-diff)*invdenom)-event[0])
        if ((z1<hl)and(z1>nhl))or((z2<hl)and(z2>nhl)):
            return cos(event[3])**2
        else:
            return 0
                        
def checkScintIntersect(event):
    m=h2*tan(event[3])
    dx=m*sin(event[2])
    dz=m*cos(event[2])
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
