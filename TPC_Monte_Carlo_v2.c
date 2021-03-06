/*
Monte Carlo Simulation of the entire TPC. We would like to do the following: 
1. Generate a cosmic ray from the expected (approximate) cos^2 zenith angle dependent distribution. 
1.b. Maybe include cosmic ray showers later...
2. Determine the path of the cosmic ray through the TPC, and whether it will trigger the data collection or not. 
3. Determine the number of primary electrons and primary photons generated by the particle. 
    a. GEM transparency simulation shows that we don't need to worry about the primary photons within the drift region since >99.1% won't make it through. 
4. Determine whether the particle path crosses the PEN, and produces scintillation photons there. 
5. Determine the signal readout on the oscilloscope due to this particle. -- May 12th determine on MMPC instead since that is what we are now using. 
6. Statistics: 
    a. type of event: normal event (drift chamber only), between ThGEM and PEN event, cross PEN event, etc. 
    b. time of first signal after trigger
    c. time of second signal after trigger
    d. time width of first signal after trigger
    e. distance between time of 1st and 2nd signal after trigger
    This data should be organized in such a way that we can look at the histograms either for all data, or by event type. 
7. Maybe include ion backflow later

Edward John Roe-March 11th, 2022

March 15th, 2022: Should consider changing all doubles to floats, because our measurements are not that accurate in the first place, and number of events is much more important than perfect precision in each event's calculations. Will have to check what data types the libraries work with though...

March 22nd, 2022: I found a nice reference which can be used for estimating the energy loss of the cosmic ray muon in the argon as well as the diffusion due to multiple coulomb scattering: 
Interaction of Radiation with Matter: From the Theory to the Measurements, by Stefano Meroli

May 12th, 2022: Fixed most of the bugs in the code, although there is still the persistent bug of the wrong number of intersects being found sometimes, which indicates a probable mistake in my algebra somewhere
when doing the geometry to determine whether the TPC is crossed. It seems to run very fast so far; 9 seconds for 100000 events. I expect it to slow down quite a lot when we start actually saving the data to a file. 

May 15th, 2022: Fixed the remaining bugs and added an improved user interface. The speed is slightly slower now (115 seconds per 1 million events), most likely because more events are correctly evaluated as crossing 
the TPC and thus more computation is needed. There are some improvements can be made which will improved the speed, for instance using the array random number filling where possible. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "SFMT.h"

#define TWOPI 6.28318530718

//event types
//we define a normal event to be one which crosses only the drift chamber. 
#define NORMAL_EVENT 0 
//we define a gem event to be one which crosses the ThGEM but not the PEN
#define GEM_EVENT 1
//we define a "between" event to be one which crosses between ThGEM and PEN at some point but does not cross the ThGEM or PEN
#define BTWN_EVENT 2
//we define a PEN event to be one which crosses the PEN but not the ThGEM
#define PEN_EVENT 3
//we define a "doublecross" event to be one which crosses both ThGEM and PEN
#define XX_EVENT 4


/*
I define a couple of data structures here. Arrays would suffice, because the 
data types are all doubles, but this makes the code more legible since the variables get well-defined names. 
If necessary to speed things up, I can always switch to arrays...
*/
//structure to hold vector
struct vector {
    double x;
    double y;
    double z;
} gem_intersect_vector, pen_intersect_vector, slope; //initialize two of these to hold intersect positions

//structure to hold two vectors with different names for the tpc intersect
struct tpc_intersect_vectors {
  double x1;
  double x2;
  double y1;
  double y2;
  double z1;
  double z2;
} current_intersect_vectors; //initialize one of these immediately. 
//it may be more efficient to use a pointer for this, but shouldn't make much difference. 
//if wanted, we can declare a pointer like this: 
//struct tpc_intersect_vectors *pointer_to_intersect_vectors = &current_intersect_vectors;


//a fast way to find the sign of a number
int sgn_double (double number){
    return ((number<=0)+(number>=0));
}
int sgn_float (float number){
    return ((number<=0)+(number>=0));
}
int sgn_int (int number){
    return ((number<=0)+(number>=0));
}

