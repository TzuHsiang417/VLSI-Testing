/* Logic Simulator
 * Last update: 2006/09/20 */
#include <iostream>
#include "gate.h"
#include "circuit.h"
#include "ReadPattern.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

//do logic simulation for test patterns
void CIRCUIT::LogicSimVectors()
{
    cout << "Run logic simulation" << endl;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        LogicSim();
        PrintIO();
    }
    return;
}

//do event-driven logic simulation
void CIRCUIT::LogicSim()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

//Used only in the first pattern
void CIRCUIT::SchedulePI()
{
    for (unsigned i = 0;i < No_PI();i++) {
        if (PIGate(i)->GetFlag(SCHEDULED)) {
            PIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PIGate(i));
        }
    }
    return;
}

//schedule all fanouts of PPIs to Queue
void CIRCUIT::SchedulePPI()
{
    for (unsigned i = 0;i < No_PPI();i++) {
        if (PPIGate(i)->GetFlag(SCHEDULED)) {
            PPIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PPIGate(i));
        }
    }
    return;
}

//set all PPI as 0
void CIRCUIT::SetPPIZero()
{
    GATE* gptr;
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        if (gptr->GetValue() != S0) {
            gptr->SetFlag(SCHEDULED);
            gptr->SetValue(S0);
        }
    }
    return;
}

//schedule all fanouts of gate to Queue
void CIRCUIT::ScheduleFanout(GATE* gptr)
{
    for (unsigned j = 0;j < gptr->No_Fanout();j++) {
        Schedule(gptr->Fanout(j));
    }
    return;
}

//initial Queue for logic simulation
void CIRCUIT::InitializeQueue()
{
    SetMaxLevel();
    Queue = new ListofGate[MaxLevel + 1];
    return;
}

//evaluate the output value of gate
VALUE CIRCUIT::Evaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue());
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = AndTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = OrTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { value = NotTable[value]; }
    return value;
}

extern GATE* NameToGate(string);

void PATTERN::Initialize(char* InFileName, int no_pi, string TAG)
{
    patterninput.open(InFileName, ios::in);
    if (!patterninput) {
        cout << "Unable to open input pattern file: " << InFileName << endl;
        exit( -1);
    }
    string piname;
    while (no_pi_infile < no_pi) {
        patterninput >> piname;
        if (piname == TAG) {
            patterninput >> piname;
            inlist.push_back(NameToGate(piname));
            no_pi_infile++;
        }
        else {
            cout << "Error in input pattern file at line "
                << no_pi_infile << endl;
            cout << "Maybe insufficient number of input\n";
            exit( -1);
        }
    }
    return;
}

//Assign next input pattern to PI
void PATTERN::ReadNextPattern()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetValue() != S0) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S0);
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetValue() != S1) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S1);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetValue() != X) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(X);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

void CIRCUIT::PrintIO()
{
    register unsigned i;
    for (i = 0;i<No_PI();++i) { cout << PIGate(i)->GetValue(); }
    cout << " ";
    for (i = 0;i<No_PO();++i) { cout << POGate(i)->GetValue(); }
    cout << endl;
    return;
}

//Assignment 2
void CIRCUIT::GeneratePattern(const char* filename, int num, bool unknown)
{
    int num_signal;

    if(unknown)
    {
        cout << "Generating the patterns with unknown" << endl;
        num_signal = 3;
    }
    else
    {
        cout << "Generating the patterns without unknown" << endl;
        num_signal = 2;
    }

    ofstream ofile(filename, ios::out);
    for(unsigned i=0; i<PIlist.size(); i++)
        ofile << "PI " << PIlist[i]->GetName() << " ";
    ofile << endl;

    srand(time(0));
    for(int i=0; i<num; i++)
    {
        for(unsigned j=0; j<PIlist.size(); j++)
        {
            int rand_value = rand() % num_signal;
            if(rand_value == 2)
                ofile << "X";
            else
                ofile << rand_value;
        }
        ofile << endl;
    }

    ofile.close();
}

void CIRCUIT::Mod_LogicSimVectors()
{
    cout << "Run ASS2 Mod logic simulation" << endl;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        Mod_LogicSim();
        PrintIO();
    }
    return;
}

void CIRCUIT::Mod_LogicSim()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Mod_Evaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

VALUE CIRCUIT::Mod_Evaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue());
    LogicEX ec_value, ec_value_temp;
    ec_value = Mod_Encode(value);
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && cv!=Mod_Decode(ec_value);++i) {
                ec_value_temp = Mod_Encode(gptr->Fanin(i)->GetValue());
                ec_value.leftbit = ec_value.leftbit & ec_value_temp.leftbit;
                ec_value.rightbit = ec_value.rightbit & ec_value_temp.rightbit;
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && cv!=Mod_Decode(ec_value);++i) {
                ec_value_temp = Mod_Encode(gptr->Fanin(i)->GetValue());
                ec_value.leftbit = ec_value.leftbit | ec_value_temp.leftbit;
                ec_value.rightbit = ec_value.rightbit | ec_value_temp.rightbit;
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) {
        ec_value.leftbit = !ec_value.leftbit; 
        ec_value.rightbit = !ec_value.rightbit; 
    }
    return Mod_Decode(ec_value);
}

LogicEX CIRCUIT::Mod_Encode(VALUE v1)
{
    LogicEX ans;
    if(v1 == S0 || v1==0)
    {
        ans.leftbit=0;
        ans.rightbit=0;
    }
    else if (v1 == S1 || v1==1)
    {
        ans.leftbit=1;
        ans.rightbit=1;
    }
    else
    {
        ans.leftbit=1;
        ans.rightbit=0;
    }

    return ans;
}

VALUE CIRCUIT::Mod_Decode(LogicEX b2v1)
{
    VALUE ans;
    if(b2v1.leftbit != b2v1.rightbit)
        ans = X;
    else if (b2v1.leftbit == 1)
        ans = S1;
    else 
        ans = S0;
    return ans;
}