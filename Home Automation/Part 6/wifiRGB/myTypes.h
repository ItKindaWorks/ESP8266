#ifndef myTypes_h
#define myTypes_h

typedef struct lightState{
	double hue;		//HSB values
	double saturation;
	double brightness;

	int red;			//RGB values
	int green;
	int blue;

	int redRate;		//fade Rates
	int greenRate;
	int blueRate;

	int fadePeriod;		//total fade period

	int updateType;	//type of update (RGB, HSB, etc.)
};

typedef struct timer {
	unsigned long previousTime;
	int interval;
};


#endif