//using the above sgn function we now have a fast way to return the absolute value of a number as well
double abs_double (double number){
    return sgn_double(number)*number;
}
float abs_float (float number){
    return sgn_float(number)*number;
}
int abs_int (int number){
    return sgn_int(number);
}

//we can also do min and max
double min_double(double num1, double num2){
    return ((num1+num2)-abs_double(num1-num2))*0.5;
}
double max_double(double num1, double num2){
    return ((num1+num2)+abs_double(num1-num2))*0.5;
}
float min_float(float num1, float num2){
    return ((num1+num2)-abs_float(num1-num2))*0.5;
}
float max_float(float num1, float num2){
    return ((num1+num2)+abs_float(num1-num2))*0.5;
}
int min_int(int num1, int num2){
    return ((num1+num2)-abs_int(num1-num2))*0.5;
}
int max_int(int num1, int num2){
    return ((num1+num2)+abs_int(num1-num2))*0.5; //not sure if this is allowed on ints... but we will never get a non-integer out so maybe it's fine. 
}

int factorial(int n){
    int val = 1;
    for (int k=2; k<=n; k=k+1){
        val = val*k;
    }
    return val;
}

//user input return types
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

//credit to paxdiablo on stackoverflow for this function
static int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return OK;
}




//My code: generate an angle theta according to a cos^2 distribution
/*
Note that this is one place the accuracy can be improved. In particular, the distribution should "flatten out" as the energy of the muons increases, since they will not be stopped or have time to decay. 
This can be added later if needed... it may be hard to find literature on this so a calculation, simulation, or experiment will need to be done. 
Also, at extreme angles (theta>80 degrees) we need to take into account the curvature of the earth. However, cosmic rays coming from extreme angles will not activate our trigger mechanism (at least not the one we are using at the moment), so this may not be an issue. 
Update: I found a paper on this, but it is a bit sketchy in my opinion... they do not explain their model well enough and it seems to me that they just fit with a bunch of parameters to get a good fit, and the interpolations will probably not be valid. 
*/
double cosmicThetaRand(sfmt_t* rand) {
    double thetamin = 0;
    double thetamax = M_PI_2; 
    double possible_theta;
    double diff;
    double prob; 
    int ctr = 0;
    diff = thetamax-thetamin; 
    while (1){ //loop until we get a number...this is a bit dangerous, but we can't do without it so we just have to wait... A potential option is to add a breakout and return theta = (thetamin+thetamax)*0.5 or (thetamin+thetamax)*rand, but that will introduce a bias. The same infinite guessing procedure is implemented in the other random generators. 
    //printf("Trying a number for theta\n");
    ctr = ctr+1;
    possible_theta = thetamin+(sfmt_genrand_real1(rand)*diff);
    prob = (pow((cos(possible_theta)),2));
    if (prob>sfmt_genrand_real1(rand)){ //See how it compares with the integrated probability using random numbers. 
        //printf("Had to try %d values for theta before a success.\n", ctr);
        return(possible_theta);
    }
    }
}

