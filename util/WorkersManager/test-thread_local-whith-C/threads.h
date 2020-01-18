#ifndef THREADS
#define THREADS

#ifdef __main__
		thread_local unsigned int id = 0;
	#else //__main__
		#ifndef C_CODE
			extern thread_local unsigned int id;
		#else //C_CODE
			extern thread_local unsigned int id;//TODO Fail
		#endif //C_CODE
#endif //__main__

#endif
