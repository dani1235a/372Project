/*
 * Authors: Connor Lundberg, Daniel Ivanov
 * Date: 5/3/2017
 */
#include "Other crap/tempslc3.h"

int controller (CPU_p, int);

int displayScreen (CPU_p, int, int, int, int, int, char *);

int dialog (CPU_p cpu);

char getch ();

void setFlags (CPU_p, unsigned int, unsigned int, unsigned int);

void writeMemory(char * fileToWriteToName);


// you can define a simple memory module here for this program
//unsigned short memory[MAX_MEMORY];   // 500 words of memory enough to store simple program
int isLoaded;
int memShift;


/*
	This function is the offset6 sign extender.
*/
int sext6(int offset6) {
	if (HIGH_ORDER_BIT_VALUE6 & offset6) return (offset6 | SEXT6_SIGN_EXTEND);
	else return offset6 & SEXT6_MASK;
}


int sext11(int offset11) {
	if (HIGH_ORDER_BIT_VALUE11 & offset11) return (offset11 | SEXT11_SIGN_EXTEND);
	else return offset11 & SEXT11_MASK;
}


/*
	This function is the offset9 sign extender.
*/
int sext9(int offset9) {
	if (HIGH_ORDER_BIT_VALUE9 & offset9) return (offset9 | SEXT9_SIGN_EXTEND);
	else return offset9;
}


int sext5(int immed5) {
	if (HIGH_ORDER_BIT_VALUE5 & immed5) return (immed5 | SEXT5_SIGN_EXTEND);
	else return (immed5 & SEXT5_MASK);
}


// This is the trap function that handles trap vectors. Acts as 
// the trap vector table for now. Currently exits the HALT trap command.
int trap(CPU_p cpu, int trap_vector) {
	int value = 0;
	int i = 0;
	char temp;
	char charToPrint, carriageReturn;
	//printf("\n");
	//printf("in here\n");
	//printf("%04X\n", trap_vector);
	//printf("\n");
	switch (trap_vector) {
		case GETC:
			//printf("A GETC!\n");
			value = (int) getch();
			break;
		case OUT:
			//printf("An OUT!\n");
			//printf("%c", cpu->gotC);
			//printf("%c", cpu->gotC);
			putchar(cpu->gotC);
			break;
		case PUTS:
			//printf("A PUTS!\n");
			i = 0;
			temp = (char ) memory[(cpu->r[0] - CONVERT_TO_DECIMAL + i)];
			while ((temp)) {  
			  printf("%c", (temp));
			  i++;
			  temp = memory[(cpu->r[0] - CONVERT_TO_DECIMAL + i)];
			}
			break;
		case HALT:
			//printf("A HALT!\n");
			//printf("halt\n");
			value = 1;
			break;
	}
	//printf("\n");
	return value;
}


/*
	This is a helper function to choose which CC values to set (N, Z, or, P)
*/
void setCC(CPU_p cpu, Register Rd) {
	//printf("0x%04X\n", Rd);
	if (Rd == 0) {
		cpu->N = 0;
		cpu->Z = 1;
		cpu->P = 0;
	} else if (Rd & HIGH_ORDER_BIT_VALUE8) { // 0000 0000 1000 0000
		cpu->N = 1;
		cpu->Z = 0;
		cpu->P = 0;
	} else {
		cpu->N = 0;
		cpu->Z = 0;
		cpu->P = 1;
	}
}
	

/*
	This function sets the appropriate flags.
*/
void setFlags (CPU_p cpu, unsigned int neg, unsigned int zero, unsigned int pos) {
	cpu->N = neg;
	cpu->Z = zero;
	cpu->P = pos;
}


/*
	This function simulates the GETC trap command in assembly.
*/
char getch() {
	char buf = 0;         
	struct termios old = {0};         
	if (tcgetattr(0, &old) < 0)                 
		perror("tcsetattr()");         
	old.c_lflag &= ~ICANON;         
	old.c_lflag &= ~ECHO;         
	old.c_cc[VMIN] = 1;         
	old.c_cc[VTIME] = 0;         
	if (tcsetattr(0, TCSANOW, &old) < 0)                 
		perror("tcsetattr ICANON");        
	if (read(0, &buf, 1) < 0)                 
		perror ("read()");         
	old.c_lflag |= ICANON;         
	old.c_lflag |= ECHO;         
	if (tcsetattr(0, TCSADRAIN, &old) < 0)                 
		perror ("tcsetattr ~ICANON");         
	return (buf); 
}