double cosmicEnergyRand(sfmt_t* rand, double theta) { 
    /*
    We assume that this is constant with respect to theta, but we can increase accuracy by including the theta dependence. We have to be careful because there are two competing effects: the 
    loss of energy due to the atmospheric depth, and the decay of the lower-energy muons due to the longer path length. The exact solution may be difficult to obtain, but will be interesting. 
    Updated and improved based on the paper by Shukla and Sankrith "Energy and angular distributions of comsic ray muons at earth." 
    Muons of <1GeV are excluded due to the sun's magnetic field blocking low-energy cosmic rays. 
    */
    double possible_E; //energy measured in GeV
    double min_E = 1; //1GeV and lower are excluded due to the sun's magnetic field. 
    double max_E = 10000; //We don't usually see anything above 10 TeV, though it can happen. We have to make the cutoff somewhere and this is where the intercept of our graphs would be if they stayed linear all the way down. 
    double prob;
    double anglefraction = theta*0.7639437268; //how far it is on the way to 75 degrees, which was used for the experiments
    double E0 = 3.87 + 19.91*anglefraction*anglefraction; //interpolating quadratically for now. This could be improved upon, of course! It should increase a function of the effective depth of the atmosphere, but this doesn't quite seem to work based on the data... it's more complex than that. A quadratic fits that form somewhat well though. 
    double epsilon = 854+(1146*anglefraction*anglefraction);
    double invepsilon = pow(epsilon,-1); //same quadratic approximation to the true function. Here the true function works well, but it's an unecessary computation in the grand scheme of things. If wanted later, the depth (but not the effective depth, which is much more complicated) of atmosphere ratio at theta as compared to 0 is given by sqrt(R^2/d^2cos^2(theta)+2R/d+1)-R/dcos(theta). 
    double invemaxpeps = pow((max_E-epsilon),-1);
    double inveminpeps = pow((min_E-epsilon),-1);
    int ctr = 0;
    //double integrated_probability = epsilon*pow((E0-epsilon),-3)*(((0.5*(E0-epsilon)*(e*E0-epsilon+2*max_E)*invemaxpeps*invemaxpeps)+log((epsilon+max_E)*invemaxpeps))-((0.5*(E0-epsilon)*(e*E0-epsilon+2*max_E)*inveminpeps*inveminpeps)+log((epsilon+max_E)*inveminpeps))); //Integrated total probability since we don't normalize later on. This is needed to use the comparison trick. 
    //double integrated_probability = -0.5*(pow(E0+max_E,2)+pow(E0,2)); //integrates the below probability function. Needed for the comparison trick. 
    double max_probability = pow(E0+min_E,-3)*pow((1+min_E*invepsilon),-1);
    while (1){
        ctr = ctr+1;
        //printf("Trying an energy value\n");
        possible_E = min_E+max_E*sfmt_genrand_real1(rand); //don't worry that technically we're adding min_E to the max by doing this; it's insignificant and it was arbitrary anyways. Not worth the extra memory space and computation it would take! 
        prob = pow((E0+possible_E),-3)*pow((1+possible_E*invepsilon),-1); //got this (as well as the rest of the things for this function) from the paper by Pragati Mitra and Prashant Shukla on 'Energy and angular distribution of cosmic muons.' That paper is a bit sketchy but it's the best thing I could find. 
        //the above probability can be significantly improved in accuracy. Don't forget to change the max probability as well...
        if (prob>max_probability*sfmt_genrand_real1(rand)){ 
            //printf("Had to try %d values for the energy before a success.\n", ctr);
            return(possible_E);
        }
    }
}



int checkTrigger (double x, double y, double theta, double phi){
    //This will need to be programmed depending ont he geometry of the trigger. Return 1 if the trigger is hit, 0 if it is not. 
    return 1;
}

