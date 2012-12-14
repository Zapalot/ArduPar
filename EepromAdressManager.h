#pragma once

class EepromAdressManager{
	public:
	static int nextFreeAdress;	///< points to the next free byte
	static int getAdressFor(int bytes);	///< returns an adress and reserves the requested number of bytes
};