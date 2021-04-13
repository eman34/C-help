#include <stdio.h>
#include <stdlib.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

//all values are 1 byte
#define BYTE unsigned char
//ram 256 address of 1 byte each
BYTE memory[256];
//registers
enum my_registers
{
	//general purpose registers
	reg_0 = 0x00,
	reg_1,
	reg_2,
	reg_3,
	//personal use registers
	reg_4, // check if the program is running
	reg_5,
	reg_6,
	//program counter
	reg_7,
};
//8 registers of 1 byte
BYTE reg[8];
//initilaize running check to true
//reg[reg_4] = 1;
//instruction sets
enum op_codes
{
	op_mov = 0x000,// copy value
	op_cal, //call another function
	op_ret, //terminate the current function
	op_ref, // store stack address
	op_add, // add two values
	op_print, // print
	op_not, //bitwise not operation
	op_equ //test if register value is equal to 0
};
// value types
enum types
{
	type_val = 0x0, //the value in proceeding is 8 bits single value
	type_register, // address of one of the 8 fixed registers
	type_stack, // particular stack symbol
	type_pointer // pointer variable
};
int file_size = 0; //file size in bytes
int end_index = 0; //this is where our entry point begins
BYTE main_function_index = -1; // this is the index in memory where the main function begins

void read_file(FILE* file);
void add(int value_1, int value_2);
const char * get_command(BYTE val);
const char * get_storage_type(BYTE val);
void parse(FILE* file);
void run();
void carry_out(BYTE cmd, BYTE type_1,BYTE  val_1, BYTE type_2, BYTE val_2);