int checkTPCIntersect (double x, double y, double theta, double phi){

    //printf("Checking if the TPC was crossed...\n");
    //see coordinate system definitions below in main function
    //tpc
    static double drift_chamber_length = 11;
    static double tpc_radius_2 = 121; //radius squared of tpc
    static double gem_thickness = 0.035;
    static double gem_to_pen_length = 0.1;
    double total_length = drift_chamber_length + gem_thickness + gem_to_pen_length;

    //trigger system
    static double scint_to_tpc_center = 22; //vertical distance from top of bottom scintillator to center of TPC drift chamber. 
    
    int num_outer_intersects_found = 0; //these are the "entering or exiting the tpc" intersects (not internal intersects like gem intersect)
    
    //overhauling this function... make sure it works first then optimize later

    //first check if the ends are crossed: 
    //end at y=0: delta y = 0-y = -y
    double invtanthetasinphi = 1/(tan(theta)*sin(phi));
    double xtemp = x-(y/tan(phi));
    double ztemp = -y*invtanthetasinphi;
    if ((pow(xtemp,2)+pow((ztemp-scint_to_tpc_center),2))<tpc_radius_2){
        current_intersect_vectors.x1 = xtemp;
        current_intersect_vectors.y1 = 0;
        current_intersect_vectors.z1 = ztemp;
        num_outer_intersects_found = num_outer_intersects_found+1;
    }

    //end at y=length of tpc: deltay = length of tpc-y
    double deltay = total_length-y;
    xtemp = x+(deltay/tan(phi));
    ztemp = deltay*invtanthetasinphi;
    if((pow(xtemp,2)+pow((ztemp-scint_to_tpc_center),2))<tpc_radius_2){
        if (0==num_outer_intersects_found){
            current_intersect_vectors.x1 = xtemp;
            current_intersect_vectors.y1 = total_length;
            current_intersect_vectors.z1 = ztemp;
        }
        else{
            current_intersect_vectors.x2 = xtemp;
            current_intersect_vectors.y2 = total_length;
            current_intersect_vectors.z2 = ztemp;
        }
        num_outer_intersects_found = num_outer_intersects_found+1;
    }

    //next we check if it crossed the cylinder of the TPC. 
    double invtanthetacosphi = 1/(tan(theta)*cos(phi));
    double a = 1 + invtanthetacosphi*invtanthetacosphi;
    double b = (x - (scint_to_tpc_center*invtanthetacosphi));
    double c = (x*x)+(scint_to_tpc_center*scint_to_tpc_center)-tpc_radius_2;

    double disc = (b*b-a*c);
    if (disc>0){
        double sd = sqrt(disc);
        double t1 = (-b+sd)/a;
        double t2 = -(b+sd)/a;
        double y1 = (t1*tan(phi))+y;
        double y2 = (t2*tan(phi))+y;
        if ((y1>0)&&(y1<total_length)){
            if (0==num_outer_intersects_found){
                current_intersect_vectors.x1 = t1+x;
                current_intersect_vectors.y1 = y1;
                current_intersect_vectors.z1 = t1*invtanthetacosphi;
            }
            else{
                current_intersect_vectors.x2 = t1+x;
                current_intersect_vectors.y2 = y1;
                current_intersect_vectors.z2 = t1*invtanthetacosphi;
            }
            num_outer_intersects_found = num_outer_intersects_found+1;
        }

        if ((y2>0)&&(y2<total_length)){
            if (0==num_outer_intersects_found){
                current_intersect_vectors.x1 = t2+x;
                current_intersect_vectors.y1 = y2;
                current_intersect_vectors.z1 = t2*invtanthetacosphi;
            }
            else{
                current_intersect_vectors.x2 = t2+x;
                current_intersect_vectors.y2 = y2;
                current_intersect_vectors.z2 = t2*invtanthetacosphi;
            }
            num_outer_intersects_found = num_outer_intersects_found+1;
        }
        
    }


    if (0<num_outer_intersects_found){
        //printf("TPC was crossed!\n");
        //printf("%d intersects were found.\n", num_outer_intersects_found);
        if (!(2==num_outer_intersects_found)){//for debug
            printf("Error! Wrong number of intersects (%d) found!!!\nThis indicates a mistake in the computations!\n", num_outer_intersects_found);
            printf("The ray generated had the following coordinates: (x,y,theta,phi)=(%f,%f,%f,%f).\n",x,y,theta,phi);
            printf("The coordinates of the first intersect found were:\nx=%f\ny=%f\nz=%f\n",current_intersect_vectors.x1,current_intersect_vectors.y1,current_intersect_vectors.z1);
            printf("the discriminant was: %f\n", disc);
            printf("Returning 0.\n\n");
            return 0;
        }
        //printf("Returning 1.\n");
        return 1;
    }
    else{
        //printf("TPC was not crossed.\n");
        //printf("Returning 0.\n");
        return 0;
    }
    }


int checkEventType (){
    static double gem_to_pen_length = 0.1;
    static double gem_thickness = 0.035;

    int event_type = -1; //initialize the event type to a garbage value so that if this doesn't work we know immediately
    //there may be a bit of room for a tiny optimization here in which we do away with these two variables. 
    int btwn_crossed = 0;
    int pen_crossed = 0;

    double y1 = current_intersect_vectors.y1;
    double y2 = current_intersect_vectors.y2;

    double min_y = min_double(y1,y2);
    double max_y = max_double(y1,y2);

    if (min_y<gem_to_pen_length){ // first check this
        btwn_crossed = 1;
        if (0==min_y){ //since the intersects must occur in the range [0,total_length], a zero intersect will always be the minimum. 
            pen_crossed = 1;
        }
    }

    if (max_y>(gem_to_pen_length+gem_thickness)){ //check the most likely cases first, and break down in a binary way, to ensure optimal speed
        if (0==btwn_crossed){
            event_type = NORMAL_EVENT;
        }
        else {
            if (0==pen_crossed){
                event_type = GEM_EVENT; //not perfectly consistent with the later, but gives us the information the way it is useful
            }
            else{
                event_type = XX_EVENT;
            }
        }
    }
    else{
            if (1==btwn_crossed){
                if (1==pen_crossed){
                    if (max_y<gem_to_pen_length){
                        event_type = PEN_EVENT; //note that a PEN event does not mean the PEN itself was crossed, but only the plane the PEN is in. 
                    }
                    else{
                        event_type = XX_EVENT;
                    }
                }
                else {
                        if (max_y<gem_to_pen_length){
                        event_type = BTWN_EVENT;
                    }
                    else{
                        event_type = GEM_EVENT;
                    }
                }
            }
            else{
                event_type = GEM_EVENT;
            }
        }

    return event_type;
}

