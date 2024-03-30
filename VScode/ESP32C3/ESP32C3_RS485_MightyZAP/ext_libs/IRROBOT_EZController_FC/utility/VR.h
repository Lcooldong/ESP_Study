/*
 * VR.h
 *
 * Created: 2017-02-21 오전 11:25:08
 *  Author: Shim
 */ 


#ifndef VR_H_
#define VR_H_



class VR {
public:
 VR();
 VR(int pin);
 virtual ~VR();
 short read(void);

private:
	int pin;
	//short val;
};



#endif /* VR_H_ */