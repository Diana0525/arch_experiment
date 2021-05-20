#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include "pin.H"

using namespace std;

ofstream OutFile;

#define truncate(val, bits) ((val) & ((1 << (bits)) - 1))

static UINT64 takenCorrect = 0;
static UINT64 takenIncorrect = 0;
static UINT64 notTakenCorrect = 0;
static UINT64 notTakenIncorrect = 0;

template <size_t N, UINT64 init = (1 << N)/2 - 1>   // N < 64
class SaturatingCnt
{
    UINT64 val;
    public:
        SaturatingCnt() { reset(); }

        void increase() { if (val < (1 << N) - 1) val++; }
        void decrease() { if (val > 0) val--; }

        void reset() { val = init; }
        UINT64 getVal() { return val; }

        BOOL isTaken() { return (val > (1 << N)/2 - 1); }
};

template<size_t N>      // N < 64
class ShiftReg
{
    UINT64 val;
    public:
        ShiftReg() { val = 0; }

        bool shiftIn(bool b)
        {
            bool ret = !!(val&(1<<(N-1)));
            val <<= 1;
            val |= b;
            val &= (1<<N)-1;
            return ret;
        }

        UINT64 getVal() { return val; }
};

class BranchPredictor
{
    public:
        BranchPredictor() { }
        virtual BOOL predict(ADDRINT addr) { return FALSE; };
        virtual void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr) {};
};

BranchPredictor* BP;



/* ===================================================================== */
/* ������ʵ��2�ֶ�̬Ԥ�ⷽ��                                               */
/* ===================================================================== */
// 1. BHT-based branch predictor
template<size_t L>
class BHTPredictor: public BranchPredictor
{
    SaturatingCnt<2> counter[1 << L];
    
    public:
        BHTPredictor() { }

        BOOL predict(ADDRINT addr)
        {
            // TODO:
			return counter[truncate(addr,L)].isTaken();
        }

        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
        {
            // TODO:
			if (takenPredicted)
			{
				if(takenActually)
				{
					counter[truncate(addr,L)].increase();
				}
				else
				{
					counter[truncate(addr,L)].decrease();
				}
			}
			else
			{
				if (takenActually)
				{
					counter[truncate(addr,L)].increase();
				}
				else
				{
					counter[truncate(addr,L)].decrease();
				}
			}
			
        }
};

// 2. Global-history-based branch predictor
template<size_t L, size_t H, UINT64 BITS = 2>
class GlobalHistoryPredictor: public BranchPredictor
{
    SaturatingCnt<BITS> bhist[1 << L];  // PHT�еķ�֧��ʷ�ֶ�
    ShiftReg<H> GHR;
    
    // TODO:
	public:
		GlobalHistoryPredictor() { }
		
		BOOL predict(ADDRINT addr)
		{
			return bhist[truncate(addr^GHR.getVal(),L)].isTaken();
		}
		
		void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
		{
			if (takenPredicted)
			{
				if(takenActually)
				{
					bhist[truncate(addr^GHR.getVal(),L)].increase();
					GHR.shiftIn(1);
				}
				else
				{
					bhist[truncate(addr^GHR.getVal(),L)].decrease();
					GHR.shiftIn(0);
				}
			}
			else
			{
				if (takenActually)
				{
					bhist[truncate(addr^GHR.getVal(),L)].increase();
					GHR.shiftIn(1);
				}
				else
				{
					bhist[truncate(addr^GHR.getVal(),L)].decrease();
					GHR.shiftIn(0);
				}
			}
		}
};

// 3. Local-history-based branch predictor
template<size_t L, size_t H, size_t HL = 6, UINT64 BITS = 2>
class LocalHistoryPredictor: public BranchPredictor
{
    SaturatingCnt<BITS> bhist[1 << L];  // PHT�еķ�֧��ʷ�ֶ�
    ShiftReg<H> LHT[1 << HL]; // 64��LHT��¼

    // TODO:
    public:
		LocalHistoryPredictor() { }
		
		BOOL predict(ADDRINT addr)
		{
			return bhist[truncate(addr^LHT[truncate(addr, HL)].getVal(),L)].isTaken();
		}
		
		void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
		{

			if(takenActually)
			{
				bhist[truncate(addr^LHT[truncate(addr, HL)].getVal(),L)].increase();
				LHT[truncate(addr, HL)].shiftIn(1);
			}
			else
			{
				bhist[truncate(addr^LHT[truncate(addr, HL)].getVal(),L)].decrease();
				LHT[truncate(addr, HL)].shiftIn(0);
			}

		}
};

/* ===================================================================== */
/* ������Ԥ������ѡ����ƿ���ȫ�ַ���ֲ���ʵ�֣���ѡһ����                   */
/* ===================================================================== */
// 1. Tournament predictor: Select output by global selection history
template<UINT64 BITS = 2>
class TournamentPredictor_GSH: public BranchPredictor
{
    SaturatingCnt<BITS> GSHR;
    BranchPredictor* BPs[2];

    public:
        TournamentPredictor_GSH(BranchPredictor* BP0, BranchPredictor* BP1)
        {
            BPs[0] = BP0;
            BPs[1] = BP1;
        }

