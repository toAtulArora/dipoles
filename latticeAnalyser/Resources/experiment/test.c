#include <stdio.h>
char* program="#include <stdio.h> %cchar* program=\"%s\";%c%cvoid main()%c{%cprintf(program,10,program,10,10,10,10,10,10);%creturn0;%c}";

int main()
{
printf(program,10,program,10,10,10,10,10,10);
return 0;
}
