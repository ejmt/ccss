int somar(int a, int b)
{
	return a + b;
}

int main()
{
	int a;
	int b;
	int c;

	a=1234;
	b=4321;
	c=somar(a, b);
	
	puts("a soma ");
	putint(a);
	puts("+");
	putint(b);
	puts("=");
	putint(c);
	puts("\n");

	return c;
}