        // TODO:
		BOOL predict(ADDRINT addr) 
		{
			if (GSHR.isTaken()) // ѡ��Ԥ����2
			{
				return BPs[1]->predict(addr);
			}
			else // ѡ��Ԥ����1
			{
				return BPs[0]->predict(addr);
			}
		};
        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr) 
		{
			if (BPs[0]->predict(addr) == takenActually && BPs[1]->predict(addr) != takenActually) // ֻ��1Ԥ����ȷ
			{
				GSHR.decrease();
			}
			else if (BPs[0]->predict(addr) != takenActually && BPs[1]->predict(addr) == takenActually)// ֻ��2Ԥ����ȷ
			{
				GSHR.increase();
			}
			BPs[0]->update(takenActually, takenPredicted, addr);
			BPs[1]->update(takenActually, takenPredicted, addr);
		};

};

// 2. Tournament predictor: Select output by local selection history
template<size_t L, UINT64 BITS = 2>
class TournamentPredictor_LSH: public BranchPredictor
{
    SaturatingCnt<BITS> LSHT[1 << L];
    BranchPredictor* BPs[2];

    public:
        TournamentPredictor_LSH(BranchPredictor* BP0, BranchPredictor* BP1)
        {
            BPs[0] = BP0;
            BPs[1] = BP1;
        }

        // TODO:
		BOOL predict(ADDRINT addr) 
		{
			 if (!LSHT[truncate(addr, L)].isTaken()) // LSHT[i]�����λΪ1
			 {
				 return BPs[0]->predict(addr); // �����Ԥ����1��Ԥ����
			 }
			 else
			 {
				 return BPs[1]->predict(addr);
			 }
		};
        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr) 
		{
			if (BPs[0]->predict(addr) == takenActually && BPs[1]->predict(addr) != takenActually) // ֻ��1Ԥ����ȷ
			{
				LSHT[truncate(addr, L)].decrease();
			}
			else if (BPs[0]->predict(addr) != takenActually && BPs[1]->predict(addr) == takenActually)// ֻ��2Ԥ����ȷ
			{
				LSHT[truncate(addr, L)].increase();
			}
			BPs[0]->update(takenActually, takenPredicted, addr);
			BPs[1]->update(takenActually, takenPredicted, addr);
		};
};
// ���������ȡ��ָ֧���ַ�����÷�֧Ԥ����ģ�ͽ��з�֧Ԥ�⣬����¼ģ�͵�Ԥ��������
// This function is called every time a control-flow instruction is encountered
void predictBranch(ADDRINT pc, BOOL direction)
{
    BOOL prediction = BP->predict(pc);
    BP->update(direction, prediction, pc);
    if (prediction)
    {
        if (direction)
            takenCorrect++;
        else
            takenIncorrect++;
    }
    else
    {
        if (direction)
            notTakenIncorrect++;
        else
            notTakenCorrect++;
    }
}
// ��׮������Ŀ���������ѡ������ʵִ�еĿ�����ָ�Ȼ��ֱ�����Щָ��ķ�֧�ɹ���ʧ�ܴ������������
// Pin calls this function every time a new instruction is encountered
void Instruction(INS ins, void * v)
{
    if (INS_IsControlFlow(ins) && INS_HasFallThrough(ins))
    {
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)predictBranch,
                        IARG_INST_PTR, IARG_BOOL, TRUE, IARG_END);

        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)predictBranch,
                        IARG_INST_PTR, IARG_BOOL, FALSE, IARG_END);
    }
}

// This knob sets the output file name
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "brchPredict.txt", "specify the output file name");

// This function is called when the application exits
VOID Fini(int, VOID * v)
{
	double precision = 100 * double(takenCorrect + notTakenCorrect) / (takenCorrect + notTakenCorrect + takenIncorrect + notTakenIncorrect);
    
    cout << "takenCorrect: " << takenCorrect << endl
    	<< "takenIncorrect: " << takenIncorrect << endl
    	<< "notTakenCorrect: " << notTakenCorrect << endl
    	<< "nnotTakenIncorrect: " << notTakenIncorrect << endl
    	<< "Precision: " << precision << endl;
    
    OutFile.setf(ios::showbase);
    OutFile << "takenCorrect: " << takenCorrect << endl
    	<< "takenIncorrect: " << takenIncorrect << endl
    	<< "notTakenCorrect: " << notTakenCorrect << endl
    	<< "nnotTakenIncorrect: " << notTakenIncorrect << endl
    	<< "Precision: " << precision << endl;
    
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // TODO: New your Predictor below.
    // BP = new BranchPredictor();
	// BP = new BHTPredictor<19>(); // 1
	// BP = new GlobalHistoryPredictor<19,19>(); // 2
	BP = new LocalHistoryPredictor<19, 19>(); // 3
	// BranchPredictor* BP_1 = new GlobalHistoryPredictor<19,19>(); // 4
	// BranchPredictor* BP_2 = new LocalHistoryPredictor<19,19>(); // 4
	// BP = new TournamentPredictor_GSH<>(BP_1, BP_2); // 4-1
	// BP = new TournamentPredictor_LSH<19>(BP_1, BP_2); // 4-2

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    
    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
