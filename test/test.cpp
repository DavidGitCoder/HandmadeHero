#include <windows.h>
#include <stdio.h>

void bar(void) {
	int BarVariable;
	BarVariable = 100;
}
// Function definition
void foo(void) { // Function signature or prototype

	/*const char* Foo = "This is the first thing we have actually printed.\r\n";
	OutputDebugStringA(
		"This is the first thing we have actually printed.\r");
	printf("\n");*/
	int FooVariable;
	FooVariable= 200;
	bar();

}

struct projectile
{
	// NOTE: These are the members, or "fields" of this structure
	char unsigned IsThisOnFire; // NOTE (Casey): 1 if it's on fire, 0 if it's not
	int Damage; // NOTE: This is how much damage it does on impact
	int ParticlePerSecond; // NOTE: For special effects
	short HowManyCooks; // NOTE: Too many cooks?

	// 1 byte + 4 bytes + 4 bytes + 2 bytes = 11 bytes, logically right?
	// BUT it's actually using 16 bytes because CPU uses 32-bit (on x86) or 4 bytes even for char (1 byte) because it's easier for it to deal
	// with memory in groups of 4 bytes, it adds padding, even though some bytes will never be used

	// BUT
	// the compiler will pack things more neatly like in this instance:
	// the size will be thus 6 bytes: 2 + 2 + 1 + 1
	// the only caveat is that the smaller types have to come last, if not, the total size will be 8 bytes
	
	//struct projectile
	//{
	//	short unsigned IsThisOnFire;
	//	short Damage;
	//	char ParticlePerSecond;
	//	char HowManyCooks; 
	//};
};

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	/*
	char SmallS; //8 bits 00000000 - 256 different values [-128,127]
	char unsigned SmallU; // 8 bits unsigned 256 different values [0,255]

	short MediumS; // 16 bits - 65536
	short unsigned MediumU;

	int LargeS; //32 bit ~4 billion
	int unsigned LargeU;
	*/
	
	// HEXADECIMALS
		// 0x006ffc4c
		//0 - 0
		//1 - 2
		//2 - 3
		//3 - 4
		//4 - 5
		//5 - 6
		//6 - 7
		//7 - 8
		//8 - 9
		//9 - 0
		//10 - A
		//11 - B
		//12 - C
		//13 - D
		//14 - E
		//15 - F
		//
		//0xA = 10
		//0xAA = 16*10 + 10 = 170
		//0xAAA = 16*16*10 + 16*10 + 10 = 2730
		//0xFF = 16*15 + 15 = 255
		//
		//006FFC4C = 16*16*16*16*16*6 + 16*16*16*16*15 + 16*16*16*15 + 16*16*12 + 16*4 + 12

	// also if 2 bytes are: low byte = 1 high byte = 240 then to calculate the decimal value do: 240*256 + 1

	projectile Test;

	int SizeOfChar = sizeof(char unsigned);
	int SizeOfInt = sizeof(int);
	int SizeOfProjectile = sizeof(projectile);
	int SizeOfTest = sizeof(Test);

	Test.IsThisOnFire = 1;
	Test.Damage = 2;
	Test.ParticlePerSecond = 3;
	Test.HowManyCooks = 4;


	unsigned short* MrPointerMan = (unsigned short*) &Test;

	projectile Projectiles[40]; // arrays are pointers
	projectile* ProjectilePointer = Projectiles; // so no & needed

	sizeof(Projectiles); // 16 * 40 = 645 bytes
	sizeof(ProjectilePointer); // pointers size is always 4 bytes

	Projectiles[30].Damage = 60;
	// C automatically multiplies 30 by the number of bytes 
	//the type is using, so here is  30*16=480 bytes down from the start of ProjectilePointer
	(ProjectilePointer + 30)->Damage = 100; 

	// (char *) forces C to treat ProjectilePointer as if it were pointing to a 1 byte (8 bits) data
	((projectile *)((char *)ProjectilePointer + 30 * sizeof(projectile)))->Damage=200; // force

	char* BytePointer = (char*)ProjectilePointer;
	BytePointer = BytePointer + 30 * sizeof(projectile);
	projectile* Thirty = (projectile*)BytePointer;
	Thirty->Damage = 300; // arrow is to access a fiedld from a  pointer, otherwise use a dot

}


