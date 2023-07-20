#ifdef DEBUG
#define Assert(expression) if(!(expression)){*(int*) 0 = 0;}
#else
#define Assert(expression)
#endif