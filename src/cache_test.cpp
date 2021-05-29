#include <iostream>
#include <time.h>
#include <Windows.h>

using namespace std;

#define ARRAY_SIZE (1 << 28)                                    // test array size is 2^28 256MB
typedef unsigned char BYTE;										// define BYTE as one-byte type

BYTE array[ARRAY_SIZE];											// test array
int L1_cache_size = 1 << 15;									// 2^15 32KB
int L2_cache_size = 1 << 18;									// 2^18 256KB
int L1_cache_block = 64;
int L2_cache_block = 64;
int L1_way_count = 8;
int L2_way_count = 4;
int write_policy = 0;											// 0 for write back ; 1 for write through

// have an access to arrays with L2 Data Cache'size to clear the L1 cache
void Clear_L1_Cache() {
	memset(array, 0, L2_cache_size);
}

// have an access to arrays with ARRAY_SIZE to clear the L2 cache
void Clear_L2_Cache() {
	memset(&array[L2_cache_size + 1], 0, ARRAY_SIZE - L2_cache_size);
}

int L1_DCache_Size() {
	cout << "L1_Data_Cache_Test" << endl;
	//add your own code
	/* 不断地在内存中读取一块连续的数据，数据块大小可以从1KB一直到128KB，然后观察平均读取速度 */
	int dataBlockBeginSize = 1;
	int dataBlockEndSize = 1 << 7;
	BYTE tmp;
	int begin; // 记录读取开始时的时间
	int finish; // 记录读取结束时的时间
	int times[7]; // 记录差值
	int count = 0;
	int max;
	int index = 1;
	for (int i = dataBlockBeginSize; i <= dataBlockEndSize; i*=2)// 读取的块数大小每次循环翻倍
	{
		
		begin = clock();
		for (int j = 0; j < 28011071; j++)
		{
			tmp += array[(rand()*rand())%(i<<10)];// 随机访问数组，数据块大小为i，介于1KB到16MB之间

		}
		finish = clock();
		times[count] = finish - begin;
		cout << "Test_Array_Size = " << i << "KB  ";
		cout << "Average access time = " << times[count] << "ms  " <<endl;
		count ++;
	}
	max = times[1]-times[0];
	for (int i = 1; i < 6; i++)
	{
		if (times[i+1]-times[i] > max)
		{
			max = times[i+1]-times[i];
			index = 1 << i;
		}
	}
	cout << "L1_DCache_Size is " << index << "KB  " <<endl;
}

int L2_Cache_Size() {
	cout << "L2_Data_Cache_Test" << endl;
	//add your own code
	/* 不断地在内存中读取一块连续的数据，数据块大小可以从128KB一直到16MB，然后观察平均读取速度 */
	int dataBlockBeginSize = 1 << 7;
	int dataBlockEndSize = 1 << 12;
	BYTE tmp;
	int begin; // 记录读取开始时的时间
	int finish; // 记录读取结束时的时间
	int times[5]; // 记录差值
	int count = 0;
	int max;
	int index = 1;
	for (int i = dataBlockBeginSize; i <= dataBlockEndSize; i*=2)// 读取的块数大小每次循环翻倍
	{
		
		begin = clock();
		for (int j = 0; j < 58011071; j++)
		{
			tmp += array[(rand()*rand())%(i<<10)];// 随机访问数组，数据块大小为i，介于1KB到16MB之间

		}
		finish = clock();
		times[count] = finish - begin;
		cout << "Test_Array_Size = " << i << "KB  ";
		cout << "Average access time = " << times[count] << "ms  " <<endl;
		count ++;
	}
	max = times[1]-times[0];
	for (int i = 1; i < 4; i++)
	{
		if (times[i+1]-times[i] > max)
		{
			max = times[i+1]-times[i];
			index = 1 << i+7;
		}
	}
	cout << "L1_DCache_Size is " << index << "KB  " <<endl;
}

int L1_DCache_Block() {
	cout << "L1_DCache_Block_Test" << endl;
	//add your own code
	/* 连续访问数组，增加每次访问间隔，当访问时间出现跳变，处于跳变的值就是cache块的大小 */
	/* 坐标跳变：间隔为1,2,4,8,16,32,64,128... */
	/*  */
	BYTE tmp;
	int count = 0;
	int times[8];
	int begin;
	int finish;
	int index = 0;
	int max;
	for (int i = 1; i <= 256; i*=2)// i为跳变间隔数
	{
		begin = clock();
		for (int j = 0; j < i; j++)// 间隔数为几，就要循环访问几次
		{
			for (int k = j; k < ARRAY_SIZE; k=k+i)
			{
				tmp += array[k];
			}
		}
		finish = clock();
		times[index] = finish - begin;
		cout << "Test_Jump_Size = " << i << "B  ";
		cout << "Average access time = " << times[index] << "ms  " <<endl;
		index++;
	}
	max = times[1]-times[0];
	for (int i = 1; i < 7; i++)
	{
		if (times[i+1]-times[i] > max)
		{
			max = times[i+1]-times[i];
			index = 1 << i;
		}
	}
	cout << "L1_Block_Size is " << index << "B  " <<endl;
}

int L2_Cache_Block() {
	cout << "L2_Cache_Block_Test" << endl;
	//add your own code
	/* 连续访问数组，增加每次访问间隔，当访问时间出现跳变，处于跳变的值就是cache块的大小 */
	/* 坐标跳变：间隔为1,2,4,8,16,32,64,128... */
	/*  */
	BYTE tmp;
	int count = 0;
	int times[8];
	int begin;
	int finish;
	int index = 0;
	int max;
	for (int i = 1; i <= 256; i*=2)// i为跳跃间隔数
	{
		begin = clock();
		for (int j = 0; j < i; j++)// 间隔数为几，就要循环访问几次
		{
			for (int k = j; k < ARRAY_SIZE; k=k+i)
			{
				tmp += array[k];
			}
		}
		finish = clock();
		times[index] = finish - begin;
		cout << "Test_Jump_Size = " << i << "B  ";
		cout << "Average access time = " << times[index] << "ms  " <<endl;
		index++;
	}
	max = times[1]-times[0];
	for (int i = 1; i < 7; i++)
	{
		if (times[i+1]-times[i] > max)
		{
			max = times[i+1]-times[i];
			index = 1 << i;
		}
	}
	cout << "L2_Block_Size is " << index << "B  " <<endl;
}

int L1_DCache_Way_Count() {
	cout << "L1_DCache_Way_Count" << endl;
	//add your own code
}

int L2_Cache_Way_Count() {
	cout << "L2_Cache_Way_Count" << endl;
	//add your own code
}

int Cache_Write_Policy() {
	cout << "Cache_Write_Policy" << endl;
	//add your own code
}

void Check_Swap_Method() {
	cout << "L1_Check_Replace_Method" << endl;
	//add your own code
	
}

int main() {
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	L1_cache_size = L1_DCache_Size();
	L2_cache_size = L2_Cache_Size();
	L1_cache_block = L1_DCache_Block();
	L2_cache_block = L2_Cache_Block();
	L1_way_count = L1_DCache_Way_Count();
	L2_way_count = L2_Cache_Way_Count();
	write_policy = Cache_Write_Policy();
	Check_Swap_Method();
	system("pause");
	return 0;
}


