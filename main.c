#include <lpc17xx.h>
#include "glcd.h"
#include <stdbool.h>
#include "fault_injection.h"


#include <stdio.h>
#include <math.h>

// Int:1  Double:0
#define INT_DOUBLE 0

// STUCK_AT_FAULT:1 (always)
// RANDOM_FAULT:0 (probability)
int fault_mode;

char cur_str[20];
char val_str[20];
char fal_str[20];
char tar_str[20];
char yo_str[20];
char demo_case[20];
int i;

double newtown_raphson(double input);
int kbd_val;
int int0_val;
int random_int;
double random_double;
struct redundant_int reda;
struct redundant_double redb;
double newtown_double;
double newtown_sqrt;
double bst_double;
double bst_sqrt;
int state_counter;
int fault_mode;
int input_int=1;
double input_double = 1.0;
double newtown[3];
double diverse[3]; 
double xn;

//faulty_int(1, RANDOM_FAULT)
//faulty_int(1, STUCK_AT_FAULT)
//faulty_double(1, RANDOM_FAULT)
//faulty_double(1, STUCK_AT_FAULT)

/*
set_input(){
    LPC_ADC->ADCR |= (1<<24);
    while (LPC_ADC->ADGDR & 0x8000 == 0);
    input_int = (LPC_ADC->ADGDR>>4) & 0xFFF;
}*/
    
    
init_pm()
{
  LPC_PINCON->PINSEL1 &= ~(3<<18);
  LPC_PINCON->PINSEL1 |= (1<<18);
  LPC_SC->PCONP |= (1<<12);
  LPC_ADC->ADCR = (1<<2)| (4<<8)| (1<<21);
  return 0;
}

struct redundant_int {
   int  value;
   int   negated;
};

struct redundant_double {
   double  value;
   double  negated;
};




void set_int(struct redundant_int *redun_a, int input){
		redun_a->value = input;
		// The two's complement of an integer is exactly the same thing as its negation.
	// which gives a hint about how to design a circuit for negation. It means "to find the negation of a number \
	// (i.e., its two's complement) you flip every bit then add 1".?
		redun_a->negated = ~(input);
}

bool get_int(struct redundant_int *redun_a){
		if(redun_a->value== ~redun_a->negated){
			return true;
		}else{
			return false;
		}
}

double negate_double(double input){
	  uint64_t * const p_int = (uint64_t *)&input;
      *p_int = ~*p_int;
	 return input;
}

void set_double (struct redundant_double *redun_a, double input){
		redun_a->value = input;
		redun_a->negated = negate_double(input);
	}

bool get_double (struct redundant_double *redun_b){
		if(negate_double(redun_b->value) == redun_b->negated){
			return true;
		}else{
			return false;
		}
}



double newtown_raphson(double input){
		double a = input;
		double xn = 0.5 *(input +1);
		double xnplus = 0.5 * (xn+ a/xn);
  
    /* faulty injection */ 
    xnplus = faulty_double(xnplus, fault_mode);
    fault_injection_reset();
  
		while(abs(xnplus - xn) > 1e-6){
				xn = xnplus;
      

			
        xnplus = 0.5*(xnplus+ a/xnplus);
		}
    
    xn = faulty_double(xn, fault_mode);
    fault_injection_reset();
    
		return xn;
}


double * newtown_raphson_once(double input){
    
    /* faulty injection */
		double a = faulty_double(input, fault_mode);
    fault_injection_reset();
    
		xn = 0.5 *(input +1);
    newtown[0] = 0.5 * (xn+ a/xn);;
    newtown[1] = 0.5*(newtown[0]+ a/newtown[0]);
    newtown[2] = 0.5*(newtown[1]+ a/newtown[1]);
		return newtown;
}




double bst(double input){
		double low = 0;
		double high = input+1;
	  double mid;
    while((high - low) > 1e-6){
			
      mid = (low+high)/2;
			/* faulty injection */
      mid = faulty_double(mid, fault_mode);
      fault_injection_reset();
      
      if(mid * mid <= input){
				low = mid;
			}else{
				high = mid;
			}
		}
			return low;
}

