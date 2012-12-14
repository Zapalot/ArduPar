#include "EepromAdressManager.h"

int EepromAdressManager::nextFreeAdress=0;
int EepromAdressManager::getAdressFor(int bytes){
	int curAdress=nextFreeAdress;
	nextFreeAdress+=bytes;
	return(curAdress);

}