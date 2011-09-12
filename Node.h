#ifndef NODE_H_
#define NODE_H_

#include <math.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include "Subjects1-6.h"
#include "Objects.h"
#include "Supervisor.h"
#include "Hash.h"

typedef std::unordered_map<int, DATATYPE> HashMap;

SubjectType(Node)
{
protected:
    NodeRelay *left, *right;//, *node0, *node1;
    Relay *node0, *node1;
    Relay *in;
    double num;
    bool isReal;
    bool leftstable;
    bool rightstable;
    HashMap data;//was soll gespeichert werden?

    void checkDead(NodeRelay **side);
    void checkValid();
    double calcRoutingBound();
    void checkStable(double id);
    void BuildSide(IdObj * ido, NodeRelay **side, bool right);


public:
    FirstAction(Node, Init)

    Action Init(InitObj * init);
    Action ConnectChild(IdObj * id);
    Action Wakeup(NumObj * num);
    Action BuildDeBruijn();
    Action Insert(DateObj * dob);
    Action Delete(NumObj * key);
    Action LookUp(NumObj * key);
    Action Join(IdObj * id);
    Action Leave(IdObj * id);
    Action Probing(Probe * ido);
    Action BuildList(IdObj * id);
    Action BuildWeakConnectedComponent(NumObj * numo);
    Action ReceiveLookUp(DateObj * dob);
    Action FinishSearch(SearchJob * sj);
    Action Search(SearchJob * sj);
    Action TriggerDataTransfer(IdObj * ido);
};


#endif  // NODE_H_