double * diversCompute(double input){
		
    diverse[0] = sqrt(input);
    diverse[0] = faulty_double(diverse[0], fault_mode);
    fault_injection_reset();
		diverse[1] = newtown_raphson(input);
		diverse[2] = bst(input);
    return diverse;
}




bool verif(double input){
		double ret = newtown_raphson(input) * newtown_raphson(input);

    ret = faulty_double(ret, fault_mode);
    fault_injection_reset();
  
		if (abs(ret - input)<1e-6 ){
				return true;
			}else{
				// fault hanlder
				return false;
			}
}


init_joystick()
{
  LPC_PINCON->PINSEL3 &= ~((3<< 8)|(3<<14)|(3<<16)|(3<<18)|(3<<20));
  LPC_GPIO1->FIODIR &= ~((1<<20)|(1<<23)|(1<<24)|(1<<25)|(1<<26));
  return 0;
}

init_int0()
{
  LPC_PINCON->PINSEL4 &= ~(3<<20);
  LPC_GPIO2->FIODIR &= ~(1<<10);
  return 0;
}


init_LEDs()
{
  LPC_GPIO1->FIODIR |= 0xB0000000; // LEDs on PORT1
  LPC_GPIO2->FIODIR |= 0x0000007C; // LEDs on PORT2
  return 0;
}

clear_LEDs()
{
  LPC_GPIO1->FIOCLR = 1 << 28;
  LPC_GPIO1->FIOCLR = 1 << 29;
  LPC_GPIO1->FIOCLR = 1 << 31;
  LPC_GPIO2->FIOCLR = 1 << 2;
}




mode_switch(){
  
      if (int0_val == 1){
       state_counter++;
       if (state_counter >= 5){
           state_counter = 0;
       }

       if (state_counter == 1){
            clear_LEDs();
            GLCD_Clear(White);
            LPC_GPIO1->FIOSET = 1 << 28;
            strncpy (demo_case, "Data Redundancy");

        }else if (state_counter==2){
              clear_LEDs();
              GLCD_Clear(White);
              LPC_GPIO1->FIOSET = 1 << 29;
              strncpy (demo_case,  "Voting System");

         }else if (state_counter==3){
              clear_LEDs();
              GLCD_Clear(White);
              LPC_GPIO1->FIOSET = 1 << 31;
              strncpy (demo_case, "Diverse Compute ");
        }else if(state_counter==4){
              clear_LEDs();
              GLCD_Clear(White);
              LPC_GPIO2->FIOSET = 1 << 2;
              strncpy (demo_case, "Result Verification ");
        }
	}
}


