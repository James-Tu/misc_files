int global_init_a = 0xaa;
int global_init_b = 0xbb;

int global_uninit_c;
int global_uninit_d; 

int add(int x, int y)
{
	global_uninit_c = 0xcc;
	global_uninit_d = 0xdd;

	return (x + y + global_init_a + global_init_b
			+ global_uninit_c + global_uninit_d);
}

int main()
{
	static int static_init_m = 0x11;
	static int static_init_n;
	int x = 0x22;
	int y;

	y = x + static_init_m;
	static_init_n = add(x, y);
	
	return static_init_n;
}