/*
	This function displays the debug screen with LOAD, STEP, DISPLAY MEM, RUN, or EXIT
	commands to use.
*/
int displayScreen(CPU_p cpu, int mem, int isRunning, int stepCount, int nopCount, int collisionFound, char *stage) {
	for (int i = 0; i < 5; i++) {
		printf("\n");
	}
	
	if (!isRunning) {
		printf("\t\tWelcome to the LC-3 Simulator Simulator\n\n");
	}
	printf("\t\tRegisters \t\t    Memory\n");
	int i = START_MEM + mem;
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 0, cpu->r[0], i, memory[mem]);

	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 1, cpu->r[1], i+1, memory[1 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 2, cpu->r[2], i+2, memory[2 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 3, cpu->r[3], i+3, memory[3 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 4, cpu->r[4], i+4, memory[4 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 5, cpu->r[5], i+5, memory[5 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 6, cpu->r[6], i+6, memory[6 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 7, cpu->r[7], i+7, memory[7 + mem]);

	i = BOTTOM_HALF + mem; // replace i with the mem dump number if you want.
	printf("\t\t\t\t\t x%X: x%04X\n",i, memory[8 + mem]);
	printf("\t\t\t\t\t x%X: x%04X\n",i+1, memory[9 + mem]);
	printf("\t\t\t\t\t x%X: x%04X\n",i+2, memory[10 + mem]);
	printf("\t\tPC:x%0.4X    IR:x%04X     x%X: x%04X\n",cpu->PC,cpu->ir,i+3, memory[11 + mem]);
	printf("\t\tA: x%04X    B: x%04X     x%X: x%04X\n",cpu->A,cpu->B,i+4, memory[12 + mem]);
	printf("\t\tMAR:x%04X  MDR:x%04X     x%X: x%04X\n",cpu->MAR + CONVERT_TO_DECIMAL,cpu->MDR,i+5, memory[13 + mem]);
	printf("\t\tCC: N: %d  Z: %01d P: %d      x%X: x%04X\n",cpu->N,cpu->Z,cpu->P,i+6, memory[14 + mem]);
	printf("\t\t\t\t\t x%X: x%04X\n",i+7, memory[15 + mem]);
	printf("\t\tStep: %d  NOP Count: %d\n\n", stepCount, nopCount);
	if (collisionFound) {
		printf("Collision Detected!\n");
	} else {
		printf("\n");
	}
	printf("\nPipeLine Info:\n\n");
	if (isRunning) {
		printf("Current Stage: %s\n", stage);
	} else {
		printf("\n");
	}
	
	printf("FBUFF: PC: 0x%04X  IR: 0x%04X\n", cpu->buffers[0].PC, cpu->buffers[0].IR);
	printf("DBUFF: PC: 0x%04X  IR: 0x%04X  Opcode: %d  DR: 0x%04X  A: 0x%04X  B: 0x%04X  SEXT: 0x%04X  Stalled: %d\n", cpu->buffers[1].PC, 
			cpu->buffers[1].IR, cpu->buffers[1].Opcode, cpu->buffers[1].Rd, cpu->buffers[1].A, 
			cpu->buffers[1].B, cpu->buffers[1].SEXT, cpu->buffers[1].isStalled);
	printf("EBUFF: PC: 0x%04X  IR: 0x%04X  Opcode: %d  DR: 0x%04X  A: 0x%04X  B: 0x%04X  SEXT: 0x%04X  Stalled: %d\n", cpu->buffers[2].PC, 
			cpu->buffers[2].IR, cpu->buffers[2].Opcode, cpu->buffers[2].Rd, cpu->buffers[2].A, 
			cpu->buffers[2].B, cpu->buffers[2].SEXT, cpu->buffers[2].isStalled);
	printf("MBUFF: PC: 0x%04X  IR: 0x%04X  Opcode: %d  DR: 0x%04X  A: 0x%04X  B: 0x%04X  SEXT: 0x%04X  Stalled: %d\n", cpu->buffers[3].PC, 
			cpu->buffers[3].IR, cpu->buffers[3].Opcode, cpu->buffers[3].Rd, cpu->buffers[3].A, 
			cpu->buffers[3].B, cpu->buffers[3].SEXT, cpu->buffers[3].isStalled);	
	
	if (!isRunning) {
		printf("\n1) Load,  2) Save,  3) Step,  4) Display Memory,  5) Edit,  6) Run,  7) (Un)Set Breakpts,  8) Exit\n");
	} else {
		printf("\nPress any key to step ");
	}
	return 0;
}


/*
	This is the dialog function that provides the functionality to the choices shown in
	the displayScreen.
*/
int dialog(CPU_p cpu) {
	int opNum = 0, isRunning = 0;
	//long newValue;
	unsigned int placeInMemory;
	unsigned short newValue; 
	int dontDeleteThisGarbage = 0;
	char newMemoryValue[4];
	char * charPtr;
	char fileName[MAX_FILE_NAME];
	FILE* inputFile;
		while (opNum != EXIT) {
			scanf("%d", &opNum);
			switch (opNum) {
				case LOAD:
					printf("File Name: ");
					scanf("%s", &fileName);
					inputFile = fopen(fileName, "r");
					if (inputFile == NULL) {
						printf("DIDN'T OPEN");
						displayScreen(cpu, 0, 0, 0, 0, 0, charPtr);
						break;
					}
					int i = 0;
					fscanf(inputFile, "%04X", &dontDeleteThisGarbage);
					while (fscanf(inputFile, "%04X", &memory[i]) != EOF) {
						i++;
					}
					isLoaded = 1;
					displayScreen(cpu, 0, 0, 0, 0, 0, charPtr);
					fclose(inputFile);
					break;
				case SAVE:
					printf("Enter a file name to save to: ");
					scanf("%s", &fileName);
					writeMemory(fileName);
					displayScreen(cpu, 0, 0, 0, 0, 0, charPtr);
					break;
				case STEP:
					if (isLoaded == 1) {
						controller(cpu, 0);
						displayScreen(cpu, 0, 0, 0, 0, 0, charPtr);
						opNum = 0;
					} else {
						printf("No file loaded!");
						displayScreen(cpu, 0, 0, 0, 0, 0, charPtr);
					}
					break;
				case DISP_MEM:
					printf("Position to move to? (in hex): ");
					scanf("%4x", &memShift);
					if(memShift - START_MEM > MAX_MEMORY - DISP_BOUNDARY) {
						printf("Error: out of memory");
						memShift = 0;
						displayScreen(cpu, memShift - START_MEM, 0, 0, 0, 0, charPtr);
						break;
					} else {
						displayScreen(cpu, memShift - START_MEM, 0, 0, 0, 0, charPtr);
					}
					break;
				case EDIT:
					printf("What memory address would you like to edit: ");
					scanf("%04x", &placeInMemory);
					printf("The contents of location %04x is  %04x\n", placeInMemory - START_MEM + 1, memory[placeInMemory - START_MEM + 1]);
					printf("What would you like the new value in location %04x to be: ", placeInMemory);
					scanf("%s", &newMemoryValue);
					printf("%s\n", newMemoryValue);
					newValue = (short)strtol(newMemoryValue, &charPtr, 16);
					memory[placeInMemory - START_MEM + 1] = newValue;
					displayScreen(cpu, placeInMemory - START_MEM - 7, 0, 0, 0, 0, charPtr);
					break;
				case RUN:
					controller(cpu, 1);
					displayScreen(cpu, 0, 0, 0, 0, 0, charPtr);
					break;
				case EXIT:
					printf("Simulation Terminated.");
					break;
			}
		}
}

/*
	This method compares the current value of PC against any breakpoints entered by the user. 
*/
int encounteredBreakPont(CPU_p cpu) {
		int encountered = 0;
		int i;
		for (i = 0; i < MAX_BREAKPOINTS; i++) {
			if(cpu->PC == cpu->breakPoints[i]) {
				encountered = 1;
				i = MAX_BREAKPOINTS;
			}
		}
		return encountered; 
}

/*
	This function takes a passed breakPoint and searches the existing collection of breakpoints.
	If a match is found the breakPoint is removed from the collection. If a match isn't found the
	breakPoint is added. 
*/
void editBreakPoint(CPU_p cpu, unsigned short breakPoint) {
	int i;
	int found = 0;
	for(i = 0; i < MAX_BREAKPOINTS; i++) {
		//User wants to remove this breakpoint
		if (cpu->breakPoints[i] == breakPoint) {
			//Set spot to available value, set found variable and exit the loop;
			cpu->breakPoints[i] = AVAILABLE_BRKPT;
			i = MAX_BREAKPOINTS;
			found = 1;
		}		
	}
	
	//If this address doesn't exist find first available spot and add it to the collection of breakpoints
	if(!found) {
		for(i = 0; i < MAX_BREAKPOINTS; i++) {
		//found the first open spot
		if (cpu->breakPoints[i] == AVAILABLE_BRKPT) {
			//Add in new break point and exit the loop;
			cpu->breakPoints[i] = breakPoint;
			i = MAX_BREAKPOINTS;
		}		
	}
	}
}


void printBuffer(CPU_p cpu, struct BUFFER buff) {
	if (buff.PC == NOP) printf("PC = NOP\n");
	else printf ("PC = %04X\n", buff.PC);
	
	if (buff.IR == NOP) printf("IR = NOP\n");
	else printf ("IR = %04X\n", buff.IR);
	
	if (buff.Rd == NOP) printf("Rd = NOP\n");
	else printf("Rd = %d\n", buff.Rd);
	
	if (buff.Opcode == NOP) printf("Opcode = NOP\n");
	else printf("Opcode = %d\n", buff.Opcode);
	
	if (buff.A == NOP) printf("A = NOP\n");
	else printf("A = %04X\n", buff.A);
	
	if (buff.B == NOP) printf("B = NOP\n");
	else printf("B = %04X\n", buff.B);
	
	if (buff.SEXT == NOP) printf("SEXT = NOP\n");
	else printf("SEXT = %04X\n", buff.SEXT);
	
	printf("General CCs:\n");
	printf("N = %d\n", cpu->N);
	printf("Z = %d\n", cpu->Z);
	printf("P = %d\n", cpu->P);
}


void printAllBuffers(CPU_p cpu) {
	for (int i = 0; i < MAX_BUFFERS; i++) {
        printf("Buffer #: %d\n", i);
		printBuffer(cpu, cpu->buffers[i]);
		printf("\n");
	}
}


int checkForCollision(Register headOfPrefetch, int headPos, Register collisionCheck, int collisionPos) {
	int nopCount = 0, Rd = 0, Rs1 = 0, Rs2 = -1;
	//printf("the head POSITION!!!! = %d\n", headPos); 
	int opcode = collisionCheck >> OPCODE_SHIFT;
	if (opcode != ST && opcode != STR && opcode != STI) {
		Rd = (headOfPrefetch & DR_MASK) >> DR_SHIFT; //Gets destination register
		Rs1 = (collisionCheck & SR_MASK) >> SR_SHIFT; //Gets source register 1
		
		Rs2 = -1;
			//printf("the head POSITION!!!!2 = %d\n", headPos); 

		if (!((collisionCheck & HIGH_ORDER_BIT_VALUE5) || (collisionCheck & HIGH_ORDER_BIT_VALUE6) 
			|| (collisionCheck & HIGH_ORDER_BIT_VALUE8) || (collisionCheck & HIGH_ORDER_BIT_VALUE9))) {   //Checks if this value is a sext.
			Rs2 = collisionCheck & SR2_MASK; //Gets the source register 2 if not a sext.
		}
	} else {
		Rd = (collisionCheck & DR_MASK) >> DR_SHIFT; //Gets destination register
		Rs1 = (headOfPrefetch & SR_MASK) >> SR_SHIFT; //Gets source register 1

		
		Rs2 = -1;
			//printf("the head POSITION!!!!2 = %d\n", headPos); 

		if (!((headOfPrefetch & HIGH_ORDER_BIT_VALUE5) || (headOfPrefetch & HIGH_ORDER_BIT_VALUE6) 
			|| (headOfPrefetch & HIGH_ORDER_BIT_VALUE8) || (headOfPrefetch & HIGH_ORDER_BIT_VALUE9))) {   //Checks if this value is a sext.
			Rs2 = headOfPrefetch & SR2_MASK; //Gets the source register 2 if not a sext.
		}
	}
	
	
	if (Rd == Rs1 || (Rs2 >= 0 && Rd == Rs2)) {  //Checks if Rd is Rs1 or that Rd is Rs2 as long as Rs2 is not a sext.
		nopCount = 4 - (collisionPos - headPos);
	}
		
	if (opcode == BR) {
		nopCount = 2;
	}
	return nopCount;
}

void printPrefetch (CPU_p cpu) {
	printf("\n");
	for (int i = 0; i < PREFETCH_SIZE; i++) {
		printf("\t 0x%04X\n", cpu->prefetch.values[i]);
	}
	printf("\n");
}


/*
	This is our predecode that determines the next value that will be passed into the IR.
*/
Register predecode (CPU_p cpu) {

	Register returnValue = 0;
	int collision = 0, i = cpu->PC, j = 0;
	cpu->prefetch.collisionFound = 0;
	if (cpu->prefetch.head >= 8) {
			//printf("or here\n");

		//printPrefetch(cpu);
		for (; i < cpu->PC + 8; i++, j++) {
			
			cpu->prefetch.values[j] = memory[i - CONVERT_TO_DECIMAL];
		}		
		cpu->prefetch.head = 0;
		//printPrefetch(cpu);
		cpu->prefetch.nopCount = 79;
		returnValue = NOP;
	}	else if (cpu->prefetch.nopCount > 0) {
		//printf("\n %d \n", cpu->prefetch.nopCount);
			//printf("here\n");

		returnValue = NOP;
	} else {
		//printf("or or here\n");
		for (int i = cpu->prefetch.head + 1; i < cpu->prefetch.head + 4 && i < PREFETCH_SIZE; i++) {
			//printf("head = %d\n", cpu->prefetch.head);
			collision = 5 * checkForCollision(cpu->prefetch.values[cpu->prefetch.head], 
						cpu->prefetch.head, cpu->prefetch.values[i], i);
			//printf("\nThe collision is: %d\n", collision);
            if (collision) {
				//printf("collision found\n");
				cpu->prefetch.collisionFound = 1;
				cpu->prefetch.nopCount = collision;
				break;
			}
		}
		returnValue = cpu->prefetch.values[cpu->prefetch.head];
		cpu->prefetch.head++;
	}
	
	return returnValue;
}


struct BUFFER initBuffer() {
	struct BUFFER buff;
	
	buff.PC = NOP;
	buff.IR = NOP;
	buff.Rd = NOP;
	buff.Opcode = NOP;
	buff.A = NOP;
	buff.B = NOP;
	buff.SEXT = NOP;
	buff.isStalled = 0;
	buff.N = 0;
	buff.Z = 0;
	buff.P = 0;
	
	return buff;
}


/*
	This is the main controller for the program. It takes a CPU struct and an int
	which is being used as a boolean for displaying the screen.
*/
int controller (CPU_p cpu, int isRunning) {
	unsigned int state;
	short cc;
	unsigned int opcode, Rd, Rs1, Rs2, immed_offset, BaseR;	// fields for the IR
	char charToPrint = ' ';
	char *temp = malloc(sizeof(char) * 8);
	int value = 0, stepCounter = 1;
    state = STORE;
	int j;
	Register predecodeValue = 0;
	
	if(isRunning && encounteredBreakPont(cpu)) {
		isRunning = 0;
	}
    for (;;) {
		//printf("here\n");
		if (!isRunning) {
			displayScreen(cpu, 0, 1, stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, temp);
			//scanf("%c", &charToPrint);
			getch();
		}
        switch (state) {
			case STORE: // Look at ST. Microstate 16 is the store to memory
				strcpy(temp, "STORE");
			
				switch (cpu->buffers[3].Opcode) {
					case ADD:
						cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
						setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
						break;
					case AND:
						cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
						setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
						break;
					case NOT:
						cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
						setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
						break;
					case LDI:
					case LDR:
					case LD:
						cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].B;
						//may need to change the buffer being passed here
						setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
						break;
					case LEA:
						cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
						setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
						break;
						break;
					case TRAP:
						if (cpu->buffers[3].SEXT == GETC) {
							cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
						}
						break;
					case NOP:
						break;
				}
//                cpu->buffers[3] = cpu->buffers[2];
				state = MEM;
				
				break;
                //state = FETCH;
			case MEM:
				strcpy(temp, "MEM");
				//printf("MEM   %d, %d, General PC = 0x%04X\n", stepCounter, cpu->prefetch.nopCount, cpu->PC);
				//may need to add in a check if the last buffer's PC is NOP (same as other states).
                if (cpu->buffers[1].isStalled) {
					//printf("DBUFF is still stalled\n");
                    cpu->prefetch.head = PREFETCH_SIZE;
					
					cpu->buffers[0] = initBuffer();
					cpu->buffers[1] = initBuffer();
                } else {
					cpu->buffers[3] = cpu->buffers[2];
                    switch (cpu->buffers[3].Opcode) {
                        case ST:
							memory[cpu->buffers[3].A - CONVERT_TO_DECIMAL] = cpu->r[cpu->buffers[3].B];
							break;
                        case STR:
							//printf("inside STR\n");
							//printf("A: 0x%04X, B: 0x%04X, A - 0x%04X: 0x%04X\n", cpu->buffers[3].A, cpu->buffers[3].B, CONVERT_TO_DECIMAL, cpu->buffers[3].A - CONVERT_TO_DECIMAL);
							//printf("memory[0x%04X] = 0x%04X\n", cpu->buffers[3].A - CONVERT_TO_DECIMAL, cpu->buffers[3].B);
							memory[cpu->buffers[3].A - CONVERT_TO_DECIMAL] = cpu->buffers[3].B;
							//printf("%c\n", (char) memory[cpu->buffers[3].A - CONVERT_TO_DECIMAL]);
							break;
                        case STI:  //not sure of how to handle STI yet
							break;
                        case LD:
							cpu->buffers[3].B = memory[cpu->buffers[3].A - CONVERT_TO_DECIMAL];
							break;
                        case LDR:
							cpu->buffers[3].B = memory[cpu->buffers[3].A - CONVERT_TO_DECIMAL];
							break;
                        case LDI:
							cpu->buffers[3].A = memory[(cpu->buffers[3].PC + cpu->buffers[3].A)
																- CONVERT_TO_DECIMAL];
							//this will need to call LD afterwards
							break;
                    }
                    //Do Mem work with EBuff (buffers[2]) here first.
                    //cpu->buffers[3].A = cpu->alu.A;
                    //cpu->buffers[3].B = cpu->alu.B;
                }
                state = EXECUTE;
                break;
			case EXECUTE: // Note that ST does not have an execute microstate
				strcpy(temp, "EXECUTE");
				//printf("in EXECUTE\n");
				//printf("EXECUTE   %d, %d, General PC = 0x%04X\n", stepCounter, cpu->prefetch.nopCount, cpu->PC);
				if (cpu->buffers[1].PC == NOP) {
					cpu->buffers[2] = cpu->buffers[1];
				} else {
					//printf("The buffer[1] B is = 0x%04X\n", cpu->buffers[1].B);
					cpu->buffers[2] = cpu->buffers[1];
					//printf("The buffer[2] B is = 0x%04X\n", cpu->buffers[2].B);

					/*cpu->buffers[2].PC = cpu->buffers[1].PC;
					cpu->buffers[2].Rd = cpu->buffers[1].Rd;
					cpu->buffers[2].Opcode = cpu->buffers[1].Opcode;*/
					switch (cpu->buffers[1].Opcode) {
						case ADD:
							//printf("A: 0x%04X\n", cpu->buffers[2].A);
							//printf("B: 0x%04X\n", cpu->buffers[2].B);

							if (cpu->buffers[2].A & HIGH_ORDER_BIT_VALUE15) {
								cpu->buffers[2].A = (cpu->buffers[2].A) + (cpu->buffers[2].B);//same reason as below
							} else if (cpu->buffers[1].B & HIGH_ORDER_BIT_VALUE15) {
								cpu->buffers[2].A = (cpu->buffers[2].A) + (cpu->buffers[2].B); //need to add here because B is negative
							} else if ((cpu->buffers[2].A & HIGH_ORDER_BIT_VALUE15) && (cpu->buffers[2].B & HIGH_ORDER_BIT_VALUE15)) {
								cpu->buffers[2].A = -(cpu->buffers[2].A) - (cpu->buffers[2].B);
							} else {
								cpu->buffers[2].A = (cpu->buffers[2].A) + (cpu->buffers[2].B);
							}
							break;
						case AND:
							cpu->buffers[2].A = cpu->buffers[2].A & cpu->buffers[2].B;
							break;
						case NOT:
							cpu->buffers[2].A = ~(cpu->buffers[2].A);
							break;
						case TRAP:
							//cpu->buffers[2].PC = cpu->MDR;
							cpu->r[7] = cpu->buffers[2].PC;
							//printf("It's a TRAP!\n");
							value = trap(cpu, cpu->buffers[2].B);
							//cpu->buffers[2].PC = cpu->r[7];
							//start NOP stall
							//printf ("Our TRAP value is = %d\n", value);
							if (value == 1) {
								//displayScreen(cpu, 0);
								return 0;
							} else if (value > 1) {
								cpu->buffers[2].A = (char) value;
								cpu->gotC = (char) value;
							}
							break;
						case JSRR:
							cpu->r[7] = cpu->buffers[2].PC;
							if (cpu->buffers[2].IR & HIGH_ORDER_BIT_VALUE12) {
								cpu->PC = cpu->buffers[2].PC + cpu->buffers[2].A;
							} else {
								cpu->PC = cpu->buffers[2].A;
							}
							cpu->buffers[1].isStalled = 1;
							cpu->prefetch.head = 8;
							//start NOP stall
							break;
						case JMP:
							//cpu->r[7] = cpu->buffers[2].PC;
							cpu->PC = cpu->r[cpu->buffers[2].A];
							cpu->buffers[1].isStalled = 1;
							cpu->prefetch.head = 8;
							break;
						case BR:
							//printf("made it to branch\n");
							if (cpu->N && (cpu->buffers[2].Rd & 4)) {
								cpu->PC = cpu->buffers[2].PC + sext9(cpu->buffers[1].SEXT);
								//printf("the new general PC = 0x%04X\n", cpu->PC);
	//                            cpu->buffers[0].isStalled = 1;
								cpu->buffers[1].isStalled = 1;
								cpu->prefetch.head = 8;
								break;
							}
							//printf("passed over N\n");
							if (cpu->Z && (cpu->buffers[2].Rd & 2)) {
								cpu->PC = cpu->buffers[2].PC + sext9(cpu->buffers[1].SEXT);
								//printf("the new general PC = 0x%04X\n", cpu->PC);
	//                            cpu->buffers[0].isStalled = 1;
								cpu->buffers[1].isStalled = 1;
								cpu->prefetch.head = 8;
								break;
							}
							//printf("passed over Z\n");
							if (cpu->P && (cpu->buffers[2].Rd & 1)) {
								cpu->PC = cpu->buffers[2].PC + sext9(cpu->buffers[1].SEXT);
								//printf("the new general PC = 0x%04X\n", cpu->PC);
	//                            cpu->buffers[0].isStalled = 1;
								cpu->buffers[1].isStalled = 1;
								cpu->prefetch.head = 8;
								break;
							}
							//printf("passed over P\n");
							break;
						case NOP:
							break;
						case LEA:
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
						case LDR:
							cpu->buffers[2].A = cpu->buffers[2].A + sext6(cpu->buffers[2].SEXT);
							break;
						case LD:
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
						case LDI:
							cpu->buffers[2].A = (cpu->buffers[2].PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[2].SEXT);
							break;
						case ST:
							//cpu->buffers[2].B = cpu->buffers[2].A;
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
						case STR:
							//cpu->buffers[2].B = cpu->buffers[2].A;
							cpu->buffers[2].A = cpu->r[BaseR] + sext6(cpu->buffers[2].SEXT);
							break;
						case STI:
							//printf("STI\n");
							//cpu->buffers[2].B = cpu->buffers[2].A;
							cpu->buffers[2].A = (cpu->buffers[2].PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[2].SEXT);
							break;
					}
				}
                if(cpu->buffers[1].isStalled) {
                    state = STORE;
                } else {
                    state = IDRR;
                }
                break;
			case IDRR:
				strcpy(temp, "IDRR");
				//printf("IDRR   %d, %d, General PC = 0x%04X\n", stepCounter, cpu->prefetch.nopCount, cpu->PC);
				if (cpu->buffers[0].PC == NOP) {
					cpu->buffers[1] = cpu->buffers[0];
				} else {			
					cpu->buffers[1].PC = cpu->buffers[0].PC;
					cpu->buffers[1].IR = cpu->buffers[0].IR;
					cpu->buffers[1].Opcode = cpu->buffers[0].IR >> OPCODE_SHIFT;			//Decode Stage
					cpu->buffers[1].Rd = cpu->buffers[0].IR & DR_MASK;
					cpu->buffers[1].Rd = (short)cpu->buffers[1].Rd >> DR_SHIFT;
					cpu->buffers[1].A = cpu->buffers[0].IR & SR_MASK;
					cpu->buffers[1].A = (short)cpu->buffers[1].A >> SR_SHIFT;
					cpu->buffers[1].B = cpu->buffers[0].B & SR2_MASK;
					//printf("SR2 = %d\n", cpu->buffers[1].B);
					cpu->buffers[1].SEXT = cpu->buffers[0].IR & SR_MASK;
					//immed_offset = cpu->buffers[0].IR & SR_MASK;
					BaseR = (cpu->buffers[0].IR & BASE_MASK) >> SR_SHIFT;
					//printf("here\n");
					//IDRR Buffer
	//				printBuffer(cpu->buffers[1]);
					switch (cpu->buffers[1].Opcode) {							//Evaluate Address Stage
						/*case LDR:
							//printf("LDR\n");
							//printf("%04X = (%04X + %04X) - %d\n", (cpu->r[BaseR] + sext6(cpu->buffers[1].SEXT)) - CONVERT_TO_DECIMAL, cpu->r[BaseR], sext6(cpu->buffers[1].SEXT), CONVERT_TO_DECIMAL);

							cpu->buffers[1].A = (cpu->r[BaseR] + sext6(cpu->buffers[1].SEXT)) - CONVERT_TO_DECIMAL;
							//printf("Yo dog");
							break;
						case LD:
							//printf("LD\n");
							cpu->buffers[1].A = (cpu->buffers[1].PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
							break;
						case LDI:
							//printf("LDI\n");
							cpu->buffers[1].A = (cpu->buffers[1].PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
							break;
						case LEA:
							cpu->buffers[1].A = (cpu->buffers[1].PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
							break;
						case ST:
							//printf("ST\n");
							cpu->buffers[1].B = cpu->buffers[1].A;
							cpu->buffers[1].A = (cpu->buffers[1].PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
							break;
						case STR:
							//printf("STR\n");
							cpu->buffers[1].B = cpu->buffers[1].A;
							cpu->buffers[1].A = (cpu->r[BaseR] - CONVERT_TO_DECIMAL) + sext6(cpu->buffers[1].SEXT);
							break;
						case STI:
							//printf("STI\n");
							cpu->buffers[1].B = cpu->buffers[1].A;
							cpu->buffers[1].A = (cpu->buffers[1].PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
							break;*/
						case TRAP:
							//printf("TRAP\n");
							cpu->buffers[1].B = cpu->buffers[1].SEXT;
							//printf("Our B value is = 0x%04X\n", cpu->buffers[1].B);
							//printf("TRAP\n");
							//printf("%04X\n", cpu->buffers[1].B);
							break;
						case NOP:
							//printf("NOP\n");
							break;
					}

					switch (cpu->buffers[1].Opcode) {							//Fetch Operand Stage
						/*case LDR:
						case LD:
							//printf("LD/LDR\n");
							//printf("%04X\n", cpu->MAR);
							cpu->buffers[1].B = memory[cpu->buffers[1].A];
							break;
						case LDI:
							//printf("LDI\n");
							cpu->buffers[1].B = memory[cpu->buffers[1].A];
							break;*/
						case ADD:
							//printf("ADD\n");
							if(HIGH_ORDER_BIT_VALUE6 & cpu->buffers[1].IR){ //0000|0000|0010|0000
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								cpu->buffers[1].B = sext5(cpu->buffers[1].SEXT);
								//printf("%04X\n", cpu->buffers[1].B);
							} else{
								//change this
								//printf("in here\n");
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								//printf("SR2 = %d\n", cpu->buffers[1].B);
								cpu->buffers[1].B = cpu->r[cpu->buffers[1].B];
								//printf("Value = 0x%04X\n", cpu->buffers[1].B);
							}

							break;
						case AND:
							//printf("AND\n");
							if(HIGH_ORDER_BIT_VALUE6 & cpu->buffers[1].IR){ //0000|0000|0010|0000
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								cpu->buffers[1].B = (cpu->buffers[1].SEXT & SEXT5_MASK);
							} else{
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								cpu->buffers[1].B = cpu->r[cpu->buffers[1].B];
							}
							break;
						case NOT:
							//printf("NOT\n");
							cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
							break;
						case JSRR:
							if (cpu->buffers[1].IR & HIGH_ORDER_BIT_VALUE12) {
								cpu->buffers[1].A = cpu->buffers[1].IR & SEXT11_MASK;
							} else {
								cpu->buffers[1].A = cpu->r[BaseR];
							}
							break;
						case LDR:
							cpu->buffers[1].A = cpu->r[BaseR];
							break;
						case STR:
						case ST:
							//printf("ST/STR\n");
							cpu->buffers[1].B = cpu->r[cpu->buffers[1].Rd];
							break;
						case STI:
							//printf("STI\n");
							cpu->buffers[1].B = memory[cpu->buffers[1].A];
							break;
						case NOP:
							//printf("NOP2\n");
							break;
					}
				}
				//cpu->buffers[2] = cpu->buffers[1];
				//cpu->buffers[1] = cpu->buffers[0];
				//cpu->buffers[1].A; //might need to take these out
				//cpu->buffers[1].B;
                state = FETCH;
				break;
            case FETCH: // microstates 18, 33, 35 in the book
				strcpy(temp, "FETCH");
				//printf("FETCH   %d, %d, General PC = 0x%04X\n", stepCounter, cpu->prefetch.nopCount, cpu->PC);
				predecodeValue = predecode(cpu);
				if (predecodeValue != NOP) {
					cpu->buffers[0].A = (cpu->PC - CONVERT_TO_DECIMAL);
					cpu->PC++;	// increment PC
					cpu->buffers[0].B = memory[cpu->buffers[0].A];
					cpu->ir = cpu->buffers[0].B;
					cc = 0;
					cpu->buffers[0].PC = cpu->PC;  //IF Buffer
					cpu->buffers[0].IR = predecodeValue;
				} else {
                    cpu->buffers[0].A = NOP;
                   // cpu->PC++;	// increment PC
                    cpu->buffers[0].B = NOP;
                    cpu->ir = NOP;
                    cc = 0;
                    cpu->buffers[0].PC = NOP;  //IF Buffer
                    cpu->buffers[0].IR = NOP;
                }
               // state = IDRR;     
					//break;
                
                //state = STORE;
				//cpu->buffers[1] = cpu->buffers[0];
				state = STORE;
                break;
        }
		//printAllBuffers(cpu);
		
		stepCounter++;
		cpu->prefetch.nopCount--;

    }
	
}

/*
	This function initializes the cpu fields
*/
void cpuInit(CPU_p cpu) {
	cpu->r[0] = 0x0000;
	cpu->r[1] = 0x0000;
	cpu->r[2] = 0x0000;
	cpu->r[3] = 0x0000;
	cpu->r[4] = 0x0000;
	cpu->r[5] = 0x0000;
	cpu->r[6] = 0x0000;
	cpu->r[7] = 0x0000;
	cpu->ir = 0x0000;
	cpu->PC = START_MEM;
	cpu->MAR = 0x0000;
	cpu->MDR = 0x0000;
	cpu->A = 0x0000;
	cpu->B = 0x0000;
	cpu->N = 0;
	cpu->Z = 0;
	cpu->P = 0;
	cpu->breakPoints[0] = AVAILABLE_BRKPT;
	cpu->breakPoints[1] = AVAILABLE_BRKPT;
	cpu->breakPoints[2] = AVAILABLE_BRKPT;
	cpu->breakPoints[3] = AVAILABLE_BRKPT;
	cpu->prefetch.head = 8;
	cpu->prefetch.nopCount = 0;
	for (int i = 0; i < PREFETCH_SIZE; i++) {
		cpu->prefetch.values[i] = NOP;
	}
	
	for (int i = 0; i < MAX_MEMORY; i++) {
		memory[i] = NOP;
	}
	
	for (int i = 0; i < MAX_BUFFERS; i++) {
		cpu->buffers[i] = initBuffer();
	}
	//printAllBuffers(cpu);
}

//returns 1 if true 0 if false
int checkIfFileExists(char* fileToCheckIfExists) {
	FILE* filePtr;
	filePtr = fopen(fileToCheckIfExists, "r+");
	if(filePtr != NULL) {
		fclose(filePtr);
		return 1;
	} else {
		return 0;
		fclose(filePtr);
	}
}

//need to #define TRAP25 61477;
void writeMemory(char * fileToWriteToName) {
	FILE * filePtr;
	int TRAP25 = 61477;
	unsigned int memoryStart, memoryEnd; 
	//the file exists so promt the user to see if they 
	//are ok with overwritting the preexisting file right here
	//include if/else statement to check user decision for overwriting
	if(checkIfFileExists(fileToWriteToName)) {

		filePtr = fopen(fileToWriteToName, "w");
		for(int i=memoryStart + 1; i <= memoryEnd; i++) {
			printf("i = %i i = x%04x\n", i, memory[i - START_MEM]);
			fprintf(filePtr, "%04x\n", memory[i - START_MEM]);
		}
		fclose(filePtr);

	//the file doesn't exist so create the new file and write to it
	} else {
		FILE * filePtr;
		filePtr = fopen(fileToWriteToName, "w");
		printf("Enter the beginning and end of the memory to save: ");
		scanf("%4x %4x", &memoryStart, &memoryEnd);
		printf("start = %4x end = %4x\n", memoryStart, memoryEnd);
		for(int i=memoryStart + 1; i <= memoryEnd; i++) {
			printf("i = %i i = x%04x\n", i, memory[i - START_MEM]);
			fprintf(filePtr, "%04x\n", memory[i - START_MEM]);
		}
		fclose(filePtr);
	}
}


/*
	This is the main function that starts the program off.
*/
int main(int argc, char* argv[]){

	//setvbuf(stdout, NULL, _IONBF, 0);
	isLoaded = 0;
	memShift = 0;
	char *temp;
	CPU_p cpu = malloc(sizeof(CPU_s));
	cpuInit(cpu);
	displayScreen(cpu, memShift, 0, 0, 0, 0, temp);
	dialog(cpu);
	return 0;
}