int main(void){

	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	init_joystick();
  init_int0();
	init_LEDs();
  init_pm();
  strncpy (demo_case,"init (stuck fault)");
	
	set_double(&redb, 2.22);
	newtown_double = 0;
  state_counter = 0;
  fault_mode = 1;
  set_int(&reda, 1);
  
	for(;;){
		// display Demo Case
		snprintf(cur_str,  sizeof(cur_str), "Case: %s", demo_case);
		GLCD_DisplayString(0, 0, 1, cur_str);
    
    
//    set_input(); 

		// Switch Demo Case
		kbd_val = ~(LPC_GPIO1->FIOPIN >>20) & 0x79;
    int0_val = ~(LPC_GPIO2->FIOPIN >> 10) & 0x01;
  
    // setup input value using pm
    

    if (kbd_val == 16){
          clear_LEDs();
          GLCD_Clear(White);
          strncpy (demo_case, "Clear Fault");
          fault_injection_reset();
          set_int(&reda,  ~reda.negated);
        GLCD_DisplayString(4, 0, 1, "Fault Clearance");
      
    }else if(kbd_val ==32){
          fault_mode = 1;
          GLCD_DisplayString(8, 0, 1, "Mode: Stuck_At_Fault");
    }else if(kbd_val == 8){
          fault_mode = 0;
          GLCD_DisplayString(8, 0, 1, "Mode: Random_Fault  ");
    }else if(kbd_val == 64){
          
          if(INT_DOUBLE ==1){
              input_int++;
              //input_int = rand();
              snprintf(tar_str, sizeof(tar_str), "Random Input: %d", input_int);
          }else if(INT_DOUBLE ==0){
              input_double += 121.121;
              snprintf(tar_str, sizeof(tar_str), "Random Input: %f", input_double);
          }
          
          GLCD_DisplayString(5, 0, 1, tar_str); 
           
          if (state_counter == 1){
            if (INT_DOUBLE ==1){
                
                set_int(&reda,  input_int);
                /*faulty generation */
                reda.value = faulty_int(reda.value, fault_mode);
                fault_injection_reset();
                if (get_int(&reda) == false){
                  GLCD_DisplayString(4, 0, 1, "Fault Detected");
                  }

                snprintf(fal_str, sizeof(fal_str), "Faulty is %2d", reda.value);
                GLCD_DisplayString(2, 0, 1, fal_str);
                  
            }else if (INT_DOUBLE==0){
                set_double(&redb, input_double);
                redb.value = faulty_double(redb.value, fault_mode);
                fault_injection_reset();
              
                    if (get_double(&redb) == false){
                      GLCD_DisplayString(4, 0, 1, "Fault Detected");
                      //break;
                    }

                snprintf(val_str, sizeof(fal_str), "proFaulty is %2f", redb.value);
                GLCD_DisplayString(2, 0, 1, val_str);
            }
            
            
        }else if (state_counter == 2){            
              
          
          
              for(i = 0; i < 3; i++){
                  newtown[i] = newtown_raphson_once(input_double)[i];}
              snprintf(val_str, sizeof(fal_str), "sqrt 1 is %f", newtown[0]);
              snprintf(tar_str, sizeof(fal_str), "sqrt 2 is %f", newtown[1]);
              snprintf(fal_str, sizeof(fal_str), "sqrt 3 is %f", newtown[2]);
              GLCD_DisplayString(2, 0, 1, val_str);
              GLCD_DisplayString(3, 0, 1, tar_str);
              GLCD_DisplayString(4, 0, 1, fal_str);
                    
              if ((abs(newtown[2] -newtown[1])) < 1e-6){
                  GLCD_DisplayString(6, 0, 1, "Results Agree");
              }else if ((abs(newtown[2] -newtown[1])) >= 1e-6){
                  GLCD_DisplayString(6, 0, 1, "Results DisAgree");
              }
        }else if(state_counter ==3){
          
                for(i = 0; i < 3; i++){
                      diverse[i] = diversCompute(input_double)[i];
                    }
          
              snprintf(yo_str, sizeof(yo_str), "sqrt is %f", diverse[0]);
              snprintf(tar_str, sizeof(tar_str), "netown is %f", diverse[1]);
              snprintf(fal_str, sizeof(fal_str), "bst is %f", diverse[2]);
              GLCD_DisplayString(2, 0, 1, yo_str);
              GLCD_DisplayString(3, 0, 1, tar_str);
              GLCD_DisplayString(4, 0, 1, fal_str);
                    
          		if ((abs(diverse[0] - diverse[1])<1e-6) || (abs(diverse[0] - diverse[2])<1e-6)){
                     GLCD_DisplayString(6, 0, 1, "Choose No.1 sqrt!");
                    
                  }else if(abs(diverse[1] - diverse[2])<1e-6){
                     GLCD_DisplayString(6, 0, 1, "Choose No.2 Newton-Raphson ");
                  }else{
                    GLCD_DisplayString(6, 0, 1, "Fault Occured");
               }
        }else if(state_counter == 4){
              if (verif(input_double) ==true){
                  GLCD_DisplayString(6, 0, 1, "Results Agree");
              }else{
                 GLCD_DisplayString(6, 0, 1, "Errors Occur ");
              }
      }
    }
      mode_switch();

  }
}