uint32_t calc_num_e_clusters(sfmt_t* rand, double N_expected, double muon_energy){//returns a random value of the number of clusters according to a poisson distribution
    uint32_t N;
    //printf("Expected number of clusters: %f\n", N_expected);
    double max = 1;
    if (N_expected<9.7){//to avoid overflow
        max = pow(N_expected,N_expected)/factorial(N_expected); //not really; we are leaving out an exponential factor of e^(expected value of N) but relative probabilities are all that matter here. 
    }
    else if (N_expected<22.1){ //if we expect a lot, use stirling's approximation
        max = exp(N_expected)/sqrt(TWOPI*N_expected); 
    }
    else {
        max = 4294967295; //max val for a uint32_t
    }
    //printf("max prob (num_e_clusters) = %f\n", max);
    uint32_t maxN = (uint32_t) N_expected+floor(muon_energy/(26.4E-3)); //We set a max number of clusters so that if 100 electrons per cluster are produced the muon will still not exceed 0.01% of its energy being lost. the probability of having more than this is negligible anyways due to the poisson distribution. 
    //printf("maxN = %u\n", maxN);
    int ctr = 0;
    while (1){
        ctr = ctr+1;
        //printf("Trying a number of clusters\n");
        N = (uint32_t) floor(maxN*sfmt_genrand_real1(rand)); //technically the probability of getting exactly MaxN is diminished in this method, but it's irrelevant since we're cutting it off after that point anyways. The important thing is that by using floor we can still get zero...
        if (N<maxN){
            if(max*sfmt_genrand_real1(rand) < (pow(N_expected,N)/factorial(N))){
                //printf("Had to try %d values for the number of clusters before a success.\n", ctr);
                return N;
            }
        }
    }
}

uint32_t calc_num_e(sfmt_t* rand){
    uint32_t n;
    //This list is from table 1.4 on page 17 of "Particle Detectino with Drift Chambers (1994 edition)" of cluster size distributions. 
    double probs[19] = {0.656, 0.15, 0.064, 0.035, 0.0225, 0.0155, 0.0105, 0.0081, 0.0061, 0.0049, 0.0039, 0.0030, 0.0025, 0.0020, 0.0016, 0.0012, 0.00095, 0.00075, 0.00063};
    int maxn = 100; //limit to 1000 electrons; we need to set a cutoff somewhere to make sure we don't allow silly large amounts... we can adjust this later if wanted of course! There has to be a balance between this and the poisson distribution cutoff above... the 1/k^2 falls off more slowly for small n but the poisson does for large n. 
    int ctr = 0;
    while (1){
        ctr = ctr+1;
        n = 1+ floor(sfmt_genrand_real1(rand)*maxn); 
        //printf("Trying a number of electrons\n");
        if (n<maxn){
            if (n<20){
                if (sfmt_genrand_real1(rand)<probs[n-1]){
                    //printf("Had to try %d values for a number of electrons for this cluster before a success.\n", ctr);
                    return n;
                }
            }
            else{
                if (sfmt_genrand_real1(rand)<(0.216/(n*n))){
                    //printf("Had to try %d values for a number of electrons for this cluster before a success.\n", ctr);
                    return n;
                }
            }
        }
    }
}

int calc_drift_contribution(){
    return 0;
}
int calc_btwn_contribution(){
    return 0;
}
int calc_pen_scint(){
    return 0;
}

