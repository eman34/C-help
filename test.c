#define TESTING
#include "vm_x2017.c"
#include <string.h>

void test_commands()
{
	get_command(0x0);
}
int main(int argc, char** argv)
{
	printf("Testing command types\n");
	printf("MOV\n");
	const char * cmd_1 = get_command(0x0);
	printf("MOV %s %s\n", strcmp("MOV", cmd_1) ? " != " : " == ", cmd_1);
	
	printf("ADD\n");
	cmd_1 = get_command(0x4);
	printf("ADD %s %s\n", strcmp("ADD", cmd_1) ? " != " : " == ", cmd_1);

	printf("\nTesting commands\n");
	printf("MOV REG 0 VAL 3\n");
	carry_out(0x0,0x1,0x0, 0x0, 3);
	printf("reg[0x0] == %d\n",reg[0x0]);

	printf("\nTesting print command\n");
	printf("print value at reg[0x0]\n");
	carry_out(0x5, 0x1, 0x0, 0, 0);
	
	return 0;
}

