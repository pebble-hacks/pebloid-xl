//
//
//
#include <stdlib.h>
#include <stdbool.h>

float my_sqrt(float x)
{
	float lastGuess = x / 2.0;
	float guess;
  	while (true)
	{
    	guess = (lastGuess + x / lastGuess) / 2.0;
    	if (abs(guess - lastGuess) < .000001)
      		return guess;
    	lastGuess = guess;
	}
	return guess;
}