void event_breakdown(n_evts, num_normal_events, num_xx_events, num_gem_events, num_btwn_events, num_pen_events){
    printf("Breakdown of events by event type: \n");
    printf("Total number of events: %d\n", n_evts);
    printf("Number of Normal events: %d\n", num_normal_events);
    printf("Number of Double-Cross events: %d\n", num_xx_events);
    printf("Number of GEM events: %d\n", num_gem_events);
    printf("Number of Between events: %d\n", num_btwn_events);
    printf("Number of PEN events: %d\n", num_pen_events);
}

void help(){
    printf("Command definitions: \n");
    printf("\"q\", \"quit\", or \"exit\"  -- exit\n");
    printf("\"d\", or \"definitions\" -- show event type definitions\n"); 
    printf("\"e\", or \"events\" -- show the events by type breakdown\n");
    printf("\"h\", or \"help\" -- display the help menu\n");
    printf("\"c\", or \"credits\" -- display credits\n");
}

int main (){
    current_intersect_vectors.x1 = 0;
    current_intersect_vectors.x2 = 0;
    current_intersect_vectors.y1 = 0;
    current_intersect_vectors.y2 = 0;
    current_intersect_vectors.z1 = 0;
    current_intersect_vectors.z2 = 0;
    //get time at the beginning so we can evaluate total time later, and have a basic measure of performance. 
    clock_t program_start_time, program_end_time;
    
    printf("\n\n--------------------------------------------------------------------------\n");
    printf("| High-Pressure Gaseous Argon Optical Time Projection Chamber Simulation |\n");
    printf("--------------------------------------------------------------------------\n\n\n");

    //First ask the user how many events are desired
    char temporary[15];
    char buffer[15]; //I made this small because we definitely shouldn't need more than 10^15 events...
    char seed[20]; //seed for random number generator
    unsigned int n_evts;
    getLine("What is the desired number of events? ", buffer, sizeof(buffer));
    n_evts = atoi(buffer);
    getLine("Enter a seed for the Mersenne Twister random number generator. ", seed, sizeof(seed));
    printf("Beginning simulation.\n");
    program_start_time = clock();
    sfmt_t sfmt;
    sfmt_init_gen_rand(&sfmt, atoi(seed)); //initialize the random number generator with the chosen seed
    //remember to save the seed in the file header so that results can be reproduced. 
    //printf("Testing the random number generator: %f\n", sfmt_genrand_real1(&sfmt));
    //printf("Testing the random number generator again: %u\n", sfmt_genrand_uint32(&sfmt));
    /*
    This may need to be moved. 
    Set up the variables for the simulation. We have many parameters for the TPC dimensions, 
    as well as variables describing the trigger. We make these static so they don't change. 
    All lengths are measured in centimeters. 
    Be sure to check these numbers before the real runs; they could be quite wrong.  
    */
    //tpc
    static double drift_chamber_length = 11;
    static double tpc_radius = 11;
    static double gem_thickness = 0.035;
    static double gem_hole_radius = 0.02;
    static double gem_to_pen_length = 0.1;

    //trigger system
    static double scint_width = 7;
    static double scint_length = 50.5;
    static double scint_to_tpc_center = 22; //vertical distance from top of bottom scintillator to center of TPC drift chamber. 
    static double tpc_center_to_top_scint = 20; //vertical distance from bottom of top scintillator to center of tpc drift chamber. 
    static double scint_dz = 42; //the sum of the last two, the vertical distance between the two scintillators. 

    /*
    To be completely unambiguous, this is the system of coordinates I am using: 
    z-axis: +z is up, -z is down towards the center of the earth. z=0 is defined to be at the top of the bottom scintillator. 
    y-axis: along the length of the TPC, i.e. its axis of radial symmetry. y=0: Pen, +y is in the direction of the cathode. Thus, all y values within the detector are positive. We will have some cosmic rays possibly enter coming form negative y coordinates, of course. 
    x-axis: +x is in the y cross z direction, as is standard. In terms of the detector, +x is to your right looking out from the PMT or SiPM array. 
    phi: the polar angle from +x-axis in the +y-direction. 
    theta: the azimuthal angle from +z-axis towards the xy-plane. 
    */

    //define some useful things that will be used a lot: 
    double max_generated_x = scint_width*0.5; //we assume that the scintillator is centered with respect to x
    double min_generated_x = -max_generated_x;
    double generated_x_dist = max_generated_x-min_generated_x;
    double y_offset = gem_to_pen_length+gem_thickness+(drift_chamber_length*0.5); //assuming the scintillator is centered on the center of the drift chamber.... this may or may not be true
    double min_generated_y = (-scint_length*0.5)+y_offset; 
    double max_generated_y = (scint_length*0.5)+y_offset;
    double generated_y_dist = max_generated_y-min_generated_y;

    int num_normal_events = 0;
    int num_btwn_events = 0;
    int num_xx_events = 0;
    int num_gem_events = 0;
    int num_pen_events = 0;

    //Main loop
    for (unsigned int j=n_evts; j--; ){ //it's marginally faster to do a counting down loop in the case where it can just compare against zero. Order does not matter here so I did it. 
        //First generate a random event. 
        double x = min_generated_x+generated_x_dist*sfmt_genrand_real1(&sfmt);
        double y = min_generated_y+generated_y_dist*sfmt_genrand_real1(&sfmt);
        double theta = cosmicThetaRand(&sfmt); //get a cosmic ray at angle theta. We should set the limits so that the ray has to trigger our data acquisition system. 
        double phi = TWOPI*sfmt_genrand_real1(&sfmt);
        //printf("%f, %f, %f, %f\n", x, y, theta, phi);
        //printf("Event generated.\n");
        /*
        Before we do any intensive computations, we do a couple of quick checks. 
        First, we check if the trigger is activated, because if not we don't want to record anything. 
        Second, we check if the TPC is crossed. If not, we record an event with no signal. 
        */
        //check if this cosmic ray will pass through the other scintillator and activate the trigger. 
        if (checkTrigger(x,y,theta,phi)){ //-->This is blank for now, need to write it!!!!
        //check if the cosmic ray will pass through the TPC
        //printf("Trigger system activated.\n");
        
        if (checkTPCIntersect(x,y,theta,phi)){ 
            
            //printf("Found intersects!\n");
            //next, check the event type
            /*
            //As a reminder here are the event types which were defined above
            //We define a normal event to be one which crosses only the drift chamber. 
            #define NORMAL_EVENT 0 
            //We define a gem event to be one which crosses the plane of the ThGEM but not the PEN. 
            #define GEM_EVENT 1
            //We define a "between" event to be one which crosses between ThGEM and PEN at some point but does not cross the ThGEM or PEN. 
            #define BTWN_EVENT 2
            //We define a PEN event to be one which crosses the plane of the PEN but not the ThGEM. 
            #define PEN_EVENT 3
            //We define a "doublecross" event to be one which crosses both the ThGEM and PEN planes. 
            #define XX_EVENT 4
            */
           
            double x1 = current_intersect_vectors.x1;
            double x2 = current_intersect_vectors.x2;
            double y1 = current_intersect_vectors.y1;
            double y2 = current_intersect_vectors.y2;
            double z1 = current_intersect_vectors.z1;
            double z2 = current_intersect_vectors.z2;
            double deltax = x2-x1;
            double deltay = y2-y1;
            double deltaz = z2-z1;
            double track_length = sqrt((deltax*deltax)+(deltay*deltay)+(deltaz*deltaz)); //we neglect the GEM thickness in those cases where the track crosses the gem. It may be crossing in gas through the gem anyways so might as well include it. 
            double muon_energy = cosmicEnergyRand(&sfmt, theta);
            //printf("Generated energy!\n");
            double gamma = muon_energy*9.4644651; //E/(mc^2)=gamma. 
            double beta = sqrt(1-pow(gamma,-2)); 

            int event_type = checkEventType(); 

            double N_expected = 30*track_length; //we may want to improve this later; it is a very rough estimate. At any rate it should be between 27 and 40 * track length in cm. 
            uint32_t num_e_clusters = calc_num_e_clusters(&sfmt, N_expected, muon_energy); //calculate the number of clusters based on a Poisson distribution and the expected number of clsuters
            //printf("Calculated number of clusters: %u clusters.\n", num_e_clusters);
            
            uint32_t num_e[num_e_clusters]; 
            double cluster_positions[num_e_clusters][3];
            
            for (int k=0; k<num_e_clusters; k=k+1){
                num_e[k] = calc_num_e(&sfmt); //decide how many electrons are in the cluster based on the distribution from table 1.4 of the book "Particle Detection with Drift Chambers"
                //printf("Calculated number of electrons for a cluster: %u\n", num_e[k]);
                
                //distribute cluster positions randomly along the track
                //for future optimizatino, this is one place where the array random number generation, which is a fair amount faster, can be used. It could also be used in the diffusion of the electrons. 
                cluster_positions[k][0] = x1+deltax*sfmt_genrand_real1(&sfmt);
                cluster_positions[k][1] = y1+deltay*sfmt_genrand_real1(&sfmt);
                cluster_positions[k][2] = z1+deltaz*sfmt_genrand_real1(&sfmt);
            }
            
            //last checked point
            //for testing purposes
            //printf("Number of clusters generated: %u\n", num_e_clusters);
            //printf("Detailed information for each cluster:\n");
            //printf("N, (x, y, z)\n");
            for (int k=0; k<num_e_clusters; k=k+1){
                //printf("%u, (%f, %f, %f)\n", num_e[k], cluster_positions[k][0],cluster_positions[k][1],cluster_positions[k][2]);
            }
            
            if (NORMAL_EVENT == event_type){
                    num_normal_events+=1;
                    calc_drift_contribution(); //this function should calculate the photons arriving at the PEN due to the e- produced in the drift region
            }
            if (XX_EVENT==event_type){
                num_xx_events+=1;
                calc_drift_contribution();
                calc_btwn_contribution();
                calc_pen_scint();
            }
            if (GEM_EVENT==event_type){
                num_gem_events+=1;
                calc_drift_contribution();
                calc_btwn_contribution(); //this function should calculate the s1 and EL photons which arrive at PEN from the region between PEN and ThGEM. 
            }
            if (PEN_EVENT==event_type){
                num_pen_events+=1;
                calc_btwn_contribution();
                calc_pen_scint(); //this function should calculate the photons produced by scintillation in the PEN. 
            }
            if (BTWN_EVENT==event_type){
                num_btwn_events+=1;
                calc_btwn_contribution();
            }
            
        }
        }
        else {
            //printf("checkTPCIntersect returned 0.\n");
        }

        //printf("Finished computations for one incident cosmic ray.\n\n");
    } //end of event loop

    program_end_time = clock();
    printf("Simulation complete. The total run time was %d seconds.\n\n", ((int)((program_end_time-program_start_time)/ CLOCKS_PER_SEC)));
    event_breakdown(n_evts, num_normal_events, num_xx_events, num_gem_events, num_btwn_events, num_pen_events);
    help();

    while(1){
        getLine("Enter a command: ", buffer, sizeof(buffer));
        if ((!(strcmp("q",buffer))) || (!(strcmp("quit",buffer))) || (!(strcmp("exit",buffer)))){
            printf("Exiting program. Goodbye. \n");
            return 0;
        }
        else if ((!(strcmp("h",buffer)) || (!(strcmp("help", buffer))))){
            help();
        }
        else if ((!(strcmp("d",buffer)) || (!(strcmp("definitions", buffer))))){
            printf("Event type definitions:\n");
            printf("We define a \"normal\" event to be one which crosses only the drift chamber.\n");
            printf("We define a \"GEM\" event to be one which crosses the plane of the ThGEM but not the plane of the PEN.\n");
            printf("We define a \"between\" event to be one which crosses between ThGEM and PEN planes at some point but does not cross the plane of ThGEM or of the PEN.\n");
            printf("We define a \"PEN\" event to be one which crosses the plane of the PEN but not of the ThGEM.\n");
            printf("We define a \"double-cross\" event to be one which crosses both the ThGEM and PEN planes.\n");
        }
        else if ((!strcmp("e",buffer)) || (!(strcmp("events",buffer)))){
            event_breakdown(n_evts, num_normal_events, num_xx_events, num_gem_events, num_btwn_events, num_pen_events);
        }
        else if ((!strcmp("c",buffer)) || (!(strcmp("credits",buffer)))){
                printf("This program was created by Edward John Roe in the year 2022.\n");
        }
    }

    return 0; //end of main function
}


