/* Program written by Evan Morgan, C12503487 DT081/4
 * Description of functionality in main()
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <string>

using namespace std;

struct cacheLine    //cache line structure to store a tag, data is an array of 4 bytes by 2 bytes where the second byte is the line's content, a valid flag and a Least Recently Used flag
{
    string tag;
    string data[4][2];
    bool valid;
    bool LRU;
};

void initCacheValues(cacheLine cache1[], cacheLine cache2[], int cache1size, int cache2size);
string randomStrGen(int length);
void writeCacheEntry(cacheLine a[], int i, string address, int j, string content);

int main()
{
    //Variable declarations
	int missCount = 0, lineFillFlag = 0;
	bool cache1Write = false, cache2Write = false, foundDRAMAddress = false, breakout = false, hit = false;
	string address = "", addressStart = "", addressEnd = "", tempAddress = "", lastAddress = "", content = "";
	ifstream CPU, DRAM;
	//Give each cache 3 sets containing 8 bytes each, resulting in 24B per cache, 48B cache memory total.
	cacheLine cache1[3], cache2[3];

	//Introduction
	cout << "This simulator models a 2-way set associative cache with 1 byte words." << endl;
	cout << "The address bus is 16 bits wide and the data bus is 8 bits wide. The" << endl;
	cout << "cache will initially be empty.  A test file containing addresses is" << endl;
	cout << "used as a test source. As each address is read from the file it is" << endl;
	cout << "checked against the cache entries.  A cache miss results in a cache" << endl;
	cout << "line fill. A cache line is 4 bytes. A second file is used acting as" << endl;
	cout << "DRAM. The full CPU address is provided on screen as a visual aid. An" << endl;
	cout << "8 bit data bus means an integer stored in memory could be from 0-255" << endl;
	cout << "and for this simulator that is what will be used for content. For test" << endl;
	cout << "purposes, a program of 70 instructions(30 unique) is used. Each cache" << endl;
	cout << "is 48B in size." << endl << endl;
	cout << "Written by Evan Morgan." << endl << endl << endl;

	//Use ifstream object CPU to open the CPU program list text file
	CPU.open("CPUAddressList.txt", ifstream::in);

	//Initialize the cache values to random strings
	initCacheValues(cache1, cache2, ((sizeof cache1)/(sizeof *cache1)), ((sizeof cache2)/(sizeof *cache2)));

	//Run through the CPU program list line by line until there's no more addresses
	while (CPU >> address)
	{
	    //Split the address into two halves - start and end
	    addressStart = address[0];
        addressStart += address[1];
        addressStart += address[2];
        addressStart += address[3];
        addressStart += address[4];
        addressStart += address[5];
        addressStart += address[6];
        addressStart += address[7];

	    addressEnd = address[8];
        addressEnd += address[9];
        addressEnd += address[10];
        addressEnd += address[11];
        addressEnd += address[12];
        addressEnd += address[13];
        addressEnd += address[14];
        addressEnd += address[15];

        //Display full address for easy debugging/simulation purposes
        cout << "Full address: " << addressStart + addressEnd << endl;
        cout << "Searching for " << addressStart << " in cache memory..." << endl;

        //For every cache line in each cache
	    for(int i=0; i<((sizeof cache1)/(sizeof *cache1)); i++)
        {
            //If the tag isn't empty, display what tag we are currently checking
            if(cache1[i].tag!="")
            {
                cout << "Block 1, Set " << i << ": " << cache1[i].tag << endl;
            }

            //If the tag isn't empty, display what tag we are currently checking
            if(cache2[i].tag!="")
            {
                cout << "Block 2, Set " << i << ": " << cache2[i].tag << endl;
            }

            //For every data value in each cacheline
            for(int j=0; j<4; j++)
            {
                //If a cache1 entry is valid and the tag is the same as the first half of the address
                if(cache1[i].valid && cache1[i].tag==addressStart)
                {
                    cout << "Potential Hit: Comparing data... " << cache1[i].data[j][0] << ":" << addressEnd << endl;

                    //If the data value in the compared tag is the same as the second half of the address
                    if(cache1[i].data[j][0]==addressEnd)
                    {
                        cout << "Hit in Block 1, Set " << i << ", Line " << j << ", Contents: " << cache1[i].data[j][1] << "!" << endl << endl;
                        //Set the hit flag to true to ensure no continuation to "miss" code
                        hit = true;
                        //Break out of the for(j) current loop
                        break;
                    }

                    //If the data value in the compared tag is not the same, display data mismatch
                    else
                    {
                        cout << "Data mismatched." << endl;
                    }
                }

                //If a cache2 entry is valid and the tag is the same as the first half of the address
                if(cache2[i].valid && cache2[i].tag==addressStart)
                {
                    cout << "Potential Hit: Comparing second half of addresses. " << cache2[i].data[j][0] << ":" << addressEnd << endl;

                    //If the data value in the compared tag is the same as the second half of the address
                    if((cache2[i].data[j][0])==addressEnd)
                    {
                        cout << "Hit in Block 2, Set " << i << ", Line " << j << ", Contents: " << cache2[i].data[j][1] << "!" << endl << endl;
                        //Set the hit flag to true to ensure no continuation to "miss" code
                        hit = true;
                        //Break out of the for(j) current loop
                        break;
                    }

                    //If the data value in the compared tag is not the same, display data mismatch
                    else
                    {
                        cout << "Data mismatched." << endl;
                    }
                }

                //If the tag matches neither tag, add 1 to the miss count
                else
                {
                    missCount++;
                }
            }
        }

        //If the miss count is higher than both the amount of sets in both caches and the hit flag hasn't been set
        if(missCount>=((sizeof cache1)/(sizeof *cache1)) && missCount>=((sizeof cache2)/(sizeof *cache2)) && hit==false)
        {
            cout << "MISS! NOT IN CACHE!" << endl << "Searching DRAM..." << endl;

            //Store the current address in the tempAddress variable, so a new address can be taken from DRAM
            tempAddress = addressStart + addressEnd;
            //Use the fstream DRAM object to open the DRAM text file that contains all the addresses and their content
            DRAM.open("DRAM.txt", ifstream::in);

            //Go through each address in the DRAM until there are none left
            while (DRAM >> address >> content)
            {
                //If the DRAM address matches the CPU address OR the CPU has already started a cache line fill AND the matched address in DRAM hasn't been found yet
                if((address==tempAddress || lineFillFlag>0) && !foundDRAMAddress)
                {
                    //The DRAM address has been found, this prevents the previous conditional being triggered as the while loop cycles through DRAM
                    foundDRAMAddress=true;
                    cout << "Found address in DRAM..." << endl;
                    cout << "Initiating cache line fill..." << endl;

                    //For every address cache line in the cache
                    for(int i = 0; i<((sizeof cache1)/(sizeof *cache1)); i++)
                    {
                        //For every data value in each cache line
                        for(int j = 0; j<4; j++)
                        {
                            //If the DRAM address repeats itself, break out of the loop to prevent trivial unuseful data transfer
                            if(lastAddress==address)
                            {
                                break;
                            }

                            //If the entry's valid flag is not set in either cache1 OR cache2 OR the CPU is in the middle of a cache line fill
                            if(!cache1[i].valid || !cache2[i].valid || lineFillFlag>0)
                            {
                                //If a cache1 entry's valid flag is not set OR the CPU is cache line filling cache1 AND the CPU is definitely not writing to cache2
                                if((!cache1[i].valid || cache1Write) && !cache2Write)
                                {
                                    //Display what is currently being stored in this line
                                    cout << "Block 1, Set " << i << ", Line " << j << " Old: Address-" << cache1[i].tag + cache1[i].data[j][0] << "; Valid-" << cache1[i].valid << "; Contents-" << cache1[i].data[j][1] << ";" << endl;
                                    //Write the address and it's associated content from DRAM to this line in Block 1
                                    writeCacheEntry(cache1, i, address, j, content);
                                    //Set the valid flag for this cache entry
                                    cache1[i].valid = true;
                                    //The CPU has now begun writing to cache1, set the flag to show this
                                    cache1Write = true;
                                    //Display the new information stored in this line
                                    cout << "Block 1, Set " << i << ", Line " << j << " New: Address-" << cache1[i].tag + cache1[i].data[j][0] << "; Valid-" << cache1[i].valid << "; Contents-" << cache1[i].data[j][1] << ";" << endl;
                                }

                                //If a cache2 entry's valid flag is not set OR the CPU is cache line filling cache2 AND the CPU is definitely not writing to cache1
                                if((!cache2[i].valid || cache2Write) && !cache1Write)
                                {
                                    //Display what is currently being stored in this line
                                    cout << "Block 2, Set " << i << ", Line " << j << "  Old: Address-" << cache2[i].tag + cache2[i].data[j][0] << "; Valid-" << cache2[i].valid << "; Contents-" << cache2[i].data[j][1] << ";" << endl;
                                    //Write the address and it's associated content from DRAM to this line in Block 2
                                    writeCacheEntry(cache2, i, address, j, content);
                                    //Set the valid flag for this cache entry
                                    cache2[i].valid = true;
                                    //The CPU has now begun writing to cache1, set the flag to show this
                                    cache2Write = true;
                                    //Display the new information stored in this line
                                    cout << "Block 2, Set " << i << ", Line " << j << " New: Address-" << cache2[i].tag + cache2[i].data[j][0] << "; Valid-" << cache2[i].valid << "; Contents-" << cache2[i].data[j][1] << ";" << endl;
                                }

                                //If this is the first iteration through this loop, set the lineFillFlag to do it 3 more times
                                if(lineFillFlag==0)
                                {
                                    lineFillFlag = 4;
                                }

                                //Decrement the lineFillFlag
                                lineFillFlag--;

                                //If the lineFillFlag is 0 after decrementation, break out of this for(j) loop as the cache line fill is complete
                                if(lineFillFlag==0)
                                {
                                    cout << endl;
                                    //Set the breakout flag to break out of the surrounding for(i) loop
                                    breakout = true;
                                    break;
                                }

                                //Set the last address as the one just checked
                                lastAddress = address;
                                //Search through DRAM for the next address
                                DRAM >> address >> content;
                            }
                        }
                        //If the breakout flag is set, this means the cache line fill is complete and we can break out of the for(i) loop
                        if(breakout==true)
                        {
                            break;
                        }
                    }
                }
            }

        //Reset values to default for next iteration of the while loop
        missCount = 0;
        breakout = false;
        cache1Write = false;
        cache2Write = false;
        DRAM.close();
        foundDRAMAddress = false;
        }
        //Reset hit value to default
        hit = false;
	}
}

//Initialize the cache values function
void initCacheValues(cacheLine cache1[], cacheLine cache2[], int cache1size, int cache2size)
{
    //For every cache line in Block 1
    for(int i = 0; i<cache1size; i++)
    {
        //Set the tag to a random string
        cache1[i].tag=randomStrGen(8);
        //For every data value in the cache line
        for(int j=0; j<4; j++)
        {
            //Set the data value to a random string
            cache1[i].data[j][0]=randomStrGen(6);
            cache1[i].data[j][1]=randomStrGen(3);
        }
        //Set both flags as false
        cache1[i].valid=false;
        cache1[i].LRU=false;
    }

    //For every cache line in Block 1
    for(int i = 0; i<cache2size; i++)
    {
        //Set the tag to a random string
        cache2[i].tag=randomStrGen(8);
        //For every data value in the cache line
        for(int j=0; j<4; j++)
        {
            //Set the data value to a random string
            cache2[i].data[j][0]=randomStrGen(6);
            cache2[i].data[j][1]=randomStrGen(3);
        }
        //Set both flags to false
        cache2[i].valid=false;
        cache2[i].LRU=false;
    }
}

//Generate a random string of a certain length
string randomStrGen(int length)
{
    string s="";
    //For every character in the length of the string
    for (int i = 0; i < length; ++i)
    {
        //Choose a random number, translate that number into an alphanumeric character and store it in the string
        int randomChar = rand()%(26+26+10);
        if (randomChar < 26)
            s[i] = 'a' + randomChar;
        else if (randomChar < 26+26)
            s[i] = 'A' + randomChar - 26;
        else
            s[i] = '0' + randomChar - 26 - 26;
    }
    //The last element of the string is always 0
    s[length] = 0;
    //Return the randomly generated string
    return s;
}

//Write a cache entry to a specific cache line
void writeCacheEntry(cacheLine a[], int i, string address, int j, string content)
{
    //Set the tag to the first half of the address
    a[i].tag = address[0];
    a[i].tag += address[1];
    a[i].tag += address[2];
    a[i].tag += address[3];
    a[i].tag += address[4];
    a[i].tag += address[5];
    a[i].tag += address[6];
    a[i].tag += address[7];

    //Set the data to the latter half of the address
    a[i].data[j][0] = address[8];
    a[i].data[j][0] += address[9];
    a[i].data[j][0] += address[10];
    a[i].data[j][0] += address[11];
    a[i].data[j][0] += address[12];
    a[i].data[j][0] += address[13];
    a[i].data[j][0] += address[14];
    a[i].data[j][0] += address[15];

    //Write the associated content to the cache
    a[i].data[j][1] = content;
}
