#Event Ratio Simulation For TPC,
#Version 1, crossed scintillators below the chamber

import random
from random import uniform
import math
from math import cos
from math import sin
from math import tan

#w=0.5 #vertical width of scintillators
#u=11  #distance to bottom of TPC
r=11  #radius of TPC
nr=-11
r2=r**2 #121
#l=11  #length of TPC
hl=5.5
nhl=-5.5
#a=7   #Horizontal width of one scintillator
ha=3.5
#b=7   #Horizontal width of the other scintillator
hb=3.5
h=22 #height from table to CENTER of TPC (=u+r-w)
h2=h**2
halfpi=1.57079633
#pi=3.1415926535897984626433
twopi=6.28318531

scint_events=0
tpc_after_scint_events=0

num_events_requested=10000000 #total number of events requested

def generateEvent():
    x=uniform(0,ha)
    z=uniform(0,hb)
    theta=uniform(0,twopi)
    phi=uniform(0,halfpi)
    return [x, z, theta, phi]

def checkTPCIntersect(event):#updated for the correct geometry
    alpha=tan(event[3])*cos(event[2])
    alpha1=alpha**-1
    alpha2=alpha**-2
    disc=(alpha2*(event[0]*alpha1+h)**2)-((alpha2+1)*(alpha2+(2*alpha1*h*event[0])+h2+r2)) #the 'discriminant' determines if there's a soln for which the infinite cylinder intersects the line. I divided by 4 since not needed to check
    if (disc<0):
        return 0
    else:
        diff=(disc**0.5)
        base=alpha2*event[0]+alpha1*h
        invdenom=(alpha2+1)**-1 #inverse of denominator
        #for readability, this is what is going on: 
        #x1=(base+diff)*invdenom
        #x2=(base-diff)*invdenom
        #beta=tan(event[3])*sin(event[2])
        #z1=event[1]+beta*alpha1*(x1-event[0])
        #z2=event[1]+beta*alpha1*(x2-event[0])
        #for computational efficiency, we don't save the intermediate numbers x1, x2
        beta=tan(event[3])*sin(event[2])
        z1=event[1]+beta*alpha1*(((base+diff)*invdenom)-event[0])
        z2=event[1]+beta*alpha1*(((base-diff)*invdenom)-event[0])
        if ((z1<hl)and(z1>nhl))or((z2<hl)and(z2>nhl)):
            return cos(event[3])**2
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