#ifndef TESTING
int main( int argc, char** argv )
{
	// load file contents
	if( argc < 2 )
	{
		printf("Usage: vm_x2017 <x2017_binary>");
		exit(1);
	}
	FILE* file = fopen(argv[1], "rb");
	if(!file)
	{
		printf("file not found");
		exit(1);
	}
	read_file(file);
	parse(file);
	//close file
	fclose(file);
	
}
#endif
void read_file(FILE* file)
{
	int ch;
	for(int i = 0; i< sizeof(memory) && ((ch = fgetc(file)) != EOF); i++)
	{
		memory[i] = ch;
		file_size = i+1;
	}
	//parse();

}
void parse(FILE* file)
{
	//lets get the file size
	fseek(file, 0L, SEEK_END);
	int file_size = ftell(file);

	int end_set = 0, mem_index = -1;
	int counter = 0, ins = 0;
	BYTE mask_1 = 0x80; // 1000 0000
	BYTE mask_2 = 0x00; // 0000 0000
	BYTE val = 0x0, val_command = 0x0, val_type_1 = 0x0, value = 0x0, label = 0x0;
	int bits_pushed = 0, bits_pushed_command = 0, bits_pushed_type_1 = 0, bits_pushed_val = 0, bits_to_get = 0, bits_pushed_label = 0;
	//we start the file from the end
	for(int c = file_size - 1; c >=0; c-- )
	{
		//get a single byte
		fseek(file, -1l, SEEK_CUR);
		int ch = fgetc(file), ch_before;
		fseek(file, -1L, SEEK_CUR);
		//printf("\n BYTE "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(ch));
		//BYTE ch = memory[c], ch_before;
		if(c >= 1)
		{
			ch_before = fgetc(file);
			//move back one character
			fseek(file, -1L, SEEK_CUR);
		}
	
		//if( c >= 1 )
			//ch_before = memory[ c - 1 ];
		for( int d = 0; d<8; d++ )
		{

			//printf("%d", ch & 0x1);
			////check if we are ending
			if(c == 0 && !end_set )
			{
				end_index = end_index < d ? d : end_index;
			}
			else if( c== 1 )
			{
				if(d < 3)
				{
					end_index = end_index < d ? d : end_index;
					end_set = 1;
				}
			}

			if(counter == 0 && ins == 0)
			{
				//how many instructions 
				val >>= 1;
				int bit = (ch & 0x1);
				if( bit == 1 )
					val = val | mask_1;
				else
					val |= mask_2;
				bits_pushed += 1;
				if(bits_pushed == 5)
				{
					val >>= 3;
					ins = val;
					//printf ("\n\n"BYTE_TO_BINARY_PATTERN"  # Number of instructions in the function", BYTE_TO_BINARY(val));
					counter = 1;
					bits_pushed = 0;
					//mem_index += 1;
					//memory[mem_index] = val;
				}
				//lets check if this is the begining function 
				//it is characterized by the remaining 
				
			}
			else if(counter == 1)
			{
				//parse command
				val_command >>= 1;
				int bit = (ch & 0x1);
				if(bit == 1)
					val_command |= mask_1;
				else
					val_command |= mask_2;				
				bits_pushed_command += 1;
				if(bits_pushed_command == 3)
				{
					val_command >>= 5;
					//printf("\n"BYTE_TO_BINARY_PATTERN" #%s", BYTE_TO_BINARY(val_command), get_command(val_command));
					if( val_command != op_ret )
					{
						counter = 2;
						ins -= 1;
						if(val_command == op_cal || val_command == op_print || val_command == op_not || val_command == op_equ )
						{
							//the command has only one variable therefore create a space to store them
							mem_index += 3;
						}
						else
							mem_index += 5;

					}
					else
						mem_index += 1; //the command doesn't have any variables
					//store the command in memory
					memory[mem_index] = val_command;
					//printf("\nmem index %d", mem_index);

					bits_pushed_command = 0;
				}	
				
			}
			//type
			else if(counter == 2 || counter == 4)
			{
				//parse type
				val_type_1 >>= 1;
				int bit = (ch & 0x1);
				if(bit == 1)
					val_type_1 |= mask_1;
				else
					val_type_1 |= mask_2;				
				bits_pushed_type_1 += 1;
				if(bits_pushed_type_1 == 2)
				{
					val_type_1 >>= 6;
					//printf("\t\t type "BYTE_TO_BINARY_PATTERN" #%s", BYTE_TO_BINARY(val_type_1), get_storage_type(val_type_1));
					counter += 1;
					// set the bits to get
					if( val_type_1 == type_val  )
						bits_to_get = 8;
					else if( val_type_1 == type_register )
						bits_to_get = 3;
					else 
						bits_to_get = 5;
					bits_pushed_type_1 = 0;
					//push the type into memory
					mem_index -= 1;
					memory[mem_index] = val_type_1;
					//printf("\nmem index %d", mem_index);

				}
				
			}
			else if(counter == 3 || counter == 5)
			{
				//parse value
				value >>= 1;
				int bit = (ch & 0x1);
				if(bit == 1)
					value |= mask_1;
				else
					value |= mask_2;
				bits_pushed_val += 1;
				if(bits_pushed_val == bits_to_get)
				{
					value >>= (8-bits_to_get);
					//printf("\t\tValue "BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY( value ));
					//push the value into memory
					mem_index -= 1;
					memory[ mem_index ] = value;
					//if command is unary return 
					//command is one value one type
					if(val_command == op_cal || val_command == op_print || val_command == op_not || val_command == op_equ )
					{
						if(ins == 1)
							counter = 6;
						else 
							counter = 1;
						//push the memory back
						mem_index += 2;
					}
					else
					{
						if(counter == 5)
						{
							if(ins == 1)
								counter = 6;
							else 
								counter = 1;
							//push the memory back
							mem_index += 4;
						}
						else
							counter += 1;
					}
					bits_pushed_val = 0;
					//printf("\nmem index %d", mem_index);
					
					
				}
				
				
			}
			else if( counter == 6 )
			{
				//function label
				label >>= 1;
				int bit = (ch & 0x1);
				if(bit == 1)
					label |= mask_1;
				else
					label |= mask_2;
				bits_pushed_label += 1;
				if(bits_pushed_label == 3)
				{
					label >>= 5;
					counter = 0;
					//printf("\nFUNC LABEL "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(label));
					mem_index += 1;
					memory[mem_index] = label;
					if(label == 0x0)
					{
						main_function_index = mem_index;
						//printf("\n main function found");
					}
					//printf("\nmem index %d", mem_index);
				}

			}
			//we need to make sure the bytes in ch are also flowing 
			ch >>= 1;
			int ch_bit = (ch_before & 0x1);
			if(ch_bit == 1)
				ch |= mask_1;
			else
				ch |= mask_2;
			ch_before >>= 1;

		}

	}
	end_index = 8 - end_index + 1;
	//printf("\nstarting index is %d\nmemory index is %d\n",  end_index, mem_index);
	run();
}
void run()
{
	int count = -1;
	for(int index = main_function_index; index >= 0; index--)
	{
		BYTE ch = memory[index], ch_command = 0x0, ch_type_1 = 0x0, ch_type_2 = 0x0, ch_val_1 = 0x0, ch_val_2 = 0x0;
		BYTE ch_previous_command = 0x0;
		if( count == -1 )
		{
			printf("\nFUNC LABEL %d", ch);
			count = 0;
			index -= 1;
			//printf(" "BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(ch));
			//we are at a new function therefore we can remove the previous command 
			ch_previous_command = 0x0;
		}
		if( count == 0 )
		{
			//at command
			ch_command = memory[index];
			printf("\n\t%s ", get_command(ch_command));
			count += 1;
			index -= 1;
			//check if the command is return
			if( ch_command == op_ret )
			{
				// we need to skip the remain code
				ch_previous_command = ch_command;
				count = -1;
			}
				
			//printf(" "BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(ch));
		}
		if( count == 1 )
		{
			//at first type
			ch_type_1 = memory[index];
			printf("%s ", get_storage_type(ch_type_1));
			count += 1;
			index -= 1;
			//printf(" "BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(ch));
		}
		if( count == 2 )
		{
			//at first value
			ch_val_1 = memory[index];
			printf("%d ", ch_val_1);
			//check if we need to proceed
			if(ch_command == op_cal || ch_command == op_print || ch_command == op_not || ch_command == op_equ )
			{
				//go back to command
				count = 0;
			}
			else
				count += 1;
			index -= 1;
			//printf(" "BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(ch));
		}
		if( count == 3 )
		{
			//at second type
			ch_type_2 = memory[index];
			printf("%s ", get_storage_type(ch_type_2));
			count += 1;
			index -= 1;
			//printf(" "BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(ch));
			
		}
		if( count == 4 )
		{
			//at second value
			ch_val_2 = memory[index];
			printf("%d ",ch_val_2);
			// go back to command
			count = 0;
			//printf(" "BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(ch));
		}
		//carry out command
		//check if we are breaking
		if( ch_previous_command == op_ret )
		{

		}
		else 
			carry_out(ch_command, ch_type_1, ch_val_1, ch_type_2, ch_val_2);
	}
	printf("\n");
}
void carry_out(BYTE cmd, BYTE type_1,BYTE  val_1, BYTE type_2, BYTE val_2)
{
	switch(cmd)
	{
		case op_mov:
			{
				if(type_1 == type_register)
				{
					reg[val_1] = val_2;
				}
				break;
			}
		case op_cal:
			{

			}
			break;
		case op_ret:
			{

			}
			break;
		case op_ref:
			{

			}
			break;
		case op_add:
			{
				if(type_1 == type_register && type_2 == type_register)
				{
					reg[val_1] += reg[val_2];
				}
			}
			break;
		case op_print:
			{
				if(type_1 == type_register)
					printf("\n%d\n",reg[val_1]);
			}
			break;
		case op_not:
			{
				reg[val_1] = ~reg[val_1];
			}
			break;
		case op_equ:
			{
				if(reg[val_1]== 0x0)
					reg[val_1] = 0x1;
				else
					reg[val_1] = 0x0;
			}
			break;
		default:
			{
				exit(1);
			}
	}
}
const char * get_command(BYTE val)
{
	switch(val)	{
		case op_mov:
			return "MOV";
		case op_cal:
			return "CAL";
		case op_ret:
			return "RET";
		case op_ref:
			return "REF";
		case op_add:
			return "ADD";
		case op_print:
			return "PRINT";
		case op_not:
			return "NOT";
		case op_equ:
			return "EQU";
		default:
			return "error";
	}
	
}
const char * get_storage_type(BYTE val){
	switch (val){
		case type_val:
			return "VAL";
		case type_register:
			return "REG";
		case type_stack:
			return "STK";
		case type_pointer:
			return "PTR";
		default:
			return "error";
	}
}
