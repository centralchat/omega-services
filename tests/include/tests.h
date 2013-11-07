#ifndef ___TESTS_H___
#define ___TESTS_H___

#define TESTFRAME_WORK


#define EXAMPLE(message, assersion) \
	printf((message)); \	
	if (assersion) \
		printf("\t SUCCESS\n");
	else
		printf("\t FAILED\n")
#define SHOULD(block, value) \
	block \
	(value)

#define SHOULD_NOT(block, value) \
	block \
	(value)

#endif