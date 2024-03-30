/*
 * MODE_SW.h
 *
 * Created: 2017-02-21 오전 11:27:04
 *  Author: Shim
 */ 


#ifndef MODE_SW_H_
#define MODE_SW_H_




class MODE_SW {
	public:
	MODE_SW();
	MODE_SW(int pin);
	virtual ~MODE_SW();
	void begin(void);
	short read(void);
	bool isON(void);
	bool isOFF(void);

	private:
	int pin;
	//short val;
};




#endif /* MODE_SW_H_ */